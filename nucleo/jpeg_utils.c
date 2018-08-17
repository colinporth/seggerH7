/**
  *      - YCbCr 4:2:0 : Each MCU is composed of 4 Y 8x8 blocks + 1 Cb 8x8 block + Cr 8x8 block
  *      - YCbCr 4:2:2 : Each MCU is composed of 2 Y 8x8 blocks + 1 Cb 8x8 block + Cr 8x8 block
  *      - YCbCr 4:4:4 : Each MCU is composed of 1 Y 8x8 block + 1 Cb 8x8 block + Cr 8x8 block
  */
//{{{
typedef struct __JPEG_MCU_RGB_ConvertorTypeDef
{
  uint32_t ColorSpace;
  uint32_t ChromaSubsampling;

  uint32_t ImageWidth;
  uint32_t ImageHeight;
  uint32_t ImageSize_Bytes;

  uint32_t LineOffset;
  uint32_t BlockSize;

  uint32_t H_factor;
  uint32_t V_factor;

  uint32_t WidthExtend;
  uint32_t ScaledWidth;

  uint32_t MCU_Total_Nb;

  uint16_t *Y_MCU_LUT;
  uint16_t *Cb_MCU_LUT;
  uint16_t *Cr_MCU_LUT;
  uint16_t *K_MCU_LUT;
  } JPEG_MCU_RGB_ConvertorTypeDef;
//}}}
//{{{  defines
#define YCBCR_420_BLOCK_SIZE       384     /* YCbCr 4:2:0 MCU : 4 8x8 blocks of Y + 1 8x8 block of Cb + 1 8x8 block of Cr   */
#define YCBCR_422_BLOCK_SIZE       256     /* YCbCr 4:2:2 MCU : 2 8x8 blocks of Y + 1 8x8 block of Cb + 1 8x8 block of Cr   */
#define YCBCR_444_BLOCK_SIZE       192     /* YCbCr 4:4:4 MCU : 1 8x8 block of Y + 1 8x8 block of Cb + 1 8x8 block of Cr   */

#if (JPEG_RGB_FORMAT == JPEG_ARGB8888)
  #define JPEG_GREEN_OFFSET        8       /* Offset of the GREEN color in a pixel         */
  #define JPEG_ALPHA_OFFSET        24      /* Offset of the Transparency Alpha in a pixel  */
  #define JPEG_BYTES_PER_PIXEL     4       /* Number of bytes in a pixel                   */
  #if (JPEG_SWAP_RB == 0)
    #define JPEG_RED_OFFSET        16      /* Offset of the RED color in a pixel           */
    #define JPEG_BLUE_OFFSET       0       /* Offset of the BLUE color in a pixel          */
  #else
    #define JPEG_RED_OFFSET        0       /* Offset of the RED color in a pixel           */
    #define JPEG_BLUE_OFFSET       16      /* Offset of the BLUE color in a pixel          */
  #endif /* JPEG_SWAP_RB */

#elif(JPEG_RGB_FORMAT == JPEG_RGB888)
  #define JPEG_GREEN_OFFSET        8       /* Offset of the GREEN color in a pixel         */
  #define JPEG_BYTES_PER_PIXEL     3       /* Number of bytes in a pixel                   */

  #if (JPEG_SWAP_RB == 0)
    #define JPEG_RED_OFFSET        16      /* Offset of the RED color in a pixel           */
    #define JPEG_BLUE_OFFSET       0       /* Offset of the BLUE color in a pixel          */
  #else
   #define JPEG_RED_OFFSET         0       /* Offset of the RED color in a pixel           */
   #define JPEG_BLUE_OFFSET        16      /* Offset of the BLUE color in a pixel          */
  #endif  /* JPEG_SWAP_RB */

