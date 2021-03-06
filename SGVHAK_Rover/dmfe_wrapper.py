"""
MIT License

Copyright (c) 2018 Roger Cheng

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
"""
import serial
import struct

import configuration

maxangle = 45 # TODO: make this generally configurable

def bytetohex(bytearray):
  """
  Returns hexadecimal string representation of byte array
  Copied from StackOverflow
  https://stackoverflow.com/questions/19210414/byte-array-to-hex-string
  """
  return ''.join('{:02x}'.format(x) for x in bytearray)

class dmfe_wrapper:
  """
  Class that implements the rover motor control methods for David M Flynn
  Enterprises motor control boards.
  """
  def __init__(self):
    self.sp = None

  def check_sp(self):
    """ Raises error if we haven't opened serial port yet. """
    if self.sp == None:
      raise ValueError("DMFE serial communication is not available.")

  def connect(self):
    """
    Read serial port connection parameters from JSON configuration file
    and open the port.
    """

    # Read parameter file
    config = configuration.configuration("dmfe")
    connectparams = config.load()['connect']

    # Open serial port with parameters
    s = serial.Serial()
    s.baudrate = connectparams['baudrate']
    s.port = connectparams['port']
    s.timeout = connectparams['timeout']
    s.open()

    if s.is_open:
      self.sp = s

  def close(self):
    """
    Closes down the serial port
    """
    if self.sp.is_open:
      self.sp.close()
      self.sp = None

  def send(self, device_id, command, data=b'\x00\x00\x00'):
    """
    Send a command to a DMFE bus device, taking care of the header and
    checksum calculation for a command packet.
    """
    self.check_sp()
    packet = bytearray([0xDD, 0xDD])

    # Sender is master with ID of 1
    packet.append(1)

    # Append receiver ID
    if device_id < 2 or device_id > 0xFE:
      raise ValueError("Device ID {} is out of valid range 0x02 to 0xFE".format(device_id))
    packet.append(device_id)

    # Append command
    packet.append(command) # TBD: need to validate command?

    # Append data
    packet = packet + data # TBD: need to validate data is bytearray of size 3?

    # Calculate & append checksum
    checksum = 0
    for i in range (2,8):
      checksum = checksum ^ packet[i]
    packet.append(checksum)

    self.sp.write(packet)

  def read_raw(self, length=100):
    """
    Reads a stream of bytes from serial device and returns it without any
    attempts at parsing or validation
    """
    self.check_sp()
    return bytearray(self.sp.read(length))

  def read_ack(self):
    """
    We expect to receive a single byte 0xFF as acknowledgement
    """
    self.check_sp()
    r = bytearray(self.sp.read(1))

    if len(r) == 0:
      raise ValueError("Expected single byte 0xFF in response but received no data.")

    if r[0] != 255:
      raise ValueError("Expected 0xFF in response but got {}".format(r[0]))

  def read_dmfeserialservo(self):
    """
    We expect a device identifier string
    """
    self.check_sp()
    r = self.sp.read(18).decode('utf-8')

    if len(r) == 0:
      raise ValueError("Expected DMFE identification string but received no data.")

    if r == 'DMFE Serial Brushe':
      raise ValueError("Expected device to be serial servo but is serial brushed controller.")

    if r != 'DMFE Serial Servo\n':
      raise ValueError("Expected 'DMFE Serial Servo' but received {}".format(r))

  def read_dmfeserialbrushed(self):
    """
    We expect a device identifier string
    """
    self.check_sp()
    r = self.sp.read(20).decode('utf-8')

    if len(r) == 0:
      raise ValueError("Expected DMFE identification string but received no data.")

    if r == 'DMFE Serial Servo\n':
      raise ValueError("Expected device to be serial brushed but is serial servo.")

    if r != 'DMFE Serial Brushed\n':
      raise ValueError("Expected 'DMFE Serial Brushed' but received {}".format(r))

  def read_datapacket(self, expectedid):
    """
    We expect a data packet originating from device ID 'expectedid'

    Returns the 4-byte data array
    """
    self.check_sp()
    r = self.sp.read(7)

    if len(r) != 7:
      raise ValueError("Expected data packet of 7 bytes but received only {}".format(len(r)))
    
    if self.bytes_to_int(r[0]) != expectedid:
      raise ValueError("Expected data packet from device {} but received from {}".format(expectedid, r[0]))
    
    if self.bytes_to_int(r[1]) != 1:
      raise ValueError("Expected data packet for master id 1 but received ID {}".format(r[1]))

    checksum = 0
    for i in range (0,6):
      checksum = checksum ^ self.bytes_to_int(r[i])

    if checksum != self.bytes_to_int(r[6]):
      raise ValueError("Calculated checksum of {} does not match transmitted checksum {}".format(checksum,r[6]))

    return r[2:6]

  def version(self, id):
    """ Identifier string for this motor controller """
    return "DMFE"
  
  @staticmethod
  def check_id(id):
    """ Verifies servo ID is within range and inverted status is boolean"""
    if not isinstance(id, (tuple,list)):
      raise ValueError("DMFE identifier must be a tuple")

    if not isinstance(id[0], int):
      raise ValueError("DMFE device address must be an integer")

    if id[0] < 2 or id[0] > 253:
      raise ValueError("DMFE device address {} outside of valid range 2-253".format(id[0]))

    if not isinstance(id[1], int):
      raise ValueError("DMFE device center position must be an integer")

    if not isinstance(id[2], bool):
      raise ValueError("Inverted status must be a boolean")

    return tuple(id)

  @staticmethod
  def bytes_to_int(bytes):
    return int(bytes.encode('hex'), 16)

  @staticmethod
  def data1byte(data):
    """
    Given parameter, pack it into a single byte. Pad remainder with zero
    and return three element byte array

    data1byte(2) returns b'\x02\x00\x00'
    """
    return struct.pack("b",data) + b'\x00\x00'

  @staticmethod
  def data2byte(data):
    """
    Given parameter, pack it into two bytes. Pad remainder with zero
    and return three element byte array

    data2byte(1024) returns b'\x00\x04\x00'
    """
    return struct.pack("H",data) + b'\x00'

  def power_percent(self, id, percentage):
    """ Send brushed motor speed command to device 'id' at specified +/- 'percentage' """
    did, center, inverted = self.check_id(id)
    self.check_sp()

    if inverted:
      percentage = percentage * -1

    pct = int(percentage)
    if abs(pct) > 100:
      raise ValueError("Motor power percentage {0} outside valid range from 0 to 100.".format(pct))

    # 50 is wheel power maximum of Mr. Blue rover. TBD: Make this general and configurable
    power = (percentage * 50) / 100

    self.send(did, 0x87, self.data1byte(power))
    self.read_ack()

  def set_max_current(self, id, current):
    """ Set maximum current allowed before tripping protection """
    did, center, inverted = self.check_id(id)
    self.check_sp()
    # Not yet implemented

  def init_velocity(self, id):
    """ Initialize device at 'id' into velocity mode """
    did, center, inverted = self.check_id(id)
    self.check_sp()
    # Not applicable to DMFE devices
  
  def velocity(self,id,pct_velocity):
    """
    Runs the device in motor mode at specified velocity
    For DMFE brush motor devices, directly translates to power_percent.
    """
    self.power_percent(id,pct_velocity)

  def init_angle(self, id):
    """
    Sets the device at 'id' into servo position mode
    """
    did, center, inverted = self.check_id(id)
    self.check_sp()
    # Not applicable to DMFE devices

  def maxangle(self, id):
    """
    Notifies maximum servo angle allowed in angle()
    """
    did, center, inverted = self.check_id(id)
    self.check_sp()
    return maxangle

  def angle(self, id, angle):
    did, center, inverted = self.check_id(id)
    self.check_sp()

    if abs(angle) > maxangle:
      raise ValueError("Steering angle {} exceeded expected maximum of {}}".format(angle,maxangle))

    if inverted:
      angle = angle * -1

    position = 2048 + (angle * 4096/360) # 0 min, 2048 center, 4096 max at 360 degrees

    self.send(did, 0x82, self.data2byte(position))
    self.read_ack()

  def steer_setzero(self, id):
    did, center, inverted = self.check_id(id)
    self.check_sp()
    # TODO: Support live adjustment

  def input_voltage(self, id):
    """
    Query DMFE controller's internal voltage monitor
    """
    did, center, inverted = self.check_id(id)
    self.check_sp()

    self.send(did, 0x96)

    resp = self.read_datapacket(did)

    return self.bytes_to_int(resp[0])/18.8

