import datetime
import logging
import re
import serial
import sys
import termios
import threading
import time
import tty

class RCReader(threading.Thread):
    ser = None
    start_called = False
    chassis = None

    def __init__(self, group = None, target = None, name = None,
                 args = (), kwargs = None, verbose = None):
        threading.Thread.__init__(self, group=group, target = target, name = name,
                                  verbose = verbose)
        self.args = args
        self.kwargs = kwargs
        self.chassis = args[0]

        return

    def start(self):
        if self.start_called:
            raise RunTimeError

        self.start_called = True
        self.fin = serial.Serial('/dev/ttyACM0', 115200, timeout = 1)


        i = 0                              # Just read some lines
        line = self.fin.readline()
        while line and i < 10:
            line = self.fin.readline()
            i += 1

        super(RCReader, self).start()

    def run(self):
        last_time = datetime.datetime(1970, 1, 1)
        rc_use_input_time = datetime.datetime(1970, 1, 1)
        send_stop = 0

        while True:
            line = self.fin.readline()

            m = re.match('I ([-\d.]+) +([-\d.]+) +([-\d.]+) +([-\d.]+) +([-\d.]+) +([-\d.]+) +([-\d.]+) +([-\d.]+)', line)
            if m != None:
                t = datetime.datetime.utcnow()

                if (t - last_time).total_seconds() > 0.05:
                    last_time = t
                    angle = (float(m.group(1)) - 1500.0) / 5.0
                    throttle = (float(m.group(3)) - 1500.0) / 5.0
                    rc_use = float(m.group(5)) - 1500.0
                    
                    if rc_use < 0.0:
                        self.chassis.use_rc_input = False
                        rc_use_input_time =  datetime.datetime(1970, 1, 1)
                    else:
                        rc_use_input_time = datetime.datetime.utcnow()
                        self.chassis.use_rc_input = True

                        if angle > 100.0:
                            angle = 100.0
                        elif angle < - 100.0:
                            angle = -100.0

                        if throttle > 100.0:
                            throttle = 100.0
                        elif throttle < -100.0:
                            throttle = -100.0

                        # print("angle: %f  throttle: %f" % (angle, throttle))
                        radius_inf = False

                        if angle >= -6.0 and angle <= 6.0:
                            radius = float("inf")
                            radius_inf = True
                        elif angle > 1.0:
                            radius = self.chassis.minRadius + (self.chassis.maxRadius - self.chassis.minRadius) * (100.0 - angle) / 100.0
                        else:
                            radius = - self.chassis.minRadius - (self.chassis.maxRadius - self.chassis.minRadius) * (100.0 + angle) / 100.0

                        if throttle >= -5.0 and throttle <= 5.0:
                            throttle = 0.0

                        # logging.getLogger('werkzeug').error("run(%f, %f, %d, %d)" % (throttle, radius, int(radius_inf), send_stop))
                        if throttle != 0.0 or radius_inf == False:
                            send_stop = 0

                            self.chassis.ensureready()
                            self.chassis.move_velocity_radius(throttle, radius)
                        elif send_stop < 1:
                            send_stop = 1

                            self.chassis.ensureready()
                            self.chassis.move_velocity_radius(throttle, radius)

            if (datetime.datetime.utcnow() - rc_use_input_time).total_seconds() > 2.0:
                self.chassis.use_rc_input = False


# t = RCReader()
# t.setDaemon(True)
# t.start()

# class Getch:
#     def __call__(self):
#         fd = sys.stdin.fileno()
#         old_settings = termios.tcgetattr(fd)
#         try:
#             tty.setraw(sys.stdin.fileno())
#             ch = sys.stdin.read(1)
#         finally:
#             termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
#         return ch
# 
# 
# getch = Getch()
# c = getch().lower()
# while c != 'q':
#     time.sleep(100)
