#define USE_BASE      // Enable the base controller code

/* Serial port baud rate */
#define BAUDRATE     115200

#define MAX_PWM        255

/* Include definition of serial commands */
#include "commands.h"

#ifdef USE_BASE
#define NUM_MOTORS 6

/* Motor driver function definitions */
#include "motor_driver.h"

/* Encoder driver function definitions */
#include "encoder_driver.h"

/* PID parameters and functions */
#include "diff_controller.h"

/* Run the PID loop at 30 times per second */
#define PID_RATE           30     // Hz

/* Convert the rate into an interval */
const int PID_INTERVAL = 1000 / PID_RATE;

/* Track the next time we make a PID calculation */
unsigned long nextPID = PID_INTERVAL;

/* Stop the robot if it hasn't received a movement command
   in this number of milliseconds */
#define AUTO_STOP_INTERVAL 2000
long lastMotorCommand = AUTO_STOP_INTERVAL;
#endif

/* Variable initialization */

/* Run a command.  Commands are defined in commands.h */
void runCommand(char cmd, int argc, char *argv[]) {
  int i = 0;
  char *p = argv[0];
  char *str;
  int pid_args[4];

  switch(cmd) {
  case GET_BAUDRATE:
    Serial.println(BAUDRATE);
    break;

#ifdef USE_BASE
  case READ_ENCODERS:
    for (int i = 0; i < NUM_MOTORS; i++) {
      if (i > 0) {
        Serial.print(" ");
      }

      Serial.print(readEncoder(i));
    }
    Serial.println();
    break;
   case RESET_ENCODERS:
    resetEncoders();
    resetPID();
    Serial.println("OK");
    break;
  case MOTOR_SPEEDS: {
      /* Reset the auto stop timer */
      lastMotorCommand = millis();
      bool not_moving = true;

      for (int i = 0; i < NUM_MOTORS; i++) {
        pids[i].TargetTicksPerFrame = atoi(argv[i]);
        not_moving = not_moving && (pids[i].TargetTicksPerFrame == 0);
      }
      if (not_moving) {
        for (int i = 0; i < NUM_MOTORS; i++) {
          setMotorSpeed(i, 0);
        }
        moving = 0;
      }
      else {
        moving = 1;
      }
    }
    Serial.println("OK");
    break;
  case SINGLE_MOTOR_SPEED: {
      /* Reset the auto stop timer */
      lastMotorCommand = millis();
      int i = atoi(argv[0]);
      pids[i].TargetTicksPerFrame = atoi(argv[1]);

      if (pids[i].TargetTicksPerFrame == 0) {
        setMotorSpeed(i, 0);
      }
      else {
        moving = 1;
      }
    }
    Serial.println("OK");
    break;
  case UPDATE_PID:
    while ((str = strtok_r(p, ":", &p)) != '\0') {
       pid_args[i] = atoi(str);
       i++;
    }
    Kp = pid_args[0];
    Kd = pid_args[1];
    Ki = pid_args[2];
    Ko = pid_args[3];
    Serial.println("OK");
    break;
#endif
  default:
    Serial.println("Invalid Command");
    break;
  }
}

/* Setup function--runs once at startup. */
void setup() {
  Serial.begin(BAUDRATE);
  while (Serial.available() == 0) {
    // do nothing
  }

  // Initialize the motor controller if used */
#ifdef USE_BASE
  initEncoder();
  initMotorController();
  resetPID();
#endif
}

/* Enter the main loop.  Read and parse input from the serial port
   and run any valid commands. Run a PID calculation at the target
   interval and check for auto-stop conditions.
*/
void loop() {
  static int argc = 0;
  static int arg_index = 0;
  static char argv_buffer[NUM_MOTORS][16];
  static char *argv[NUM_MOTORS];
  static char chr;
  static char cmd = 0;

  if (argv[0] == NULL) {
    for (int i = 0; i < NUM_MOTORS; i++) {
      argv[i] = &argv_buffer[i][0];
    }
  }
  while (Serial.available() > 0) {
    // Read the next character
    chr = Serial.read();

    // Terminate a command with a CR
    if (chr == 13) {
      argv[argc][arg_index] = '\0';
      runCommand(cmd, argc + 1, argv);

      for (int i = 0; i < NUM_MOTORS; i++) {
        memset(argv[i], 0, sizeof(argv[i]));
      }

      argc = 0;
      arg_index = 0;
      cmd = 0;
    }
    // Use spaces to delimit parts of the command
    else if (chr == ' ') {
      if (arg_index > 0) {
        argv[argc][arg_index] = '\0';
        arg_index = 0;
        argc++;

        if (argc >= NUM_MOTORS) {
          argc = NUM_MOTORS - 1;
        }
      }
    }
    else {
      if (cmd == 0) {
        // The first arg is the single-letter command
        cmd = chr;
      }
      else {
        // Subsequent arguments can be more than one character
        argv[argc][arg_index++] = chr;
      }
    }
  }

  // If we are using base control, run a PID calculation at the appropriate intervals
#ifdef USE_BASE
  if (millis() > nextPID) {
    updatePID();
    nextPID += PID_INTERVAL;
  }

  // Check to see if we have exceeded the auto-stop interval
  if ((millis() - lastMotorCommand) > AUTO_STOP_INTERVAL) {
    moving = 0;
    resetPID();

    for (int i = 0; i < NUM_MOTORS; i++) {
      setMotorSpeed(i, 0);
    }
  }
#endif
}

