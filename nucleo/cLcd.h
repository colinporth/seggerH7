#pragma once
//{{{  includes
#include "cmsis_os.h"
#include "semphr.h"

#include "../common/utils.h"
#include "../common/cPointRect.h"
#include <map>
#include <vector>

#include "../system/stm32h7xx.h"
#include "heap.h"

#include <ft2build.h>
#include FT_FREETYPE_H
//}}}
//{{{  colour defines
#define COL_BLACK         0x0000
#define COL_GREY          0x7BEF
#define COL_WHITE         0xFFFF

#define COL_BLUE          0x001F
#define COL_GREEN         0x07E0
#define COL_RED           0xF800

#define COL_CYAN          0x07FF
#define COL_MAGENTA       0xF81F
#define COL_YELLOW        0xFFE0
//}}}
//{{{  screen resolution defines
#ifdef NEXXY_SCREEN
  // NEXXY 7 inch
  #define LCD_WIDTH         800
  #define LCD_HEIGHT       1280

  #define BOX_HEIGHT         30
  #define SMALL_FONT_HEIGHT  12
  #define FONT_HEIGHT        26
  #define BIG_FONT_HEIGHT    40

#else
  // ASUS eee 10 inch
  #define LCD_WIDTH        1024  // min 39Mhz typ 45Mhz max 51.42Mhz
  #define LCD_HEIGHT        600

  #define BOX_HEIGHT         20
  #define SMALL_FONT_HEIGHT  10
  #define FONT_HEIGHT        16
  #define BIG_FONT_HEIGHT    32
#endif
//}}}

//{{{
class cTile {
public:
  cTile() {};
  cTile (uint8_t* piccy, uint16_t components, uint16_t pitch,
         uint16_t x, uint16_t y, uint16_t width, uint16_t height)
     :  mPiccy(piccy), mComponents(components), mPitch(pitch), mX(x), mY(y), mWidth(width), mHeight(height) {
   if (components == 2)
     mFormat = DMA2D_INPUT_RGB565;
   else if (components == 3)
     mFormat = DMA2D_INPUT_RGB888;
   else
     mFormat = DMA2D_INPUT_ARGB8888;
    };

  ~cTile () {
    sdRamFree (mPiccy);
    mPiccy = nullptr;
    };

  void* operator new (std::size_t size) { return pvPortMalloc (size); }
  void operator delete (void* ptr) { vPortFree (ptr); }

  uint8_t* mPiccy = nullptr;
  uint16_t mComponents = 0;
  uint16_t mPitch = 0;
  uint16_t mX = 0;
  uint16_t mY = 0;
  uint16_t mWidth = 0;
  uint16_t mHeight = 0;
  uint16_t mFormat = 0;
  };
//}}}
//{{{
class cFontChar {
public:
  void* operator new (std::size_t size) { return pvPortMalloc (size); }
  void operator delete (void* ptr) { vPortFree (ptr); }

  uint8_t* bitmap;
  int16_t left;
  int16_t top;
  int16_t pitch;
  int16_t rows;
  int16_t advance;
  };
//}}}

class cLcd {
public:
  enum eDma2dWait { eWaitNone, eWaitDone, eWaitIrq };
  cLcd();
  ~cLcd();

  void init (const std::string& title);
  static uint16_t getWidth() { return LCD_WIDTH; }
  static uint16_t getHeight() { return LCD_HEIGHT; }
  static cPoint getSize() { return cPoint (getWidth(), getHeight()); }

  static uint16_t getBoxHeight() { return BOX_HEIGHT; }
  static uint16_t getSmallFontHeight() { return SMALL_FONT_HEIGHT; }
  static uint16_t getFontHeight() { return FONT_HEIGHT; }
  static uint16_t getBigFontHeight() { return BIG_FONT_HEIGHT; }

  uint16_t getBrightness() { return mBrightness; }