#elif (JPEG_RGB_FORMAT == JPEG_RGB565)
  #define JPEG_GREEN_OFFSET        5       /* Offset of the GREEN color in a pixel         */
  #define JPEG_BYTES_PER_PIXEL     2       /* Number of bytes in a pixel                   */
  #define JPEG_RGB565_GREEN_MASK   0x07E0  /* Mask of Green component in RGB565 Format     */

  #if (JPEG_SWAP_RB == 0)
    #define JPEG_RED_OFFSET        11      /* Offset of the RED color in a pixel           */
    #define JPEG_BLUE_OFFSET       0       /* Offset of the BLUE color in a pixel          */
    #define JPEG_RGB565_RED_MASK   0xF800  /* Mask of Red component in RGB565 Format       */
    #define JPEG_RGB565_BLUE_MASK  0x001F  /* Mask of Blue component in RGB565 Format      */
  #else
    #define JPEG_RED_OFFSET        0       /* Offset of the RED color in a pixel           */
    #define JPEG_BLUE_OFFSET       11      /* Offset of the BLUE color in a pixel          */
    #define JPEG_RGB565_RED_MASK   0x001F  /* Mask of Red component in RGB565 Format       */
    #define JPEG_RGB565_BLUE_MASK  0xF800  /* Mask of Blue component in RGB565 Format      */
  #endif /* JPEG_SWAP_RB */

#else
  #error "unknown JPEG_RGB_FORMAT "
#endif /* JPEG_RGB_FORMAT */

#define CLAMP(value) CLAMP_LUT[(value) + 0x100] /* Range limitting macro */
//}}}

static JPEG_MCU_RGB_ConvertorTypeDef JPEG_ConvertorParams;

static int32_t CR_RED_LUT[256];           /* Cr to Red color conversion Look Up Table  */
static int32_t CB_BLUE_LUT[256];          /* Cb to Blue color conversion Look Up Table */
static int32_t CR_GREEN_LUT[256];         /* Cr to Green color conversion Look Up Table*/
static int32_t CB_GREEN_LUT[256];         /* Cb to Green color conversion Look Up Table*/

//{{{
static const uint8_t CLAMP_LUT[] = {
/* clamp range 0xffffffff to 0xffffff00 */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* clamp range 0x00 to 0xff */
0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
/* clamp range 0x100 to 0x1ff */
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
};
//}}}

