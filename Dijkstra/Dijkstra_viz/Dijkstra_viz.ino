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
} homeb, infob, restb, infoback, inforgt, infolft, infoext;

uint32_t pressCD = 0;
#define sz 10
uint8_t nodes[sz][2];
uint16_t vertices[sz][sz];
uint8_t l_size = sz;
int priority[sz][2];//(1)cost and (2)from where
bool visited[sz];
#undef sz

bool isRunning = false;
bool isShowingPar = false;
bool isShowingInfo = false;

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
  bmpDraw(restb, "rest.bmp", MAX_X - 45, MAX_Y - 45);
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
  random(3);
  tft.setTextSize(1);
  nodes[0][0] = 35;
  nodes[0][1] = 35;
  priority[0][0] = 0;
  tft.fillCircle(nodes[0][0], nodes[0][1], 4, GREEN);
  nodes[1][0] = 200;
  nodes[1][1] = 200;
  priority[1][0] = 32000;
  visited[1] = false;
  tft.fillCircle(nodes[1][0], nodes[1][1], 4, RED);
  for (uint8_t i = 2; i < l_size; i++)
  {
    nodes[i][0] = ((i) % 3) * 80 + random(1, 65);
    nodes[i][1] = (i / 3) * 60 + random(1, 40);
    tft.fillCircle(nodes[i][0], nodes[i][1], 4, WHITE);
    priority[i][0] = 32000;
    visited[i] = false;
    tft.setCursor(nodes[i][0], nodes[i][1] - 15);
    tft.setTextSize(1);
    tft.println(i-1);
  }

  for (uint8_t i = 0; i < l_size; i++)
  {
    for (uint8_t j = i; j < l_size; j++)
    {
      if (random(3) == 1) //33% chance
      {
        vertices[i][j] = sqrt(pow(nodes[i][0] - nodes[j][0], 2) + pow(nodes[i][1] - nodes[j][1], 2));
        vertices[j][i] = vertices[i][j];
      }
      else
      {
        vertices[i][j] = 0;
        vertices[j][i] = 0;
      }
    }
  }
  vertices[0][1] = 0;
  vertices[1][0] = 0;
}

void l_update()
{
  tft.drawRect(0, 0, 240, 240, BLACK);
  for (uint8_t i = 0; i < l_size; i++)
    for (uint8_t j = i; j < l_size; j++)
    {
      if (vertices[i][j] != 0)
        tft.drawLine(nodes[i][0], nodes[i][1], nodes[j][0], nodes[j][1], WHITE);
    }
}
bool found = false;

void clear_path(int from)
{
  tft.drawLine(nodes[from][0], nodes[from][1],  nodes[priority[from][1]][0], nodes[priority[from][1]][1], WHITE);
  tft.fillCircle(nodes[from][0], nodes[from][1], 4, WHITE);
  if(priority[from][1]!=0)
  clear_path+(priority[from][1]);
}

void draw_path(int from)
{
  tft.drawLine(nodes[from][0], nodes[from][1],  nodes[priority[from][1]][0], nodes[priority[from][1]][1], GREEN);
  if (from == 1)tft.fillCircle(nodes[from][0], nodes[from][1], 4, RED);
  else if (priority[from][1] == 0)
  {
    tft.fillCircle(nodes[0][0], nodes[0][1], 4, GREEN);
    tft.fillCircle(nodes[from][0], nodes[from][1], 4, YELLOW);
  }
  else tft.fillCircle(nodes[from][0], nodes[from][1], 4, YELLOW);
  if(priority[from][1]!=0)
  draw_path(priority[from][1]);
}

void l_path(uint8_t from)
{
  visited[from] = true;
  tft.fillCircle(nodes[from][0], nodes[from][1], 4, MAGENTA);
  draw_path(from);
  for (uint8_t i = 0; i < l_size; i++)
  {
    if (vertices[from][i] == 0 || visited[i] )continue;
    tft.fillCircle(nodes[i][0], nodes[i][1], 4, GREEN);
    tft.drawLine(nodes[from][0], nodes[from][1],  nodes[i][0], nodes[i][1], MAGENTA);
    int t = 1000;
    while(t)
    {
      check_presses(0);
      t--;
      if(isShowingInfo || !isRunning)break;
    }
    if(!isRunning)return;
    if (i!= from && priority[i][0] > priority[from][0] + vertices[i][from])
    {-
      priority[i][0] = priority[from][0] + vertices[i][from];
      priority[i][1] = from;
      if (i == 1) {        
        draw_path(1);
        found = true;
        return;
      }
      
    }
    tft.fillCircle(nodes[i][0], nodes[i][1], 4, WHITE);
    tft.drawLine(nodes[from][0], nodes[from][1],  nodes[i][0], nodes[i][1], WHITE);
  }
  clear_path(from);
  int min = 32000;
  uint8_t rec;
  for (uint8_t i = 0; i < l_size; i++)
  {
    if (priority[i][0] <= min && visited[i] == false)
    {
      min = priority[i][0];
      rec = i;
    }
  }
  tft.fillCircle(nodes[from][0], nodes[from][1], 4, WHITE);
  if(min==32000)return;
  l_path(rec);

}

void info_screen()
{
  tft.fillRect(0, 0, 320, 240, BLACK);
  uint8_t scr_index = 1;
  while (isShowingInfo)
  {
    char* scr_str = "dijkx.bmp";
    scr_str[4] = scr_index + '0';
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
      if (is_pressed(restb))
      {
        isRunning = !isRunning;
        if (isRunning) bmpDraw(restb, "rest.bmp", MAX_X - 45, MAX_Y - 45);
        else bmpDraw(restb, "rest.bmp", MAX_X - 45, MAX_Y - 45);
      }
    }
  }
  pressCD++;
}

void loop()////////////////////////////////////LOOP//////////////////////////////////////////////////////
{
  isRunning = true;
  tft.fillRect(0, 0, 320, 240, BLACK);
  draw_ui();
  l_setup();
  l_update();
  l_path(0);
   while (isRunning)
  {
    check_presses(0);
    if(isShowingInfo)break;
  }
  if (isShowingInfo)
  {
    info_screen();
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
