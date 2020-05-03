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
} homeb, infob, pauseb, infoback, inforgt, infolft, infoext;

uint32_t pressCD = 0;

const uint8_t scale = 8;
const uint8_t size = 30;
uint8_t cell[15][15];
uint8_t wall[200][2];
int walli;

bool isRunning = false;
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
  /* isRunning != isRunning;
    if (isRunning) bmpDraw(pauseb, "pause.bmp", MAX_X - 45, MAX_Y - 45);
    else bmpDraw(playb, "play.bmp", MAX_X - 45, MAX_Y - 45);*/


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
  for (uint8_t i = 0; i < size; i++)
    for (uint8_t j = 0; j < size; j++)
    {
      if (j == 0 || i == 0 || j == size || i == size || i % 2 == 0 || j % 2 == 0)
        cell[i][j] = 1;
      else
        cell[i][j] = 102;
      if (j + 1 < size)
        tft.drawLine(scale * i, scale * j, scale * i, scale * (j + 1), RED);
      if (i + 1 < size)
        tft.drawLine(scale * i, scale * j, scale * (i + 1), scale * j, RED);
    }
  cell[1][1] = 0;

  wall[0][0] = 1;
  wall[0][1] = 2;

  wall[1][0] = 2;
  wall[1][1] = 1;

  walli = 2;
}

void add_neighbors(int row, int col)
{
  if (cell[row][col + 1] > 0)
  {
    wall[walli][0] = row;
    wall[walli][1] = col + 1;
    walli++;
  }
  if (cell[row][col - 1] > 0)
  {
    wall[walli][0] = row;
    wall[walli][1] = col - 1;
    walli++;
  }
  if (cell[row + 1][col] > 0)
  {
    wall[walli][0] = row + 1;
    wall[walli][1] = col;
    walli++;
  }
  if (cell[row - 1][col] > 0)
  {
    wall[walli][0] = row - 1;
    wall[walli][1] = col;
    walli++;
  }

}

void remove_wall(int to_remove)
{
  if (wall[to_remove][1] % 2 == 1)
    tft.drawLine(scale * (wall[to_remove][0] - 1), scale * (wall[to_remove][1] - 1), scale * (wall[to_remove][0]), scale * (wall[to_remove][1] - 1), GREEN);
  else
    tft.drawLine(scale * (wall[to_remove][0] - 1), scale * (wall[to_remove][1] - 1), scale * (wall[to_remove][0] - 1), scale * (wall[to_remove][1]), GREEN);
  for (int i = to_remove; i < walli; i++)
  {
    wall[i][0] = wall[i + 1][0];
    wall[i][1] = wall[i + 1][1];
  }
  walli--;
}

void check_neighbors(int check_index)
{
  int c_row = wall[check_index][0];
  int c_col = wall[check_index][1];
  if (cell[c_row][c_col + 1] > 100 && cell[c_row][c_col - 1] == 0)
  {
    cell[c_row][c_col] = 0;
    cell[c_row][c_col + 1] = 0;
    add_neighbors(c_row, c_col + 1);
  }
  else if (cell[c_row][c_col - 1] > 100 && cell[c_row][c_col + 1] == 0)
  {
    cell[c_row][c_col] = 0;
    cell[c_row][c_col - 1] = 0;
    add_neighbors(c_row, c_col - 1);
  }
  else if (cell[c_row + 1][c_col] > 100 && cell[c_row - 1][c_col] == 0)
  {
    cell[c_row][c_col] = 0;
    cell[c_row + 1][c_col] = 0;
    add_neighbors(c_row + 1, c_col);
  }
  else if (cell[c_row - 1][c_col] > 100 && cell[c_row + 1][c_col] == 0)
  {
    cell[c_row][c_col] = 0;
    cell[c_row - 1][c_col] = 0;
    add_neighbors(c_row - 1, c_col);
  }
}

boolean isEdge(int check_index)
{
  int c_row = wall[check_index][0];
  int c_col = wall[check_index][1];
  if (c_col == 0 || c_row == 0 || c_col == size || c_row == size)
    return true;
  else return false;
}

void l_prim()
{
  while (walli > 0)
  {
    int next_visit = random(0, walli);
    if (!isEdge(next_visit))
      check_neighbors(next_visit);
    remove_wall(next_visit);
  }
}

void info_screen()
{
  tft.fillRect(0, 0, 320, 240, BLACK);
  uint8_t scr_index = 1;
  while (isShowingInfo)
  {
    char* scr_str = "selsortx.bmp";
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
      /* if (is_pressed(playb))
        {
         isRunning = !isRunning;
         if (isRunning) bmpDraw(pauseb, "pause.bmp", MAX_X - 45, MAX_Y - 45);
         else bmpDraw(playb, "play.bmp", MAX_X - 45, MAX_Y - 45);
        }*/
    }
  }
  pressCD++;
}

void loop()//////////////////////////////////////////////////////////////////////////////////////////
{
  tft.fillRect(0, 0, 320, 240, BLACK);
  draw_ui();
  l_setup();
  l_prim();
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
