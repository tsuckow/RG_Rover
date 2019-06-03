#define NUM_ENC 6

volatile long enc_pos[NUM_ENC];

static const int8_t ENC_STATES [] = {0,1,-1,0,-1,0,0,1,1,0,0,-1,0,-1,1,0};  //encoder lookup table

#define ENC_1A_A_PIN 5
#define ENC_1A_B_PIN 6
#define ENC_1B_A_PIN 7
#define ENC_1B_B_PIN 8

#define ENC_2A_A_PIN 25
#define ENC_2A_B_PIN 26
#define ENC_2B_A_PIN 27
#define ENC_2B_B_PIN 28

#define ENC_3A_A_PIN 29
#define ENC_3A_B_PIN 30
#define ENC_3B_A_PIN 31
#define ENC_3B_B_PIN 32

static void enc_1a_isr() {
  static uint8_t enc_last = 0;

  bool a = digitalRead(ENC_1A_A_PIN);
  bool b = digitalRead(ENC_1A_B_PIN);

  enc_last <<= 2;
  enc_last |= ((uint8_t)a) << 1 | (uint8_t)b;

  enc_pos[0] += ENC_STATES[(enc_last & 0x0f)];
}

static void enc_1b_isr() {
  static uint8_t enc_last = 0;

  bool a = digitalRead(ENC_1B_A_PIN);
  bool b = digitalRead(ENC_1B_B_PIN);

  enc_last <<= 2;
  enc_last |= ((uint8_t)a) << 1 | (uint8_t)b;

  enc_pos[1] += ENC_STATES[(enc_last & 0x0f)];
}

static void enc_2a_isr() {
  static uint8_t enc_last = 0;

  bool a = digitalRead(ENC_2A_A_PIN);
  bool b = digitalRead(ENC_2A_B_PIN);

  enc_last <<= 2;
  enc_last |= ((uint8_t)a) << 1 | (uint8_t)b;

  enc_pos[2] += ENC_STATES[(enc_last & 0x0f)];
}

static void enc_2b_isr() {
  static uint8_t enc_last = 0;

  bool a = digitalRead(ENC_2B_A_PIN);
  bool b = digitalRead(ENC_2B_B_PIN);

  enc_last <<= 2;
  enc_last |= ((uint8_t)a) << 1 | (uint8_t)b;

  enc_pos[3] += ENC_STATES[(enc_last & 0x0f)];
}

static void enc_3a_isr() {
  static uint8_t enc_last = 0;

  bool a = digitalRead(ENC_3A_A_PIN);
  bool b = digitalRead(ENC_3A_B_PIN);

  enc_last <<= 2;
  enc_last |= ((uint8_t)a) << 1 | (uint8_t)b;

  enc_pos[4] += ENC_STATES[(enc_last & 0x0f)];
}

static void enc_3b_isr() {
  static uint8_t enc_last = 0;

  bool a = digitalRead(ENC_3B_A_PIN);
  bool b = digitalRead(ENC_3B_B_PIN);

  enc_last <<= 2;
  enc_last |= ((uint8_t)a) << 1 | (uint8_t)b;

  enc_pos[5] += ENC_STATES[(enc_last & 0x0f)];
}

void initEncoder() {
  for (int i = 0; i < NUM_ENC; i++) {
    enc_pos[i] = 0;
  }

  pinMode(ENC_1A_A_PIN, INPUT_PULLUP);
  pinMode(ENC_1A_B_PIN, INPUT_PULLUP);
  pinMode(ENC_1B_A_PIN, INPUT_PULLUP);
  pinMode(ENC_1B_B_PIN, INPUT_PULLUP);

  pinMode(ENC_2A_A_PIN, INPUT_PULLUP);
  pinMode(ENC_2A_B_PIN, INPUT_PULLUP);
  pinMode(ENC_2B_A_PIN, INPUT_PULLUP);
  pinMode(ENC_2B_B_PIN, INPUT_PULLUP);

  pinMode(ENC_3A_A_PIN, INPUT_PULLUP);
  pinMode(ENC_3A_B_PIN, INPUT_PULLUP);
  pinMode(ENC_3B_A_PIN, INPUT_PULLUP);
  pinMode(ENC_3B_B_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ENC_1A_A_PIN), enc_1a_isr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_1A_B_PIN), enc_1a_isr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_1B_A_PIN), enc_1b_isr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_1B_B_PIN), enc_1b_isr, CHANGE);

  attachInterrupt(digitalPinToInterrupt(ENC_2A_A_PIN), enc_2a_isr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_2A_B_PIN), enc_2a_isr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_2B_A_PIN), enc_2b_isr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_2B_B_PIN), enc_2b_isr, CHANGE);

  attachInterrupt(digitalPinToInterrupt(ENC_3A_A_PIN), enc_3a_isr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_3A_B_PIN), enc_3a_isr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_3B_A_PIN), enc_3b_isr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_3B_B_PIN), enc_3b_isr, CHANGE);

}

long readEncoder(int i) {
  if (i < 0 || i >= NUM_ENC) {
    return 0;
  }
  
  return enc_pos[i];
}

void resetEncoder(int i) {
  if (i < 0 || i >= NUM_ENC) {
    return;
  }

  enc_pos[i] = 0;
}

void resetEncoders() {
  for (int i = 0; i < NUM_ENC; i++) {
    resetEncoder(i);
  }
}
