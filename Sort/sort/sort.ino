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

Adafruit_GFX_Button on_btn, off_btn;

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
  int top, bot, lft, rgt;
  int w, h;
} homeb, infob, pauseb, playb, parb;

int pressCD = 0;

uint8_t l_array[250];
uint8_t l_size = 100;
uint8_t l_scale;

bool isRunning = false;
bool isShowingPar = false;
bool isShowingInfo = false;

void setup()
{
  Serial.begin(9600);
  tft.reset();
  uint16_t identifier = tft.readID();
  tft.begin(identifier);

  progmemPrint(PSTR("Initializing SD card..."));
  if (!SD.begin(SD_CS)) {
    progmemPrintln(PSTR("failed!"));
    return;
  }
  progmemPrintln(PSTR("OK!"));
  tft.fillScreen(0);
  tft.setRotation(1);

}


void draw_ui()
{

  bmpDraw(homeb, "home.bmp", MAX_X - 45, 5);
  bmpDraw(infob, "info.bmp", MAX_X - 45, 45);
  bmpDraw(parb, "par.bmp", MAX_X - 45, 85);
  isRunning != isRunning;
  if (isRunning) bmpDraw(pauseb, "pause.bmp", MAX_X - 45, MAX_Y - 45);
  else bmpDraw(playb, "play.bmp", MAX_X - 45, MAX_Y - 45);


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

void l_setup()
{
  for (int i = 0; i < l_size; i++)
    l_array[i] = (random(1, 240));
  l_scale = 270 / l_size;
}

void l_update()
{
  tft.drawRect(0, 0, 250, 240, BLACK);
  for (int i = 0; i < l_size; i++)
    tft.drawRect(l_scale * i, MAX_Y - l_array[i], l_scale, l_array[i], WHITE);
}

BTN par_sz, par_min, par_max, par_rand, par_rev;
bool p_random = true;
bool p_reversed = false;
void draw_param()
{
  tft.fillRect(0, 0, 270, 240, BLACK);
  tft.drawRect(par_sz.lft, par_sz.top, par_sz.w, par_sz.h, GREEN);
  tft.drawRect(par_min.lft, par_min.top, par_min.w, par_min.h, GREEN);
  tft.drawRect(par_max.lft, par_max.top, par_max.w, par_max.h, GREEN);
  tft.drawRect(par_rand.lft, par_rand.top, par_rand.w, par_rand.h, GREEN);
  tft.drawRect(par_rev.lft, par_rev.top, par_rev.w, par_rev.h, GREEN);
  
  tft.setTextSize(2);
  tft.setCursor(25, 25);
  tft.println("Array size:");
  tft.setCursor(25, 75);
  tft.println("Min value:");
  tft.setCursor(25, 125);
  tft.println("Max value:");

  tft.setTextSize(1);
  tft.setCursor(75, 175);
  tft.println("Random:");
  tft.setCursor(150, 175);
  tft.println("Reversed:");

  tft.setCursor(5, 50);
  tft.println("100");
  tft.setCursor(225, 50);
  tft.println("250");

  tft.setCursor(5, 100);
  tft.println("0");
  tft.setCursor(225, 100);
  tft.println("120");

  tft.setCursor(5-, 150);
  tft.println("120");
  tft.setCursor(225, 150);
  tft.println("240");

  tft.setTextSize(5);

  tft.setCursor(83, 187);
  if (p_random)
    tft.println("X");
  tft.setCursor(162, 187);
  if (p_reversed)
    tft.println("X");
}
void par_screen()
{
  tft.fillRect(0, 0, 320, 240, BLACK);
  bmpDraw(parb, "par.bmp", MAX_X - 45, MAX_Y - 50);
  bmpDraw(homeb, "home.bmp", MAX_X - 45, 5);
  bmpDraw(infob, "info.bmp", MAX_X - 45, 45);
  bool first = true;
  par_sz.top = 50;
  par_sz.bot = 60;
  par_sz.lft = 30;
  par_sz.rgt = 220;
  par_sz.w = par_sz.rgt - par_sz.lft;
  par_sz.h = par_sz.bot - par_sz.top;
  
  par_min.top = 100;
  par_min.bot = 110;
  par_min.lft = 30;
  par_min.rgt = 220;
  par_min.w = par_min.rgt - par_min.lft;
  par_min.h = par_min.bot - par_min.top; 

  par_max.top = 150;
  par_max.bot = 160;
  par_max.lft = 30;
  par_max.rgt = 220;
  par_max.w = par_max.rgt - par_max.lft;
  par_max.h = par_max.bot - par_max.top;
  
  par_rand.top = 185;
  par_rand.bot = 225;
  par_rand.lft = 75;
  par_rand.rgt = 115;
  par_rand.w = par_rand.rgt - par_rand.lft;
  par_rand.h = par_rand.bot - par_rand.top;
  
  par_rev.top = 185;
  par_rev.bot = 225;
  par_rev.lft = 155;
  par_rev.rgt = 195;
  par_rev.w = par_rev.rgt - par_rev.lft;
  par_rev.h = par_rev.bot - par_rev.top;
  
  draw_param();
  while (isShowingPar)
  {
    if (Touch_getXY() || first)
    {
      first = false;
      check_presses(0);
      if (is_pressed(par_rand))
      {
        p_random = true;
        p_reversed = false;
        draw_param();
      }
      if (is_pressed(par_rev))
      {
        p_random = false;
        p_reversed = true;
        draw_param();
      }
      if (is_pressed(par_sz) || is_pressed(par_min) || is_pressed(par_max))
      {

      }

    }
  }
}

void l_selectionSort()
{
  uint8_t i, j, min_idx;
  // One by one move boundary of unsorted subarray
  for (i = 0; i < l_size - 1; i++)
  {
    tft.drawRect(l_scale * i, MAX_Y - l_array[i], l_scale, l_array[i], GREEN);
    // Find the minimum element in unsorted array
    min_idx = i;
    for (j = i + 1; j < l_size; j++)
    {
      check_presses(250);
      while (!isRunning)
      {
        check_presses(250);
        if (isShowingPar)
        {
          break;
        }
      }
      if (isShowingPar)
      {
        break;
      }
      if (l_array[j] < l_array[min_idx])
      {
        tft.drawRect(l_scale * j, MAX_Y - l_array[j], l_scale, l_array[j], RED);
        tft.drawRect(l_scale * min_idx, MAX_Y - l_array[min_idx], l_scale, l_array[min_idx], WHITE);
        min_idx = j;
        delay(25);
      }

    }
    if (isShowingPar)
    {
      break;
    }
    // Swap the found minimum element with the first element
    tft.drawRect(l_scale * min_idx, MAX_Y - l_array[i], l_scale, l_array[i], WHITE);
    tft.drawRect(l_scale * i, 0, l_scale, MAX_Y, BLACK);
    uint8_t temp = l_array[min_idx];
    l_array[min_idx] = l_array[i];
    l_array[i] = temp;
    tft.drawRect(l_scale * i, MAX_Y - l_array[i], l_scale, l_array[i], GREEN);
    // delay(25);

  }
}

void check_presses(int CD)
{
  if (pressCD > CD)
  {
    if (Touch_getXY())
    {
      if (is_pressed(homeb))
      {
        pressCD = 0;
        Serial.println("Take me home");
      }
      if (is_pressed(infob))
      {
        pressCD = 0;
        isShowingInfo = !isShowingInfo;
        Serial.println("Send help");
      }
      if (is_pressed(parb))
      {
        pressCD = 0;
        isShowingPar = !isShowingPar;
        Serial.println("Settings");
      }
      if (is_pressed(playb))
      {
        pressCD = 0;
        isRunning = !isRunning;
        if (isRunning) bmpDraw(pauseb, "pause.bmp", MAX_X - 45, MAX_Y - 45);
        else bmpDraw(playb, "play.bmp", MAX_X - 45, MAX_Y - 45);
      }
    }
  }
  pressCD++;
}

void loop()//////////////////////////////////////////////////////////////////////////////////////////
{
  tft.fillRect(0, 0, 320, 240, BLACK);
  draw_ui();
  l_setup();
  l_update();
  l_selectionSort();
  if (isShowingPar)
  {
    par_screen();
  }
  isRunning = false;
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
void bmpDraw(BTN& btn, char *filename, int x, int y) {

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
    progmemPrintln(PSTR("File not found"));
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
  char c;
  while (c = pgm_read_byte(str++)) Serial.print(c);
}

// Same as above, with trailing newline
void progmemPrintln(const char *str) {
  progmemPrint(str);
  Serial.println();
}