//{{{
static uint32_t JPEG_MCU_YCbCr444_ARGB_ConvertBlocks(uint8_t *pInBuffer,
                                      uint8_t *pOutBuffer,
                                      uint32_t BlockIndex,
                                      uint32_t DataCount,
                                      uint32_t *ConvertedDataCount)
{
  uint32_t numberMCU;
  uint32_t i,j, currentMCU, xRef,yRef;

  uint32_t refline;
  int32_t ycomp, crcomp, cbcomp;

  int32_t c_red, c_blue, c_green;

  uint8_t *pOutAddr;
  uint8_t *pChrom, *pLum;

  numberMCU = DataCount / YCBCR_444_BLOCK_SIZE;
  currentMCU = BlockIndex;


  while(currentMCU < (numberMCU + BlockIndex))
  {

    xRef = ((currentMCU *8) / JPEG_ConvertorParams.WidthExtend)*8;

    yRef = ((currentMCU *8) % JPEG_ConvertorParams.WidthExtend);

    refline = JPEG_ConvertorParams.ScaledWidth * xRef + (JPEG_BYTES_PER_PIXEL*yRef);

    currentMCU++;

    pChrom = pInBuffer + 64; /* pChroma = pInBuffer + 4*64 */

    pLum = pInBuffer;

    for(i= 0; i <  8; i++)
    {
      if(refline < JPEG_ConvertorParams.ImageSize_Bytes)
      {
        pOutAddr = pOutBuffer+ refline;


          for(j=0; j < 8; j++)
          {
            cbcomp = (int32_t)(*pChrom);
            c_blue = (int32_t)(*(CB_BLUE_LUT + cbcomp));

            crcomp = (int32_t)(*(pChrom + 64));
            c_red = (int32_t)(*(CR_RED_LUT + crcomp));

            c_green = ((int32_t)(*(CR_GREEN_LUT + crcomp)) + (int32_t)(*(CB_GREEN_LUT + cbcomp))) >> 16;

#if (JPEG_RGB_FORMAT == JPEG_ARGB8888)

            ycomp = (int32_t)(*(pLum +j));

            *(__IO uint32_t *)pOutAddr =
              (CLAMP(ycomp + c_red) << JPEG_RED_OFFSET)     | \
              (CLAMP( ycomp + c_green) << JPEG_GREEN_OFFSET) | \
              (CLAMP(ycomp + c_blue) << JPEG_BLUE_OFFSET);

#elif (JPEG_RGB_FORMAT == JPEG_RGB888)

            ycomp = (int32_t)(*(pLum +j));

            pOutAddr[JPEG_RED_OFFSET/8] = CLAMP(ycomp + c_red);
            pOutAddr[JPEG_GREEN_OFFSET/8] = CLAMP(ycomp + c_green);
            pOutAddr[JPEG_BLUE_OFFSET/8] = CLAMP(ycomp + c_blue);

#elif (JPEG_RGB_FORMAT == JPEG_RGB565)

            ycomp = (int32_t)(*(pLum +j));

            *(__IO uint16_t *)pOutAddr =
              ((CLAMP(ycomp + c_red) >> 3) << JPEG_RED_OFFSET)     | \
              ((CLAMP( ycomp + c_green) >> 2) << JPEG_GREEN_OFFSET) | \
              ((CLAMP(ycomp + c_blue) >> 3) << JPEG_BLUE_OFFSET);

#endif /* JPEG_RGB_FORMAT */

            pOutAddr += JPEG_BYTES_PER_PIXEL;

            pChrom++;
          }
          pLum += 8;

        refline += JPEG_ConvertorParams.ScaledWidth;
      }
    }

    pInBuffer +=  YCBCR_444_BLOCK_SIZE;
  }
  return numberMCU;
}
//}}}
//{{{
static uint32_t JPEG_MCU_YCbCr422_ARGB_ConvertBlocks(uint8_t *pInBuffer,
                                      uint8_t *pOutBuffer,
                                      uint32_t BlockIndex,
                                      uint32_t DataCount,
                                      uint32_t *ConvertedDataCount)
{
  uint32_t numberMCU;
  uint32_t i,j,k, currentMCU, xRef,yRef;

  uint32_t refline;
  int32_t ycomp, crcomp, cbcomp;

  int32_t c_red, c_blue, c_green;

  uint8_t *pOutAddr;
  uint8_t *pChrom, *pLum;

  numberMCU = DataCount / YCBCR_422_BLOCK_SIZE;
  currentMCU = BlockIndex;


  while(currentMCU < (numberMCU + BlockIndex))
  {
    xRef = ((currentMCU *16) / JPEG_ConvertorParams.WidthExtend)*8;

    yRef = ((currentMCU *16) % JPEG_ConvertorParams.WidthExtend);

    refline = JPEG_ConvertorParams.ScaledWidth * xRef + (JPEG_BYTES_PER_PIXEL*yRef);

    currentMCU++;

    pChrom = pInBuffer + 128; /* pChroma = pInBuffer + 2*64 */

    pLum = pInBuffer;

    for(i= 0; i <  8; i++)
    {
       if(refline < JPEG_ConvertorParams.ImageSize_Bytes)
      {
        pOutAddr = pOutBuffer + refline;

        for(k= 0; k<2; k++)
        {
          for(j=0; j < 8; j+=2)
          {
            cbcomp = (int32_t)(*(pChrom));
            c_blue = (int32_t)(*(CB_BLUE_LUT + cbcomp));

            crcomp = (int32_t)(*(pChrom + 64));
            c_red = (int32_t)(*(CR_RED_LUT + crcomp));

            c_green = ((int32_t)(*(CR_GREEN_LUT + crcomp)) + (int32_t)(*(CB_GREEN_LUT + cbcomp))) >> 16;



#if (JPEG_RGB_FORMAT == JPEG_ARGB8888)

            ycomp = (int32_t)(*(pLum +j));

            *(__IO uint32_t *)pOutAddr =
              (CLAMP(ycomp + c_red) << JPEG_RED_OFFSET)     | \
              (CLAMP( ycomp + c_green) << JPEG_GREEN_OFFSET) | \
              (CLAMP(ycomp + c_blue) << JPEG_BLUE_OFFSET);
            /**********/
            ycomp = (int32_t)(*(pLum +j +1));

            *((__IO uint32_t *)(pOutAddr + 4)) =
              (CLAMP(ycomp + c_red) << JPEG_RED_OFFSET)     | \
              (CLAMP( ycomp + c_green) << JPEG_GREEN_OFFSET) | \
              (CLAMP(ycomp + c_blue) << JPEG_BLUE_OFFSET);

#elif (JPEG_RGB_FORMAT == JPEG_RGB888)

           ycomp = (int32_t)(*(pLum +j));

            pOutAddr[JPEG_RED_OFFSET/8] = CLAMP(ycomp + c_red);
            pOutAddr[JPEG_GREEN_OFFSET/8] = CLAMP(ycomp + c_green);
            pOutAddr[JPEG_BLUE_OFFSET/8] = CLAMP(ycomp + c_blue);

            /**********/
            ycomp = (int32_t)(*(pLum +j +1));

            pOutAddr[3 + JPEG_RED_OFFSET/8] = CLAMP(ycomp + c_red);
            pOutAddr[3 + JPEG_GREEN_OFFSET/8] = CLAMP(ycomp + c_green);
            pOutAddr[3 + JPEG_BLUE_OFFSET/8] = CLAMP(ycomp + c_blue);

#elif (JPEG_RGB_FORMAT == JPEG_RGB565)

            ycomp = (int32_t)(*(pLum +j));

            *(__IO uint16_t *)pOutAddr =
              ((CLAMP(ycomp + c_red) >> 3) << JPEG_RED_OFFSET)     | \
              ((CLAMP( ycomp + c_green) >> 2) << JPEG_GREEN_OFFSET) | \
              ((CLAMP(ycomp + c_blue) >> 3) << JPEG_BLUE_OFFSET);
            /**********/
            ycomp = (int32_t)(*(pLum +j +1));

            *((__IO uint16_t *)(pOutAddr + 2)) =
              ((CLAMP(ycomp + c_red) >> 3) << JPEG_RED_OFFSET)     | \
              ((CLAMP( ycomp + c_green) >> 2) << JPEG_GREEN_OFFSET) | \
              ((CLAMP(ycomp + c_blue) >> 3) << JPEG_BLUE_OFFSET);

#endif /* JPEG_RGB_FORMAT*/

            pOutAddr += JPEG_BYTES_PER_PIXEL * 2;

            pChrom++;
          }
          pLum += 64;
        }

        pLum = pLum - 128 + 8;

        refline += JPEG_ConvertorParams.ScaledWidth;
      }
    }

    pInBuffer +=  YCBCR_422_BLOCK_SIZE;
  }
  return numberMCU;
}
//}}}
//{{{
static uint32_t JPEG_MCU_YCbCr420_ARGB_ConvertBlocks(uint8_t *pInBuffer,
                                      uint8_t *pOutBuffer,
                                      uint32_t BlockIndex,
                                      uint32_t DataCount,
                                      uint32_t *ConvertedDataCount)
{
  uint32_t numberMCU;
  uint32_t i,j,k, currentMCU, xRef,yRef;

  uint32_t refline;
  int32_t ycomp, crcomp, cbcomp;

  int32_t c_red, c_blue, c_green;

  uint8_t *pOutAddr, *pOutAddr2;
  uint8_t *pChrom, *pLum;

  numberMCU = DataCount / YCBCR_420_BLOCK_SIZE;
  currentMCU = BlockIndex;


  while(currentMCU < (numberMCU + BlockIndex))
  {
    xRef = ((currentMCU *16) / JPEG_ConvertorParams.WidthExtend)*16;

    yRef = ((currentMCU *16) % JPEG_ConvertorParams.WidthExtend);

    refline = JPEG_ConvertorParams.ScaledWidth * xRef + (JPEG_BYTES_PER_PIXEL*yRef);

    currentMCU++;

    pChrom = pInBuffer + 256; /* pChroma = pInBuffer + 4*64 */

    pLum = pInBuffer;

    for(i= 0; i <  16; i+=2)
    {
      if(i == 8)
      {
        pLum = pInBuffer + 128;
      }

      if(refline < JPEG_ConvertorParams.ImageSize_Bytes)
      {
        pOutAddr = pOutBuffer + refline;
        pOutAddr2 = pOutAddr + JPEG_ConvertorParams.ScaledWidth;

        for(k= 0; k<2; k++)
        {
          for(j=0; j < 8; j+=2)
          {
            cbcomp = (int32_t)(*(pChrom));
            c_blue = (int32_t)(*(CB_BLUE_LUT + cbcomp));

            crcomp = (int32_t)(*(pChrom + 64));
            c_red = (int32_t)(*(CR_RED_LUT + crcomp));

            c_green = ((int32_t)(*(CR_GREEN_LUT + crcomp)) + (int32_t)(*(CB_GREEN_LUT + cbcomp))) >> 16;



#if (JPEG_RGB_FORMAT == JPEG_ARGB8888)

            ycomp = (int32_t)(*(pLum +j));

            *(__IO uint32_t *)pOutAddr =
              (CLAMP(ycomp + c_red) << JPEG_RED_OFFSET)     | \
              (CLAMP( ycomp + c_green) << JPEG_GREEN_OFFSET) | \
              (CLAMP(ycomp + c_blue) << JPEG_BLUE_OFFSET);
            /**********/
            ycomp = (int32_t)(*(pLum +j +1));

            *((__IO uint32_t *)(pOutAddr + 4)) =
              (CLAMP(ycomp + c_red) << JPEG_RED_OFFSET)     | \
              (CLAMP( ycomp + c_green) << JPEG_GREEN_OFFSET) | \
              (CLAMP(ycomp + c_blue) << JPEG_BLUE_OFFSET);

            /**********/
            ycomp = (int32_t)(*(pLum +j +8));

            *(__IO uint32_t *)pOutAddr2 =
              (CLAMP(ycomp + c_red) << JPEG_RED_OFFSET)     | \
              (CLAMP( ycomp + c_green) << JPEG_GREEN_OFFSET) | \
              (CLAMP(ycomp + c_blue) << JPEG_BLUE_OFFSET);

            /**********/
            ycomp = (int32_t)(*(pLum +j +8 +1));

            *((__IO uint32_t *)(pOutAddr2 +4)) =
              (CLAMP(ycomp + c_red) << JPEG_RED_OFFSET)     | \
              (CLAMP( ycomp + c_green) << JPEG_GREEN_OFFSET) | \
              (CLAMP(ycomp + c_blue) << JPEG_BLUE_OFFSET);


#elif (JPEG_RGB_FORMAT == JPEG_RGB888)

           ycomp = (int32_t)(*(pLum +j));

            pOutAddr[JPEG_RED_OFFSET/8] = CLAMP(ycomp + c_red);
            pOutAddr[JPEG_GREEN_OFFSET/8] = CLAMP(ycomp + c_green);
            pOutAddr[JPEG_BLUE_OFFSET/8] = CLAMP(ycomp + c_blue);

            /**********/
            ycomp = (int32_t)(*(pLum +j +1));

            pOutAddr[3 + JPEG_RED_OFFSET/8] = CLAMP(ycomp + c_red);
            pOutAddr[3 + JPEG_GREEN_OFFSET/8] = CLAMP(ycomp + c_green);
            pOutAddr[3 + JPEG_BLUE_OFFSET/8] = CLAMP(ycomp + c_blue);

            /**********/
            ycomp = (int32_t)(*(pLum +j +8));

            pOutAddr2[JPEG_RED_OFFSET/8] = CLAMP(ycomp + c_red);
            pOutAddr2[JPEG_GREEN_OFFSET/8] = CLAMP(ycomp + c_green);
            pOutAddr2[JPEG_BLUE_OFFSET/8] = CLAMP(ycomp + c_blue);

            /**********/
            ycomp = (int32_t)(*(pLum +j +8 +1));

            pOutAddr2[3+ JPEG_RED_OFFSET/8] = CLAMP(ycomp + c_red);
            pOutAddr2[3 + JPEG_GREEN_OFFSET/8] = CLAMP(ycomp + c_green);
            pOutAddr2[3 + JPEG_BLUE_OFFSET/8] = CLAMP(ycomp + c_blue);

#elif (JPEG_RGB_FORMAT == JPEG_RGB565)

            ycomp = (int32_t)(*(pLum +j));

            *(__IO uint16_t *)pOutAddr =
              ((CLAMP(ycomp + c_red) >> 3) << JPEG_RED_OFFSET)     | \
              ((CLAMP( ycomp + c_green) >> 2) << JPEG_GREEN_OFFSET) | \
              ((CLAMP(ycomp + c_blue) >> 3) << JPEG_BLUE_OFFSET);
            /**********/
            ycomp = (int32_t)(*(pLum +j +1));

            *((__IO uint16_t *)(pOutAddr + 2)) =
              ((CLAMP(ycomp + c_red) >> 3) << JPEG_RED_OFFSET)     | \
              ((CLAMP( ycomp + c_green) >> 2) << JPEG_GREEN_OFFSET) | \
              ((CLAMP(ycomp + c_blue) >> 3) << JPEG_BLUE_OFFSET);

            /**********/
            ycomp = (int32_t)(*(pLum +j +8));

            *(__IO uint16_t *)pOutAddr2 =
              ((CLAMP(ycomp + c_red) >> 3) << JPEG_RED_OFFSET)     | \
              ((CLAMP( ycomp + c_green) >> 2) << JPEG_GREEN_OFFSET) | \
              ((CLAMP(ycomp + c_blue) >> 3) << JPEG_BLUE_OFFSET);

            /**********/
            ycomp = (int32_t)(*(pLum +j +8 +1));

            *((__IO uint16_t *)(pOutAddr2 +2)) =
              ((CLAMP(ycomp + c_red) >> 3) << JPEG_RED_OFFSET)     | \
              ((CLAMP( ycomp + c_green) >> 2) << JPEG_GREEN_OFFSET) | \
              ((CLAMP(ycomp + c_blue) >> 3) << JPEG_BLUE_OFFSET);
#endif /* JPEG_RGB_FORMAT */

            pOutAddr += JPEG_BYTES_PER_PIXEL * 2;
            pOutAddr2 += JPEG_BYTES_PER_PIXEL * 2;

            pChrom++;
          }
          pLum += 64;
        }

        pLum = pLum - 128 + 16;

        refline += 2*JPEG_ConvertorParams.ScaledWidth;
      }
    }

    pInBuffer +=  YCBCR_420_BLOCK_SIZE;
  }
  return numberMCU;
}
//}}}
//{{{
HAL_StatusTypeDef JPEG_GetDecodeColorConvertFunc(JPEG_ConfTypeDef *pJpegInfo, JPEG_YCbCrToRGB_Convert_Function *pFunction, uint32_t *ImageNbMCUs)
{
  uint32_t hMCU, vMCU;
  JPEG_ConvertorParams.ColorSpace = pJpegInfo->ColorSpace;
  JPEG_ConvertorParams.ImageWidth = pJpegInfo->ImageWidth;
  JPEG_ConvertorParams.ImageHeight = pJpegInfo->ImageHeight;
  JPEG_ConvertorParams.ImageSize_Bytes = pJpegInfo->ImageWidth * pJpegInfo->ImageHeight * JPEG_BYTES_PER_PIXEL;

  JPEG_ConvertorParams.ChromaSubsampling = pJpegInfo->ChromaSubsampling;
  if(JPEG_ConvertorParams.ColorSpace == JPEG_YCBCR_COLORSPACE) {
    if(JPEG_ConvertorParams.ChromaSubsampling == JPEG_420_SUBSAMPLING) {
      *pFunction = JPEG_MCU_YCbCr420_ARGB_ConvertBlocks;
      JPEG_ConvertorParams.LineOffset = JPEG_ConvertorParams.ImageWidth % 16;
      if(JPEG_ConvertorParams.LineOffset != 0)
        JPEG_ConvertorParams.LineOffset = 16 - JPEG_ConvertorParams.LineOffset;
      JPEG_ConvertorParams.H_factor = 16;
      JPEG_ConvertorParams.V_factor  = 16;
    }
    else if(JPEG_ConvertorParams.ChromaSubsampling == JPEG_422_SUBSAMPLING) {
      *pFunction =  JPEG_MCU_YCbCr422_ARGB_ConvertBlocks;
      JPEG_ConvertorParams.LineOffset = JPEG_ConvertorParams.ImageWidth % 16;
      if(JPEG_ConvertorParams.LineOffset != 0)
        JPEG_ConvertorParams.LineOffset = 16 - JPEG_ConvertorParams.LineOffset;
      JPEG_ConvertorParams.H_factor = 16;
      JPEG_ConvertorParams.V_factor  = 8;
    }
    else /*4:4:4*/ {
      *pFunction =  JPEG_MCU_YCbCr444_ARGB_ConvertBlocks;
      JPEG_ConvertorParams.LineOffset = JPEG_ConvertorParams.ImageWidth % 8;
      if(JPEG_ConvertorParams.LineOffset != 0)
        JPEG_ConvertorParams.LineOffset = 8 - JPEG_ConvertorParams.LineOffset;
      JPEG_ConvertorParams.H_factor = 8;
      JPEG_ConvertorParams.V_factor  = 8;
    }
  }
  else if(JPEG_ConvertorParams.ColorSpace == JPEG_GRAYSCALE_COLORSPACE) {
    *pFunction =  JPEG_MCU_Gray_ARGB_ConvertBlocks;
    JPEG_ConvertorParams.LineOffset = JPEG_ConvertorParams.ImageWidth % 8;
    if(JPEG_ConvertorParams.LineOffset != 0)
      JPEG_ConvertorParams.LineOffset = 8 - JPEG_ConvertorParams.LineOffset;
    JPEG_ConvertorParams.H_factor = 8;
    JPEG_ConvertorParams.V_factor  = 8;
  }
  else if(JPEG_ConvertorParams.ColorSpace == JPEG_CMYK_COLORSPACE) {
    *pFunction =  JPEG_MCU_YCCK_ARGB_ConvertBlocks;
    JPEG_ConvertorParams.LineOffset = JPEG_ConvertorParams.ImageWidth % 8;
    if(JPEG_ConvertorParams.LineOffset != 0)
      JPEG_ConvertorParams.LineOffset = 8 - JPEG_ConvertorParams.LineOffset;
    JPEG_ConvertorParams.H_factor = 8;
    JPEG_ConvertorParams.V_factor  = 8;
  }
  else
    return HAL_ERROR; /* Color space Not supported*/

  JPEG_ConvertorParams.WidthExtend = JPEG_ConvertorParams.ImageWidth + JPEG_ConvertorParams.LineOffset;
  JPEG_ConvertorParams.ScaledWidth = JPEG_BYTES_PER_PIXEL * JPEG_ConvertorParams.ImageWidth;

  hMCU = (JPEG_ConvertorParams.ImageWidth / JPEG_ConvertorParams.H_factor);
  if((JPEG_ConvertorParams.ImageWidth % JPEG_ConvertorParams.H_factor) != 0)
    hMCU++; /*+1 for horizenatl incomplete MCU */

  vMCU = (JPEG_ConvertorParams.ImageHeight / JPEG_ConvertorParams.V_factor);
  if((JPEG_ConvertorParams.ImageHeight % JPEG_ConvertorParams.V_factor) != 0)
    vMCU++; /*+1 for vertical incomplete MCU */
  JPEG_ConvertorParams.MCU_Total_Nb = (hMCU * vMCU);
  *ImageNbMCUs = JPEG_ConvertorParams.MCU_Total_Nb;

  return HAL_OK;
  }
//}}}

//{{{
void JPEG_InitPostProcColorTables(void) {

  for (int32_t i = 0; i <= 255; i++) {
    int32_t index = (i * 2) - 256;
    CR_RED_LUT[i] = ( (((int32_t) ((1.40200 / 2) * (1L << 16)))  * index) + ((int32_t) 1 << (16 - 1))) >> 16;

    CB_BLUE_LUT[i] = ( (((int32_t) ((1.77200 / 2) * (1L << 16)))  * index) + ((int32_t) 1 << (16 - 1))) >> 16;

    CR_GREEN_LUT[i] = (-((int32_t) ((0.71414 / 2) * (1L << 16)))) * index;
    CB_GREEN_LUT[i] = (-((int32_t) ((0.34414 / 2) * (1L << 16)))) * index;
    }
  }
//}}}
