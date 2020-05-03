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

#define max_page 5
int pixel_x, pixel_y;

struct BTN {
  int top, bot, lft, rgt;
  int w, h;
} homeb, infoback, inforgt, infolft;

uint32_t pressCD = 0;

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

void info_screen()
{
  tft.fillRect(0, 0, 320, 240, BLACK);
  uint8_t scr_index = 1;
  while (1)
  {
    char* scr_str = "hanoix.bmp";
    scr_str[5] = scr_index + '0';
    bmpDraw(infoback, scr_str, 0, 0);
    bmpDraw(homeb, "home.bmp",MAX_X-46, 13);
    if(scr_index!=max_page)
    bmpDraw(inforgt, "right.bmp", MAX_X - 37 - 15, MAX_Y - 45);
    if(scr_index!=1)
    bmpDraw(infolft, "left.bmp", 15, MAX_Y - 45);
    Serial.println(scr_str);
    while (1)
    {
      if (Touch_getXY())
      {
        {
          if (is_pressed(homeb))
          {
            load_home();
            break;
          }
          if (is_pressed(inforgt) && scr_index < max_page)
          {
            scr_index++;
            break;
          }
          if (is_pressed(infolft) && scr_index > 1)
          {
            scr_index--;
            break;
          }
        }
      }
    }
  }
}

void load_home()
{
  Serial.write("MAINMEN.HEX");
  delay(1000);
  while (1)
  {}
}

void loop()//////////////////////////////////////////////////////////////////////////////////////////
{
  info_screen(); 
}//\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

bool Touch_getXY(void)
{
  TSPoint p = ts.getPoint();
  //p.x = 1048-p.x;
  // p.y -= 142;
  pinMode(YP, OUTPUT);      //restore shared pins
  pinMode(XM, OUTPUT);
  digitalWrite(YP, HIGH);   //because TFT control pins
  digitalWrite(XM, HIGH);
  bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
  if (pressed) {
    pixel_x = map(p.y, TS_LEFT, TS_RT, 0, tft.width()); //.kbv makes sense to me
    pixel_y = map(p.x, TS_TOP, TS_BOT, 0, tft.height());
    /* Serial.print(pixel_x);
      Serial.print("      ");
      Serial.print(pixel_y);
      Serial.println();*/
  }
  return pressed;
}


#define BUFFPIXEL 20
void bmpDraw(BTN & btn, char *filename, int x, int y) { //modified library example function

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
