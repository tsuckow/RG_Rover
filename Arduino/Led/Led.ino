#include <RGBmatrixPanel.h>

#define CLK A4 // USE THIS ON METRO M4 (not M0)
#define OE   9
#define LAT 10
#define A   A0
#define B   A1
#define C   A2

// Last parameter = 'true' enables double-buffering, for flicker-free,
// buttery smooth animation.  Note that NOTHING WILL SHOW ON THE DISPLAY
// until the first call to swapBuffers().  This is normal.
RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, true);

int8_t ball[3][4] = {
  {  3,  0,  1,  1 }, // Initial X,Y pos & velocity for 3 bouncy balls
  { 17, 15,  1, -1 },
  { 27,  4, -1,  1 }
};

static const uint16_t PROGMEM ballcolor[3] = {
  0x0080, // Green=1
  0x0002, // Blue=1
  0x1000  // Red=1
};

void handleString(const char *str, uint16_t loops) {
  int16_t textX = matrix.width();
  int16_t textMin = (uint16_t)((strlen(str) + 1) * -12);
  loops = (strlen(str) + 1) * 12 * loops + textX * 3;
  uint16_t hue = 0;

  for (uint16_t l = 0; l < loops; l++) {
    // Clear background
    matrix.fillScreen(0);

    // Bounce three balls around
    for (uint8_t i = 0; i < 3; i++) {
      // Draw 'ball'
      matrix.fillCircle(ball[i][0], ball[i][1], 5, pgm_read_word(&ballcolor[i]));
      // Update X, Y position
      ball[i][0] += ball[i][2];
      ball[i][1] += ball[i][3];
      // Bounce off edges
      if ((ball[i][0] == 0) || (ball[i][0] == (matrix.width() - 1))) {
        ball[i][2] *= -1;
      }
      if ((ball[i][1] == 0) || (ball[i][1] == (matrix.height() - 1))) {
        ball[i][3] *= -1;
      }
    }

    // Draw big scrolly text on top
    matrix.setTextColor(matrix.ColorHSV(hue, 255, 255, true));
    matrix.setCursor(textX, 1);
    matrix.print(str);

    // Move text left (w/wrap), increase hue
    if ((--textX) < textMin) {
      textX = matrix.width();
    }

    hue += 7;
    if (hue >= 1536) {
      hue -= 1536;
    }

#if !defined(__AVR__)
    // On non-AVR boards, delay slightly so screen updates aren't too quick.
    delay(50);
#endif

    // Update display
    matrix.swapBuffers(false);
  }
}

class Feature {
  public:
  Feature(String style, int x, int y, uint16_t *c) {
    this->style = style;
    this->x = x;
    this->y = y;
    this->colors[0] = c[0];
    this->colors[1] = c[1];
  }

  void setStyle(String style) {
    this->style = style;
  }

  String getStyle() {
    return this->style;
  }

  void setCenter(int x, int y) {
    this->x = x;
    this->y =y;
  }

  void setX(int x) {
    this->x = x;
  }

  int getX() {
    return this->x;
  }

  void setY(int y) {
    this->y = y;
  }

  int getY() {
    return this->y;
  }

  void setColors(uint16_t *c) {
    this->colors[0] = c[0];
    this->colors[1] = c[1];
  }

  protected:
  String style;
  int x;
  int y;
  uint16_t colors[2];
};

class Eye: public Feature {
  public:
  Eye(String style, int x, int y, uint16_t *c)
  : Feature(style, x, y, c) {
    this->direct = 0;
    this->prev_style = "";
  }

  void setDirection(int d) {
    this->direct = d;
  }

  String getPrevStyle() {
    return this->prev_style;
  }

  void wink() {
    this->prev_style = this->style;
    this->style = 'wink';
    draw();
  }

