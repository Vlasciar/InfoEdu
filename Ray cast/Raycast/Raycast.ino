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
} homeb, infob, restb, infoback, inforgt, infolft, infoext , parb, par_sz;

uint32_t pressCD = 0;

uint8_t l_array[25][4];//X1 Y1     X2 Y2
uint8_t l_size = 10;

bool isShowingInfo = false;
bool isShowingPar = false;

int16_t posX, posY;

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
  posX = MAX_X / 2;
  posY = MAX_Y / 2;
}


void draw_ui()
{
  bmpDraw(homeb, "home.bmp", MAX_X - 45, 5);
  bmpDraw(infob, "info.bmp", MAX_X - 45, 45);
  bmpDraw(parb, "par.bmp", MAX_X - 45, 85);
  bmpDraw(restb, "play.bmp", MAX_X - 45, MAX_Y - 45);
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
void l_update()
{
  tft.fillRect(0, 0, MAX_X - 50 + 1, MAX_Y, BLACK);
  for (uint8_t i = 0; i < l_size; i++)
  {
    tft.drawLine(l_array[i][0], l_array[i][1], l_array[i][2], l_array[i][3], RED);
  }
}
void l_setup()
{
  tft.fillRect(0, 0, MAX_X - 50 + 1, MAX_Y, BLACK);
  for (uint8_t i = 0; i < l_size; i++)
  {
    l_array[i][0] = random(0, MAX_X - 50);
    l_array[i][1] = random(0, MAX_Y);
    l_array[i][2] = random(0, MAX_X - 50);
    l_array[i][3] = random(0, MAX_Y);
    tft.drawLine(l_array[i][0], l_array[i][1], l_array[i][2], l_array[i][3], RED);
  }
}

uint8_t p_sz;
void draw_param()
{
  tft.fillRect(0, 0, 270, 240, BLACK);
  tft.drawRect(par_sz.lft, par_sz.top, par_sz.w, par_sz.h, GREEN);

  tft.fillRect(par_sz.lft + 1, par_sz.top + 1, par_sz.rgt - par_sz.lft - 2, 8, BLACK);
  tft.fillRect(par_sz.lft + 1, par_sz.top + 1, p_sz - par_sz.lft - 2, 8, GREEN);


  tft.setTextSize(2);
  tft.setCursor(25, 25);
  tft.println("Number of lines:");

  tft.setCursor(5, 50);
  tft.println("3");
  tft.setCursor(225, 50);
  tft.println("25");

}

void par_screen()
{
  tft.fillRect(0, 0, 320, 240, BLACK);
  bmpDraw(parb, "exit.bmp", MAX_X - 45, MAX_Y - 50);
  bool first = true;
  par_sz.top = 50;
  par_sz.bot = 60;
  par_sz.lft = 30;
  par_sz.rgt = 220;
  par_sz.w = par_sz.rgt - par_sz.lft;
  par_sz.h = par_sz.bot - par_sz.top;

  p_sz = map(l_size, 3, 25, par_sz.lft, par_sz.rgt);

  draw_param();
  while (isShowingPar)
  {
    if (Touch_getXY() || first)
    {
      first = false;
      check_presses(0);
      if (is_pressed(par_sz))
      {
        p_sz = pixel_x;
        l_size = map(pixel_x, par_sz.lft, par_sz.rgt, 3, 25);
        draw_param();
      }
    }
  }
}

void drawline(float x4, float y4)
{
  float x1, x2, x3, y1, y2, y3;
  x3 = posX;
  y3 = posY;
  int16_t recX = x4;//punc de intersectie cele mai apropiate
  int16_t recY = y4;
  int16_t reclen = abs(posX - recX) + abs( posY - recY);
  for (uint8_t i = 0; i < l_size; i++)
  {
    x1 = l_array[i][0];
    y1 = l_array[i][1];
    x2 = l_array[i][2];
    y2 = l_array[i][3];
    float den = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    float t = ((float)((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4))) / den;
    float u = -((float)((x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3))) / den;
    if (t > 0 && t < 1 && u > 0)
    {
      int16_t tX = x1 + t * (float(x2 - x1)); //temporary x, y and len
      int16_t tY = y1 + t * (float(y2 - y1));
      int16_t tlen = abs(posX - tX) + abs(posY - tY);
      if (tlen < reclen)
      {
        recX = tX;
        recY = tY;
        reclen = tlen;
      }
    }
  }
  tft.drawLine(posX, posY, recX, recY, WHITE);
}

void l_raycast()
{
  posX = pixel_x;
  posY = pixel_y;
  uint16_t targX, targY;
  targY = 0;
  for (int16_t targX = 0; targX <= MAX_X - 50; targX += 10)
  {
    drawline(targX, targY);
  }
  targX = MAX_X - 50;
  for (int16_t targY = 0; targY <= MAX_Y; targY += 10)
  {
    drawline(targX, targY);
  }
  targY = MAX_Y;
  for (int16_t targX = MAX_X - 50; targX >= 0 && targX <= MAX_X - 50 + 1; targX -= 10)
  {
    drawline(targX, targY);
  }
  targX = 0;
  for (int16_t targY = MAX_Y; targY >= 0 && targY <= MAX_Y + 1; targY -= 10)
  {
    drawline(targX, targY);
  }
  targY = 0;
}

void info_screen()
{
  tft.fillRect(0, 0, MAX_X, MAX_Y, BLACK);
  uint8_t scr_index = 1;
  while (isShowingInfo)
  {
    char* scr_str = "raycastx.bmp";
    scr_str[7] = scr_index + '0';
    bmpDraw(infoback, scr_str, 0, 0);
    bmpDraw(infoext, "exit.bmp", MAX_X - 52, 10);
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

void check_presses(int CD)
{
  if (pressCD > CD)
  {
    if (is_pressed(homeb))
    {
      load_home();
    }
    if (is_pressed(infob))
    {
      isShowingInfo = !isShowingInfo;
    }
    if (is_pressed(restb))
    {
      l_setup();
    }
    if (is_pressed(parb))
    {
      isShowingPar = !isShowingPar;
    }
  }
  pressCD++;
}

void loop()//////////////////////////////////////////////////////////////////////////////////////////
{
  tft.fillRect(0, 0, MAX_X, MAX_Y, BLACK);
  draw_ui();
  l_setup();
  pixel_x = posX;
  pixel_y = posY;
  l_raycast();
  while (1)
  {
    if (Touch_getXY())
    {
      if (pixel_x < MAX_X - 50)
      {
        l_update();
        l_raycast();
      }
      else check_presses(0);
    }
    if (isShowingInfo)
    {
      info_screen();
      tft.fillRect(0, 0, MAX_X, MAX_Y, BLACK);
      draw_ui();
      l_update();
    }
    if (isShowingPar)
    {
      par_screen();
      draw_ui();
    }
  }
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
