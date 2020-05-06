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
} homeb, infoback, inforgt, infolft, infob, playb, parb, infoext;

bool isRunning = false;
bool isShowingPar = false;
bool isShowingInfo = false;

uint32_t pressCD = 0;

uint8_t A, B;
uint8_t initial_A = 175, initial_B = 55;

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

void draw_ui()
{

  bmpDraw(homeb, "home.bmp", MAX_X - 45, 5);
  bmpDraw(infob, "info.bmp", MAX_X - 45, 45);
  bmpDraw(parb, "par.bmp", MAX_X - 45, 85);
  isRunning != isRunning;
  if (isRunning) bmpDraw(playb, "pause.bmp", MAX_X - 45, MAX_Y - 45);
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


BTN par_A, par_B, par_rand, par_rev;
uint8_t  p_Aval, p_Bval;
bool p_random = false;
bool p_preset = true;

void draw_param()
{
  tft.fillRect(0, 0, 270, 240, BLACK);
  tft.drawRect(par_B.lft, par_B.top, par_B.w, par_B.h, GREEN);
  tft.drawRect(par_A.lft, par_A.top, par_A.w, par_A.h, GREEN);
  tft.drawRect(par_rand.lft, par_rand.top, par_rand.w, par_rand.h, GREEN);
  tft.drawRect(par_rev.lft, par_rev.top, par_rev.w, par_rev.h, GREEN);

  tft.fillRect(par_B.lft + 1, par_B.top + 1, par_B.rgt - par_B.lft - 2, 8, BLACK);
  tft.fillRect(par_B.lft + 1, par_B.top + 1, p_Bval - par_B.lft - 2, 8, GREEN);

  tft.fillRect(par_A.lft + 1, par_A.top + 1, par_A.rgt - par_A.lft - 2, 8, BLACK);
  tft.fillRect(par_A.lft + 1, par_A.top + 1, p_Aval - par_A.lft - 2, 8, GREEN);

  tft.setTextSize(2);
  tft.setCursor(25, 75);
  tft.println("A value:");
  tft.setCursor(25, 125);
  tft.println("B value:");

  tft.setTextSize(1);
  tft.setCursor(75, 175);
  tft.println("Random:");
  tft.setCursor(150, 175);
  tft.println("Pre-Set:");


  tft.setCursor(15, 100);
  tft.println("0");
  tft.setCursor(225, 100);
  tft.println("240");

  tft.setCursor(15, 150);
  tft.println("0");
  tft.setCursor(225, 150);
  tft.println("240");

  tft.setTextSize(5);

  tft.setCursor(83, 187);
  if (p_random)
    tft.println("X");
  tft.setCursor(162, 187);
  if (p_preset)
    tft.println("X");
}
void par_screen()
{
  tft.fillRect(0, 0, 320, 240, BLACK);
  bmpDraw(parb, "exit.bmp", MAX_X - 45, MAX_Y - 50);
  bool first = true;

  par_B.top = 150;
  par_B.bot = 160;
  par_B.lft = 30;
  par_B.rgt = 220;
  par_B.w = par_B.rgt - par_B.lft;
  par_B.h = par_B.bot - par_B.top;

  par_A.top = 100;
  par_A.bot = 110;
  par_A.lft = 30;
  par_A.rgt = 220;
  par_A.w = par_A.rgt - par_A.lft;
  par_A.h = par_A.bot - par_A.top;

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

  p_Bval = map(initial_B, 0, 240, par_B.lft, par_B.rgt);
  p_Aval = map(initial_A, 0, 240, par_A.lft, par_A.rgt);

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
        p_preset = false;
        draw_param();
      }
      if (is_pressed(par_rev))
      {
        p_random = false;
        p_preset = true;
        draw_param();
      }
      if (is_pressed(par_B))
      {
        p_Bval = pixel_x;
        initial_B = map(pixel_x, par_B.lft, par_B.rgt, 0, 240);
        A=initial_A;
        draw_param();
      }
      if (is_pressed(par_A))
      {
        p_Aval = pixel_x;
        initial_A = map(pixel_x, par_A.lft, par_A.rgt, 0, 240);
        B = initial_B;
        draw_param();
      }
    }
  }
}

void euclid()
{
  tft.fillRect(0, 0, MAX_X - 50, MAX_Y, BLACK);
  uint8_t apos = MAX_Y - A / 2;
  if (apos > 230)apos = 230;
  uint8_t bpos = MAX_Y - B / 2;
  if (bpos > 230)bpos = 230;
  tft.fillRect(50, MAX_Y - A, 20, A, GREEN);
  tft.fillRect(85, MAX_Y - B, 20, B, GREEN);
  tft.setTextSize(1);
  tft.setCursor(25, apos);
  tft.println(A);
  tft.setCursor(110, bpos);
  tft.println(B);
  if (A == 0)
    return;
  else
  {
    uint8_t temp = B % A;
    B = A;
    A = temp;
    uint64_t time2 = millis() + 1000;
    while (millis() < time2) //delay
    {
      check_presses(500);
      while (!isRunning) {
        check_presses(500);
        if (isShowingPar || isShowingInfo)
        return;
      }
      if (isShowingPar || isShowingInfo)
        return;
    }
    euclid();
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
        load_home();
      }
      if (is_pressed(infob))
      {
        isShowingInfo = !isShowingInfo;
        Serial.println("Send help");
      }
      if (is_pressed(parb))
      {
        isShowingPar = !isShowingPar;
        Serial.println("Settings");
      }
      if (is_pressed(playb))
      {
        isRunning = !isRunning;
        if (isRunning) bmpDraw(playb, "pause.bmp", MAX_X - 45, MAX_Y - 45);
        else bmpDraw(playb, "play.bmp", MAX_X - 45, MAX_Y - 45);
      }
    }
  }
  pressCD++;
}

void info_screen()
{
  tft.fillRect(0, 0, 320, 240, BLACK);
  uint8_t scr_index = 1;
  while (isShowingInfo)
  {
    char* scr_str = "euclidx.bmp";
    scr_str[6] = scr_index + '0';
    bmpDraw(infoback, scr_str, 0, 0);
    bmpDraw(infoext, "exit.bmp", MAX_X - 46, 13);
    if (scr_index != max_page)
      bmpDraw(inforgt, "right.bmp", MAX_X - 37 - 15, MAX_Y - 45);
    if (scr_index != 1)
      bmpDraw(infolft, "left.bmp", 15, MAX_Y - 45);
    while (1)
    {
      if (Touch_getXY())
      {
        {
          if (is_pressed(infoext))
          {
            isShowingInfo = false;
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
  tft.fillRect(0, 0, 320, 240, BLACK);
  draw_ui();
  A = initial_A;
  B = initial_B;
  if (p_random)
  {
    A = random(240);
    B = random(240);
  }
  euclid();
  tft.setTextSize(2);
  tft.setCursor(MAX_X / 2, MAX_Y / 2);
  tft.println("GCD:");
  tft.setCursor(MAX_X / 2 + 15, MAX_Y / 2 + 20);
  tft.println(B);
  isRunning = false;
  bmpDraw(playb, "play.bmp", MAX_X - 45, MAX_Y - 45);
  while (!isRunning) {
    check_presses(500);
    if (isShowingPar || isShowingInfo)
      break;
  }
  if (isShowingInfo)
    info_screen();
  if (isShowingPar)
    par_screen();
  isRunning = false;
  isShowingInfo = false;
  isShowingPar = false;
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