  void draw() {
    matrix.fillRect(x - 2, y - 2, 5, 5, 0);
    if (style == "8bit") {
      matrix.fillRect(x - 1, y - 2, 3, 1, colors[0]);
      matrix.fillRect(x - 1, y + 2, 3, 1, colors[0]);
      matrix.fillRect(x - 2, y - 1, 1, 3, colors[0]);
      matrix.fillRect(x + 2, y - 1, 1, 3, colors[0]);
      matrix.fillRect(x - 1, y - 1, 3, 3, colors[1]);

      if (direct == 0) {
        matrix.fillRect(x - 1, y + 1, 3, 1, matrix.Color333(7, 7, 7));
        matrix.drawPixel(x, y, matrix.Color333(7, 7, 7));
      }
      else if (direct < 0) {
        matrix.fillRect(x, y, 2, 2, matrix.Color333(7, 7, 7));
        matrix.drawPixel(x, y, colors[1]);
      }
      else if (direct > 0) {
        matrix.fillRect(x - 1, y, 2, 2, matrix.Color333(7, 7, 7));
        matrix.drawPixel(x, y, colors[1]);
      }
    }
    else if (style == "wink") {
      matrix.drawPixel(x - 2, y, colors[0]);
      matrix.drawPixel(x - 1, y + 1, colors[0]);
      matrix.drawPixel(x - 1, y - 1, colors[0]);
      matrix.drawPixel(x, y - 2, colors[0]);
      matrix.drawPixel(x, y + 2, colors[0]);
    }
  }

  private:
  int direct;
  String prev_style;
};

class Mouth: public Feature {
  public:
  Mouth(String style, int x, int y, uint16_t *c)
  : Feature(style, x, y, c) {
  }

  void draw() {
    matrix.fillRect(x - 3, y, 5, 3, 0);
    
    if (style == "happy") {
      matrix.fillRect(x - 2, y + 1, 5, 1, colors[0]);
      matrix.drawPixel(x - 3, y, colors[0]);
      matrix.drawPixel(x + 3, y, colors[0]);
    }
    else {
      matrix.fillRect(x - 1, y + 1, 3, 1, colors[1]);
      matrix.drawPixel(x - 2, y + 2, colors[1]);
      matrix.drawPixel(x + 2, y + 2, colors[1]);
    }
  }
};

class EyeBrow: public Feature {
  public:
  EyeBrow(String style, int x, int y, uint16_t *c)
  : Feature(style, x, y, c) {
  }

  void draw() {
    matrix.fillRect(x - 2, y - 1, 5, 2, 0);
    
    if (style == "arc") {
      matrix.fillRect(x - 1, y - 1, 3, 1, colors[0]);
      matrix.drawPixel(x - 2, y, colors[0]);
      matrix.drawPixel(x + 2, y, colors[0]);
    }
    else {
      matrix.drawPixel(x, y, colors[1]);
      matrix.drawPixel(x - 1, y - 1, colors[1]);
      matrix.drawPixel(x + 1, y - 1, colors[1]);
    }
  }
};

class Cheek: public Feature {
  public:
  Cheek(String style, int x, int y, uint16_t *c)
  : Feature(style, x, y, c) {
  }

  void draw() {
    uint16_t c = colors[1];
    
    if (style == "circle") {
      c = colors[0];
    }
    
    matrix.drawPixel(x - 1, y - 2, c);
    matrix.drawPixel(x - 2, y, c);
    matrix.drawPixel(x - 1, y + 2, c);
    matrix.drawPixel(x + 1, y + 2, c);
    matrix.drawPixel(x + 2, y, c);
    matrix.drawPixel(x + 1, y - 2, c);
  }
};

class Connected: public Feature {
  public:
  Connected(String style, int x, int y, uint16_t *c)
  : Feature(style, x, y, c) {
    _connected = false;
  }

  void setConnected(bool st) {
    _connected = st;
  }
  
  void draw() {
    if (_connected) {
      matrix.drawPixel(x, y, colors[0]);
    }
    else {
      matrix.drawPixel(x, y, colors[1]);
    }
  }
  private:
  bool _connected;
};
  
Eye *left_eye;
Eye *right_eye;
Mouth *mouth;
EyeBrow *left_brow;
EyeBrow * right_brow;
Cheek *left_cheek;
Cheek *right_cheek;
Connected *con;

char input_buffer[100];
uint16_t buf_index = 0;

