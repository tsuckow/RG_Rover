import datetime
import logging
import serial
from struct import *

import configuration

class teensy_motors_wrapper:
  """
  Class that implements the rover motor control methods for serial bus
  based on a Teensy 3.5 running a modified ROS Arduino Bridge
  """
  def __init__(self):
    self.sp = None

  def check_sp(self):
    """ Raises error if we haven't opened serial port yet. """
    if self.sp == None:
      raise ValueError("Teensy serial communication is not available.")

  def connect(self):
    """
    Read serial port connection parameters from JSON configuration file
    and open the port.
    """

    # Read parameter file
    config = configuration.configuration("teensy_motors")
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

  def version(self, id):
    """ Identifier string for this motor controller """
    return "Teensy Motors"

  @staticmethod
  def check_id(id):
    """ Verifies servo ID is within range and inverted status is boolean"""
    if not isinstance(id, (tuple,list)):
      raise ValueError("Teensy Motors identifier must be a tuple")

    if not isinstance(id[0], int):
      raise ValueError("Teensy Motors address must be an integer")

    if id[0] < 0 or id[0] > 5:
      raise ValueError("Teensy Motors address {} outside of valid range 0-5".format(id[0]))

    if not isinstance(id[1], bool):
      raise ValueError("Inverted status must be a boolean")

    return tuple(id)

  def power_percent(self, id, percentage):
    """ Runs motor at specified +/- percentage """
    mid, inverted = self.check_id(id)
    self.check_sp()

    pct = int(percentage)
    if abs(pct) > 100:
      raise ValueError("Motor power percentage {0} outside valid range from 0 to 100.".format(pct))

    # The Teensy expects +/- 255 
    power = int(percentage * 2.54)

    if inverted:
      power = power * -1

    # logging.getLogger('werkzeug').error("%s: teensy_motors.power_percent(%d, %f)  -> write(%d, %d)" % (datetime.datetime.utcnow().strftime("%H%M%S.%f"), mid, percentage, mid, power))
    self.sp.write("n %d %d\r" % (mid, power))

  def set_max_current(self, id, current):
    sid, center, inverted = self.check_id(id)
    self.check_sp()
    # Does nothing

  def init_velocity(self, id):
    sid, inverted = self.check_id(id)
    self.check_sp()
    # Does nothing

  def velocity(self,id,pct_velocity):
    """
    Runs the specified servo in motor mode at specified velocity
    In case of LewanSoul servos, it is the same as power_percent.
    """
    self.power_percent(id,pct_velocity)

  def init_angle(self, id):
    pass

  def maxangle(self, id):
    return 0

  def angle(self, id, angle):
    pass

  def steer_setzero(self, id):
    pass

  def input_voltage(self, id):
    return 0