  void setShowInfo (bool show);
  void setTitle (const std::string& str) { mTitle = str; mChanged = true; }
  void change() { mChanged = true; }
  //{{{
  bool isChanged() {
    bool wasChanged = mChanged;
    mChanged = false;
    return wasChanged;
    }
  //}}}
  //{{{
  void toggle() {
    mShowInfo = !mShowInfo;
    change();
    }
  //}}}

  void info (uint16_t colour, const std::string str);
  void info (const std::string str) { info (COL_WHITE, str); }

  void clear (uint16_t colour);
  void rect (uint16_t colour, const cRect& r);
  void rectClipped (uint16_t colour, cRect r);
  void rectOutline (uint16_t colour, const cRect& r, uint8_t thickness);
  void ellipse (uint16_t colour, cPoint centre, cPoint radius);

  void stamp (uint16_t colour, uint8_t* src, const cRect& r);
  void stampClipped (uint16_t colour, uint8_t* src, cRect r);
  int text (uint16_t colour, uint16_t fontHeight, const std::string& str, cRect r);

  void copy (cTile* srcTile, cPoint p);
  void copy90 (cTile* srcTile, cPoint p);
  void size (cTile* srcTile, const cRect& r);

  inline void pixel (uint16_t colour, cPoint p) { *(mBuffer[mDrawBuffer] + p.y * getWidth() + p.x) = colour; }
  void grad (uint16_t colTL, uint16_t colTR, uint16_t colBL, uint16_t colBR, const cRect& r);
  void line (uint16_t colour, cPoint p1, cPoint p2);
  void ellipseOutline (uint16_t colour, cPoint centre, cPoint radius);

  static void rgb888toRgb565 (uint8_t* src, uint8_t* dst, uint16_t xsize, uint16_t ysize);
  static void yuvMcuToRgb565 (uint8_t* src, uint8_t* dst, uint16_t xsize, uint16_t ysize, uint32_t chromaSampling);
  static void yuvMcuTo565sw (uint8_t* src, uint8_t* dst, uint16_t xsize, uint16_t ysize, 
                             uint32_t BlockIndex, uint32_t DataCount);

  void start();
  void drawInfo();
  void present();

  void display (int brightness);

  static cLcd* mLcd;
  static uint32_t mShowBuffer;

  static eDma2dWait mDma2dWait;
  static SemaphoreHandle_t mDma2dSem;
  static SemaphoreHandle_t mFrameSem;

  void* operator new (std::size_t size) { return pvPortMalloc (size); }
  void operator delete (void* ptr) { vPortFree (ptr); }

private:
  void ltdcInit (uint16_t* frameBufferAddress);
  cFontChar* loadChar (uint16_t fontHeight, char ch);

  static void ready();
  void reset();

  //{{{  vars
  uint32_t rectRegs[5];
  uint32_t stampRegs[15];

  LTDC_HandleTypeDef mLtdcHandle;
  TIM_HandleTypeDef mTimHandle;
  int mBrightness = 50;

  bool mChanged = true;
  bool mDrawBuffer = false;
  uint16_t* mBuffer[2] = {nullptr, nullptr};

  uint32_t mBaseTime = 0;
  uint32_t mStartTime = 0;
  uint32_t mDrawTime = 0;
  uint32_t mWaitTime = 0;
  uint32_t mNumPresents = 0;

  std::map<uint16_t, cFontChar*> mFontCharMap;

  FT_Library FTlibrary;
  FT_Face FTface;
  FT_GlyphSlot FTglyphSlot;

  bool mShowInfo = true;
  std::string mTitle;

  static const int kMaxLines = 40;
  //{{{
  class cLine {
  public:
    cLine() {}
    ~cLine() {}

    //{{{
    void clear() {
      mTime = 0;
      mColour = COL_WHITE;
      mString = "";
      }
    //}}}

    int mTime = 0;
    int mColour = COL_WHITE;
    std::string mString;
    };
  //}}}
  cLine mLines[kMaxLines];
  int mCurLine = 0;
  //}}}
  };