void setup() {
  uint16_t colors[2];

  Serial.begin(115200);

  for (int i = 0; i < sizeof(input_buffer); i++) {
    input_buffer[i] = '\0';
  }
  
  matrix.begin();
  matrix.setTextWrap(false); // Allow text to run off right edge
  matrix.setTextSize(2);

  colors[0] = matrix.Color333(0, 0, 6);
  colors[1] = matrix.Color333(0, 6, 0);
  left_eye = new Eye("8bit", 12, 6, colors);
  right_eye = new Eye("8bit", 20, 6, colors);

  colors[0] = matrix.Color333(6, 6, 6);
  colors[1] = matrix.Color333(0, 1, 1);
  left_brow = new EyeBrow("arc", 12, 2, colors);
  right_brow = new EyeBrow("arc", 20, 2, colors);
  
  colors[0] = matrix.Color333(2, 0, 0);
  colors[1] = matrix.Color333(0, 0, 0);
  left_cheek = new Cheek("circle", 7, 10, colors);
  right_cheek = new Cheek("circle", 25, 10, colors);
  
  colors[0] = matrix.Color333(6, 0, 0);
  colors[1] = matrix.Color333(1, 0, 1);
  mouth = new Mouth("happy", 16, 13, colors);

  colors[0] = matrix.Color333(0, 7, 0);
  colors[1] = matrix.Color333(7, 0, 0);
  con = new Connected("", 31, 0, colors);

  draw_face();
}

void draw_face() {
  matrix.fillScreen(0);
  left_eye->draw();
  right_eye->draw();
  left_brow->draw();
  right_brow->draw();
  left_cheek->draw();
  right_cheek->draw();
  mouth->draw();
  con->draw();
  matrix.swapBuffers(false);
}

void happy() {
  left_brow->setStyle("arc");
  right_brow->setStyle("arc");
  left_cheek->setStyle("circle");
  right_cheek->setStyle("circle");
  mouth->setStyle("happy");
}

void frown() {
  left_brow->setStyle("");
  right_brow->setStyle("");
  left_cheek->setStyle("");
  right_cheek->setStyle("");
  mouth->setStyle("");
}

void runCommand() {
  // Serial.print(input_buffer[0]); Serial.print(' '); Serial.print(input_buffer[1]); Serial.println("<<");
  
  switch (input_buffer[0]) {
    case 'c':
        if (input_buffer[1] == 't') {
          con->setConnected(true);
        }
        else {
          con->setConnected(false);
        }
        break;
    case 'e':
        switch (input_buffer[1]) {
          case 'l':
              left_eye->setDirection(-1);
              right_eye->setDirection(-1);
              break;
          case 'r':
              left_eye->setDirection(1);
              right_eye->setDirection(1);
              break;
          default:
              left_eye->setDirection(0);
              right_eye->setDirection(0);
        }
        break;
    case 'f':
        switch (input_buffer[1]) {
          case 'f':
              frown();
              break;
          case 'h':
          default:
              happy();
        }
        break;
    case 't':
        uint16_t l = input_buffer[1] - 0x30;
        handleString(input_buffer + 2, l);
      break;
  }
  draw_face();
  Serial.println("OK");
}

void loop() {
  while (Serial.available() > 0) {
    char chr = Serial.read();

    // Terminate a command with a CR
    if (chr == 13) {
      input_buffer[buf_index] = '\0';

      runCommand();

      for (int i = 0; i < sizeof(input_buffer); i++) {
        input_buffer[i] = '\0';
      }
      buf_index = 0;
    }
    else {
      input_buffer[buf_index++] = chr;
    }
  }
  
  /*
  handleString("Hello Maker Faire - Bay Area 2019 ", 3);
  matrix.fillScreen(0);
  matrix.swapBuffers(false);
  delay(10000);

  handleString("Sawppy - The Rover ;-) ", 3);
  matrix.fillScreen(0);
  matrix.swapBuffers(false);
  delay(10000);

  left_eye->setDirection(0);
  right_eye->setDirection(0);
  happy();
  delay(10000);

  frown();
  delay(10000);
  */

  delay(100);
}