if __name__ == "__main__":
  """
  Command line interface to work with DMFE serial bus devices.
  Implements a subset of functionality as-needed
  """
  import argparse

  parser = argparse.ArgumentParser(description="DMFE Serial Bus Device Command Line Utility")

  parser.add_argument("-id", "--id", help="Device identifier integer 2-253, default is 2.", type=int, default=2)
  group = parser.add_mutually_exclusive_group()
  group.add_argument("-m", "--move", help="Move servo to specified position 0-4096", type=int)
  group.add_argument("-s", "--spin", help="Spin the motor at a specified speed from 0 to 50", type=int)
  group.add_argument("-v", "--voltage", help="Read current input voltage", action="store_true")
  args = parser.parse_args()

  c = dmfe_wrapper()
  c.connect()

  if args.move != None: # Explicit check against None because zero is a valid value
    if args.move < 0 or args.move > 4096:
      print("Move destination {} is outside valid range of 0 to 4096 (4096 = 360 degrees)".format(args.move))
    else:
      c.send(args.id,0xaa)
      c.read_dmfeserialservo()
      print("Moving device {} to position {}".format(args.id, args.move))
      c.send(args.id, 0x82, c.data2byte(args.move))
      c.read_ack()
  elif args.spin != None: # Zero is a valid parameter
    if args.spin < -50 or args.spin > 50:
      print("Spin speed {} is outside valid range of -50 to 50".format(args.spin))
    else:
      c.send(args.id,0xaa)
      c.read_dmfeserialbrushed()
      print("Spinning motor {} at speed {}".format(args.id, args.spin))
      c.send(args.id, 0x87, c.data1byte(args.spin))
      c.read_ack()
  elif args.voltage:
      c.send(args.id, 0x96)
      resp = c.read_datapacket(args.id)
      print("Device {} reports {} which translates to {} volts".format(args.id, c.bytes_to_int(resp[0]), c.bytes_to_int(resp[0])/18.8))
  else:
    # None of the actions were specified? Show help screen.
    parser.print_help()

  c.close()
