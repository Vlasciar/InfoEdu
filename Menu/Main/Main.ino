//#include <SPFD5408_Adafruit_GFX.h>    // Core graphics library
#include <SPFD5408_Adafruit_TFTLCD.h> // Hardware-specific library
#include <SPI.h>
#include <SD.h>
#include <SPFD5408_TouchScreen.h>


#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)

#define SD_CS 53     // Set the chip select line to whatever you use
#else
#define SD_CS 10     // Set the chip select line to whatever you use (10 doesnt conflict with the library)
#endif

#define XP 8
#define XM A2
#define YP A3
#define YM 9


#define TS_LEFT 135
#define TS_RT 925
#define TS_TOP 127
#define TS_BOT 907

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, A4);
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

#define MINPRESSURE 200
#define MAXPRESSURE 1000

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF


#define MAX_X 320
#define MAX_Y 240


int pixel_x, pixel_y;

struct BTN {
  uint16_t top, bot, lft, rgt;
  uint16_t w, h;
} btn1, btn2, btn3, btn4, btn5, btn6, up, down;

uint8_t scroll_index = 0;
char* menushex[] = {"SELSORT.HEX","BBLSORT.HEX",
                    "EUCLIDA.HEX","RAYCAST.HEX",
                    "TOHANOI.HEX","PRIMALG.HEX",
                    "DIJKSTR.HEX","ASTARPFA.HEX"
                   };
char* menus[] = {"Selection Sort", "Bubble Sort",
                  "Euclidean Alg.","Raycasting",
                  "Hanoi","Prim's Alg.",
                  "Dijkstra's Alg.+","A*"
                };
uint8_t options;
uint32_t pressCD = 0;

bool cangoup = false, cangodown = true;
void setup()
{
  Serial.begin(9600);
  tft.reset();
  uint16_t identifier = tft.readID();
  tft.begin(identifier);

  if (!SD.begin(SD_CS)) {
    return;
  }
  tft.fillScreen(0);
  tft.setRotation(1);

  options = sizeof(menus) / sizeof(menus[0]);
  draw_ui();
}

void draw_ui()
{
  tft.fillRect(0, 0, 320, 240, BLACK);
  tft.setTextSize(5);
  tft.setTextColor(RED);
  tft.setCursor(50, 5);
  tft.println("Name");
  tft.setCursor(50, 50);
  tft.println("here");

  if ((5 + 2 * scroll_index - (((scroll_index + 2) * 2 + 1) % 6)) < options)
    bmpDraw(btn1, "menu1.bmp", 5, 200 - ((50 * (scroll_index + 2)) % 150));
  if ((5 + 2 * scroll_index - (((scroll_index + 1) * 2 + 1) % 6)) < options)
    bmpDraw(btn2, "menu2.bmp", 5, 200 - ((50 * (scroll_index + 1)) % 150));
  if ((5 + 2 * scroll_index - (((scroll_index) * 2 + 1) % 6)) < options)
    bmpDraw(btn3, "menu3.bmp", 5, 200 - ((50 * (scroll_index)) % 150));

  if ((5 + 2 * scroll_index - (((scroll_index + 2) * 2) % 6)) < options)
    bmpDraw(btn4, "menu1.bmp", 140, 200 - ((50 * (scroll_index + 2)) % 150));
  if ((5 + 2 * scroll_index - (((scroll_index + 1) * 2) % 6)) < options)
    bmpDraw(btn5, "menu2.bmp", 140, 200 - ((50 * (scroll_index + 1)) % 150));
  if ((5 + 2 * scroll_index - (((scroll_index) * 2) % 6)) < options)
    bmpDraw(btn6, "menu3.bmp", 140, 200 - ((50 * (scroll_index)) % 150));

  uint8_t i = 2 * scroll_index;
  tft.setTextSize(1);
  tft.setTextColor(WHITE);

  for (uint8_t y = 0; y < 3; y++)
  {
    for (uint8_t x = 0; x < 2; x++)
    {
      if (i >= options) break;
      tft.setCursor(8 + x * 140, 110 + y * 50);
      tft.print(i + 1);
      tft.print(")");
      tft.print(menus[i]);
      i++;
    }
  }

  if (scroll_index != 0)
  {
    bmpDraw(up, "up.bmp", MAX_X - 45, 5);
    cangoup = true;
  }
  else cangoup = false;

  if (2 * scroll_index + 6 <= options)
  {
    cangodown = true;
    bmpDraw(down, "down.bmp", MAX_X - 45, MAX_Y - 45);
  }
  else cangodown = false;

}

bool is_pressed(BTN btn)
{
  if (pixel_x > btn.lft && pixel_x < btn.rgt && pixel_y < btn.bot && pixel_y > btn.top)
  {
    pressCD = 0;
    return true;
  }
  return false;
}

void load(int i)
{
  Serial.write(menushex[i]);
  delay(1000);
  while (1)
  {}
}
void check_presses(int CD)
{
  if (pressCD > CD)
  {
    if (Touch_getXY())
    {
      if (cangoup && is_pressed(up))
      {
        scroll_index--;
        draw_ui();
      }
      if (cangodown && is_pressed(down))
      {
        scroll_index++;
        draw_ui();
      }
      if (is_pressed(btn1))
      {
        uint8_t i = (5 + 2 * scroll_index - (((scroll_index + 2) * 2 + 1) % 6));
        load(i);
      }
      if (is_pressed(btn2))
      {
        uint8_t i = (5 + 2 * scroll_index - (((scroll_index + 1) * 2 + 1) % 6));
        load(i);
      }
      if (is_pressed(btn3))
      {
        uint8_t i = (5 + 2 * scroll_index - (((scroll_index) * 2 + 1) % 6));
        load(i);
      }
      if (is_pressed(btn4))
      {
        uint8_t i = (5 + 2 * scroll_index - (((scroll_index + 2) * 2) % 6));
        load(i);
      }
      if (is_pressed(btn5))
      {
        uint8_t i = (5 + 2 * scroll_index - (((scroll_index + 1) * 2) % 6));
        load(i);
      }
      if (is_pressed(btn6))
      {
        uint8_t i = (5 + 2 * scroll_index - (((scroll_index) * 2) % 6));
        load(i);
      }
    }
  }
  if (pressCD < 30000)
    pressCD++;
}

void loop()//////////////////////////////////////////////////////////////////////////////////////////
{
  while (1)
  {
    check_presses(25000);
  }
}//\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

bool Touch_getXY(void)
{
  TSPoint p = ts.getPoint();
  pinMode(YP, OUTPUT);      //restore shared pins
  pinMode(XM, OUTPUT);
  digitalWrite(YP, HIGH);   //because TFT control pins
  digitalWrite(XM, HIGH);
  bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
  if (pressed) {
    pixel_x = map(p.y, TS_LEFT, TS_RT, 0, tft.width()); //.kbv makes sense to me
    pixel_y = map(p.x, TS_TOP, TS_BOT, 0, tft.height());
  }
  return pressed;
}


#define BUFFPIXEL 20
void bmpDraw(BTN& btn, char *filename, int x, int y) {//modified library example function

  File     bmpFile;
  int      bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t rowSize;               // Not always = bmpWidth; may have padding
  uint8_t  sdbuffer[3 * BUFFPIXEL]; // pixel in buffer (R+G+B per pixel)
  uint16_t lcdbuffer[BUFFPIXEL];  // pixel out buffer (16-bit per pixel)
  uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  int      w, h, row, col;
  uint8_t  r, g, b;
  uint32_t pos = 0, startTime = millis();
  uint8_t  lcdidx = 0;
  boolean  first = true;
  btn.lft = x;
  btn.top = y;
  if ((x >= tft.width()) || (y >= tft.height())) return;

  if ((bmpFile = SD.open(filename)) == NULL) {
    return;
  }

  // Parse BMP header
  if (read16(bmpFile) == 0x4D42) { // BMP signature
    (void)read32(bmpFile);// Read & ignore bytes
    (void)read32(bmpFile); // Read & ignore bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
    (void)read32(bmpFile);// Read & ignore bytes

    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if (read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
      if ((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed

        goodBmp = true; // Supported BMP format -- proceed!

        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * 3 + 3) & ~3;

        // If bmpHeight is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if (bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip      = false;
        }

        // Crop area to be loaded
        w = bmpWidth;
        h = bmpHeight;
        btn.w = w;
        btn.h = h;
        btn.rgt = btn.lft + w;
        btn.bot = btn.top + h;
        if ((x + w - 1) >= tft.width())  w = tft.width()  - x;
        if ((y + h - 1) >= tft.height()) h = tft.height() - y;

        // Set TFT address window to clipped image bounds
        tft.setAddrWindow(x, y, x + w - 1, y + h - 1);

        for (row = 0; row < h; row++) {
          if (flip)
            pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
          else     // Bitmap is stored top-to-bottom
            pos = bmpImageoffset + row * rowSize;
          if (bmpFile.position() != pos) { // Need seek?
            bmpFile.seek(pos);
            buffidx = sizeof(sdbuffer); // Force buffer reload
          }

          for (col = 0; col < w; col++) { // For each column...
            // Time to read more pixel data?
            if (buffidx >= sizeof(sdbuffer)) { // Indeed
              // Push LCD buffer to the display first
              if (lcdidx > 0) {
                tft.pushColors(lcdbuffer, lcdidx, first);
                lcdidx = 0;
                first  = false;
              }
              bmpFile.read(sdbuffer, sizeof(sdbuffer));
              buffidx = 0; // Set index to beginning
            }

            // Convert pixel from BMP to TFT format
            b = sdbuffer[buffidx++];
            g = sdbuffer[buffidx++];
            r = sdbuffer[buffidx++];
            lcdbuffer[lcdidx++] = tft.color565(r, g, b);
          } // end pixel
        } // end scanline
        // Write any remaining data to LCD
        if (lcdidx > 0) {
          tft.pushColors(lcdbuffer, lcdidx, first);
        }
      } // end goodBmp
    }
  }

  bmpFile.close();
  if (!goodBmp) progmemPrintln(PSTR("BMP format not recognized."));
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(File f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}

// Copy string from flash to serial port
// Source string MUST be inside a PSTR() declaration!
void progmemPrint(const char *str) {

}

// Same as above, with trailing newline
void progmemPrintln(const char *str) {

}
