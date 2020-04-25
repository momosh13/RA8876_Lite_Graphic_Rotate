#include <SPI.h>
#include "Arduino.h"
#include "Print.h"
#include "Ra8876_Lite.h"
#include "pic16bpp_byte.h"
#include "pic16bpp_word.h"

#define DEMO_ASCII_8X12
#define DEMO_ASCII_16X24
#define DEMO_ASCII_32X48

#ifdef DEMO_ASCII_8X12
#include "ascii_table_8x12.h"
#endif

#ifdef DEMO_ASCII_16X24
#include "ascii_table_16x24.h"
#endif

#ifdef DEMO_ASCII_32X48
#include "ascii_table_32x48.h"
#endif

void lcdPutChar8x12(unsigned short x,unsigned short y,unsigned short fgcolor,unsigned short bgcolor,boolean bg_transparent,unsigned char code);
void lcdPutString8x12(unsigned short x,unsigned short y, unsigned short fgcolor, unsigned short bgcolor,boolean bg_transparent,char *ptr);
void lcdPutChar16x24(unsigned short x,unsigned short y,unsigned short fgcolor,unsigned short bgcolor,boolean bg_transparent,unsigned char code);
void lcdPutString16x24(unsigned short x,unsigned short y, unsigned short fgcolor, unsigned short bgcolor,boolean bg_transparent,char *ptr);
void lcdPutChar32x48(unsigned short x,unsigned short y,unsigned short fgcolor,unsigned short bgcolor,boolean bg_transparent, unsigned char code);
void lcdPutString32x48(unsigned short x,unsigned short y, unsigned short fgcolor, unsigned short bgcolor,boolean bg_transparent, char *ptr);

void lcdPutChar8x12_Rotate(unsigned short x,unsigned short y,unsigned short fgcolor,unsigned short bgcolor,boolean bg_transparent,unsigned char code);
void lcdPutString8x12_Rotate(unsigned short x,unsigned short y, unsigned short fgcolor, unsigned short bgcolor,boolean bg_transparent,char *ptr);
void lcdPutChar16x24_Rotate(unsigned short x,unsigned short y,unsigned short fgcolor,unsigned short bgcolor,boolean bg_transparent,unsigned char code);
void lcdPutString16x24_Rotate(unsigned short x,unsigned short y, unsigned short fgcolor, unsigned short bgcolor,boolean bg_transparent,char *ptr);
void lcdPutChar32x48_Rotate(unsigned short x,unsigned short y,unsigned short fgcolor,unsigned short bgcolor,boolean bg_transparent, unsigned char code);
void lcdPutString32x48_Rotate(unsigned short x,unsigned short y, unsigned short fgcolor, unsigned short bgcolor,boolean bg_transparent, char *ptr);

const int RA8876_XNSCS = 52;
const int RA8876_XNRESET = 51;

Ra8876_Lite ra8876lite(RA8876_XNSCS, RA8876_XNRESET);  
   
void setup() {
   Serial.begin(9600);
   Serial.println("RA8876 Lite");

   if (!ra8876lite.begin()) 
   {
   Serial.println("RA8876 or RA8877 Fail");
   while (1);
   }
   Serial.println("RA8876 or RA8877 Pass!");
   
   ra8876lite.displayOn(true);
 
}

void loop() {
   unsigned long i;
   
  //set VDIR=1;
  ra8876lite.lcdRegDataWrite(RA8876_DPCR, XPCLK_INV<<7|RA8876_DISPLAY_ON<<6|RA8876_VDIR_BT<<3|RA8876_OUTPUT_RGB);//12h
  delay(20);
  //set host write memory direction 11b: Bottom to Top then Left to Right.
  ra8876lite.lcdRegWrite(RA8876_MACR);//02h
  ra8876lite.lcdDataWrite(RA8876_DIRECT_WRITE<<6|RA8876_READ_MEMORY_LRTB<<4|RA8876_WRITE_MEMORY_TBLR<<1);
  //set character rotation for use internal or external font rotate  
  ra8876lite.setTextParameter2_Rotate(RA8876_TEXT_FULL_ALIGN_DISABLE, RA8876_TEXT_CHROMA_KEY_DISABLE,RA8876_TEXT_WIDTH_ENLARGEMENT_X1,RA8876_TEXT_HEIGHT_ENLARGEMENT_X1);//cdh

  //clear page1
  ra8876lite.canvasImageStartAddress(PAGE1_START_ADDR);
  ra8876lite.canvasImageWidth(SCREEN_WIDTH);
  ra8876lite.activeWindowXY(0,0);
  ra8876lite.activeWindowWH(SCREEN_WIDTH,SCREEN_HEIGHT); 
  
  ra8876lite.drawSquareFill_Rotate(0, 0, 599, 799, COLOR65K_BLUE);
  
  //write a pixel dot to page1
  ra8876lite.setPixelCursor_Rotate(20,20);
  ra8876lite.ramAccessPrepare();
  ra8876lite.lcdDataWrite(0x00);//RGB565 LSB data
  ra8876lite.lcdDataWrite(0xf8);//RGB565 MSB data
  
  //write a pixel dot to page1
  ra8876lite.setPixelCursor_Rotate(30,30);
  ra8876lite.ramAccessPrepare();
  ra8876lite.lcdDataWrite16bbp(COLOR65K_WHITE);//RGB565 16bpp data
  
  //write a pixel dot to page1
  ra8876lite.putPixel_16bpp_Rotate(40,40,COLOR65K_MAGENTA);
  delay(3000);
  
 //while(1);
  //clear page1
  ra8876lite.canvasImageStartAddress(PAGE1_START_ADDR);
  ra8876lite.canvasImageWidth(SCREEN_WIDTH);
  ra8876lite.activeWindowXY(0,0);
  ra8876lite.activeWindowWH(SCREEN_WIDTH,SCREEN_HEIGHT); 
  ra8876lite.drawSquareFill_Rotate(0, 0, 599, 799, COLOR65K_BLUE);
  

  ra8876lite.putPicture_16bpp_Rotate(0,0,128,128);
  for(i=0;i<16384;i++)
  {
   ra8876lite.lcdDataWrite16bbp(COLOR65K_YELLOW);//RGB565 16bpp data
  }

  ra8876lite.putPicture_16bpp_Rotate(50+128,50+128,128,128,pic16bpp_byte);

  ra8876lite.putPicture_16bpp_Rotate(600-128,800-128,128,128,pic16bpp_word);
  
  delay(3000);
 // while(1);
  //clear page1
  ra8876lite.canvasImageStartAddress(PAGE1_START_ADDR);
  ra8876lite.canvasImageWidth(SCREEN_WIDTH);
  ra8876lite.activeWindowXY(0,0);
  ra8876lite.activeWindowWH(SCREEN_WIDTH,SCREEN_HEIGHT); 
  ra8876lite.drawSquareFill_Rotate(0, 0, 599, 799, COLOR65K_BLUE);
  
  //graphic font 
  #ifdef DEMO_ASCII_8X12
  lcdPutString8x12_Rotate(0,0,0xFFFF,0x0000,true," !\"#$%&'()*+,-./012345678");
  lcdPutString8x12_Rotate(0,12,0xFFFF,0x0000,true,"9:;<=>?@ABCDEFGHIJKLMNOPQ");
  lcdPutString8x12_Rotate(0,24,0xFFFF,0x0000,true,"RSTUVWXYZ[\\]^_`abcdefghij");
  lcdPutString8x12_Rotate(0,36,0xFFFF,0x0000,true,"klmnopqrstuvwxyz{|}~");
  #endif
  
  #ifdef DEMO_ASCII_16X24
  lcdPutString16x24_Rotate(0,48,0xFFFF,0x0000,true," !\"#$%&'()*+,-./012345678");
  lcdPutString16x24_Rotate(0,72,0xFFFF,0x0000,true,"9:;<=>?@ABCDEFGHIJKLMNOPQ");
  lcdPutString16x24_Rotate(0,96,0xFFFF,0x0000,true,"RSTUVWXYZ[\\]^_`abcdefghij");
  lcdPutString16x24_Rotate(0,120,0xFFFF,0x0000,true,"klmnopqrstuvwxyz{|}~");
  #endif
  
  #ifdef DEMO_ASCII_32X48
  lcdPutString32x48_Rotate(0,144,0xFFFF,0x0000,false," !\"#$%&'()*+,-./012345678");
  lcdPutString32x48_Rotate(0,192,0xFFFF,0x0000,false,"9:;<=>?@ABCDEFGHIJKLMNOPQ");
  lcdPutString32x48_Rotate(0,240,0xFFFF,0x0000,false,"RSTUVWXYZ[\\]^_`abcdefghij");
  lcdPutString32x48_Rotate(0,288,0xFFFF,0x0000,false,"klmnopqrstuvwxyz{|}~");
  #endif

  delay(3000);
  
  ra8876lite.canvasImageStartAddress(PAGE1_START_ADDR);
  ra8876lite.canvasImageWidth(SCREEN_WIDTH);
  ra8876lite.activeWindowXY(0,0);
  ra8876lite.activeWindowWH(SCREEN_WIDTH,SCREEN_HEIGHT); 
  ra8876lite.drawSquareFill_Rotate(0, 0, 599, 799, COLOR65K_BLUE);
  
  //internal font 
  ra8876lite.setTextParameter1(RA8876_SELECT_INTERNAL_CGROM,RA8876_CHAR_HEIGHT_16,RA8876_SELECT_8859_1);//cch
  ra8876lite.setTextParameter2_Rotate(RA8876_TEXT_FULL_ALIGN_DISABLE, RA8876_TEXT_CHROMA_KEY_DISABLE,RA8876_TEXT_WIDTH_ENLARGEMENT_X1,RA8876_TEXT_HEIGHT_ENLARGEMENT_X1);
  ra8876lite.textColor(COLOR65K_WHITE,COLOR65K_BLACK);
  ra8876lite.putString_Rotate(10,0,"Show internal font 8x16");
  
  ra8876lite.setTextParameter1(RA8876_SELECT_INTERNAL_CGROM,RA8876_CHAR_HEIGHT_24,RA8876_SELECT_8859_1);//cch
  ra8876lite.setTextParameter2_Rotate(RA8876_TEXT_FULL_ALIGN_DISABLE, RA8876_TEXT_CHROMA_KEY_DISABLE,RA8876_TEXT_WIDTH_ENLARGEMENT_X1,RA8876_TEXT_HEIGHT_ENLARGEMENT_X1);
  ra8876lite.textColor(COLOR65K_BLUE,COLOR65K_MAGENTA);
  ra8876lite.putString_Rotate(10,26,"Show internal font 12x24");
  
  ra8876lite.setTextParameter1(RA8876_SELECT_INTERNAL_CGROM,RA8876_CHAR_HEIGHT_32,RA8876_SELECT_8859_1);//cch
  ra8876lite.setTextParameter2_Rotate(RA8876_TEXT_FULL_ALIGN_DISABLE, RA8876_TEXT_CHROMA_KEY_DISABLE,RA8876_TEXT_WIDTH_ENLARGEMENT_X1,RA8876_TEXT_HEIGHT_ENLARGEMENT_X1);
  ra8876lite.textColor(COLOR65K_RED,COLOR65K_YELLOW);
  ra8876lite.putString_Rotate(10,60,"Show internal font 16x32");
  
  ra8876lite.drawLine_Rotate(40,40,159,159,COLOR65K_RED);
  ra8876lite.drawLine_Rotate(40,159,159,40,COLOR65K_LIGHTRED);
  
  ra8876lite.drawSquare_Rotate(200+30, 50, 399-30, 199-50, COLOR65K_GRAYSCALE23);
  ra8876lite.drawSquareFill_Rotate(420, 20, 579, 179, COLOR65K_GREEN);
  
  ra8876lite.drawTriangle_Rotate(220,250,360,360,250,380,COLOR65K_MAGENTA);
  ra8876lite.drawTriangleFill_Rotate(500,220,580,380,420,380,COLOR65K_LIGHTMAGENTA);

  ra8876lite.drawCircle_Rotate(100,700,30,COLOR65K_YELLOW);
  ra8876lite.drawCircleFill_Rotate(100,500,60,COLOR65K_LIGHTYELLOW);
  
  ra8876lite.drawEllipse_Rotate(300,500,50,80,COLOR65K_CYAN);
  ra8876lite.drawEllipseFill_Rotate(500,500,80,50,COLOR65K_LIGHTCYAN);
  
  ra8876lite.drawCircleSquare_Rotate(150,700, 350, 780, 20, 20, COLOR65K_BROWN);
  ra8876lite.drawCircleSquareFill_Rotate(400,650, 470, 760, 10, 10, COLOR65K_MAGENTA);


  delay(2000);
  //while(1);
  
  ra8876lite.canvasImageStartAddress(PAGE1_START_ADDR);
  ra8876lite.canvasImageWidth(SCREEN_WIDTH);
  ra8876lite.activeWindowXY(0,0);
  ra8876lite.activeWindowWH(SCREEN_WIDTH,SCREEN_HEIGHT); 
  ra8876lite.drawSquareFill_Rotate(0, 0, 599, 799, COLOR65K_BLUE);
  
  ra8876lite.setTextParameter1(RA8876_SELECT_INTERNAL_CGROM,RA8876_CHAR_HEIGHT_32,RA8876_SELECT_8859_1);//cch
  ra8876lite.setTextParameter2_Rotate(RA8876_TEXT_FULL_ALIGN_DISABLE, RA8876_TEXT_CHROMA_KEY_DISABLE,RA8876_TEXT_WIDTH_ENLARGEMENT_X1,RA8876_TEXT_HEIGHT_ENLARGEMENT_X1);
  ra8876lite.textColor(COLOR65K_WHITE,COLOR65K_BLACK);
  
  ra8876lite.putDec_Rotate(10,10,1,2,"n");
  ra8876lite.putDec_Rotate(10,44,2147483647,11,"n");
  ra8876lite.putDec_Rotate(10,78,-12345,10,"n");
  ra8876lite.putDec_Rotate(10,112,-2147483648,11,"n");
  
  ra8876lite.putDec_Rotate(10,146,1,2,"-");
  ra8876lite.putDec_Rotate(10,180,2147483647,11,"-");
  ra8876lite.putDec_Rotate(10,214,-12345,10,"-");
  ra8876lite.putDec_Rotate(10,248,-2147483648,11,"-");
  
  ra8876lite.putDec_Rotate(10,282,1,2,"+");
  ra8876lite.putDec_Rotate(10,316,2147483647,11,"+");
  ra8876lite.putDec_Rotate(10,350,-12345,10,"+");
  ra8876lite.putDec_Rotate(10,384,-2147483648,11,"+");
  
  ra8876lite.putDec_Rotate(10,418,1,2,"0");
  ra8876lite.putDec_Rotate(10,452,2147483647,11,"0");
  ra8876lite.putDec_Rotate(10,486,-12345,10,"0");
  ra8876lite.putDec_Rotate(10,520,-2147483648,11,"0");  
  delay(2000);
  
  ra8876lite.canvasImageStartAddress(PAGE1_START_ADDR);
  ra8876lite.canvasImageWidth(SCREEN_WIDTH);
  ra8876lite.activeWindowXY(0,0);
  ra8876lite.activeWindowWH(SCREEN_WIDTH,SCREEN_HEIGHT); 
  ra8876lite.drawSquareFill_Rotate(0, 0, 599, 799, COLOR65K_BLUE);
  
  ra8876lite.setTextParameter1(RA8876_SELECT_INTERNAL_CGROM,RA8876_CHAR_HEIGHT_32,RA8876_SELECT_8859_1);//cch
  ra8876lite.setTextParameter2_Rotate(RA8876_TEXT_FULL_ALIGN_DISABLE, RA8876_TEXT_CHROMA_KEY_DISABLE,RA8876_TEXT_WIDTH_ENLARGEMENT_X1,RA8876_TEXT_HEIGHT_ENLARGEMENT_X1);
  ra8876lite.textColor(COLOR65K_WHITE,COLOR65K_BLACK);
  
  
  ra8876lite.putFloat_Rotate(10,10,1.1,7,1,"n");
  ra8876lite.putFloat_Rotate(10,44,483647.12,11,2,"n");
  ra8876lite.putFloat_Rotate(10,78,-12345.123,11,3,"n");
  ra8876lite.putFloat_Rotate(10,112,-123456.1234,11,4,"n");
  
  ra8876lite.putFloat_Rotate(10,146,1.1234,7,1,"-");
  ra8876lite.putFloat_Rotate(10,180,483647.12,11,2,"-");
  ra8876lite.putFloat_Rotate(10,214,-12345.123,11,3,"-");
  ra8876lite.putFloat_Rotate(10,248,-123456.1234,11,4,"-");
  
  ra8876lite.putFloat_Rotate(10,282,1.1,7,1,"+");
  ra8876lite.putFloat_Rotate(10,316,483647.12,11,2,"+");
  ra8876lite.putFloat_Rotate(10,350,-12345.123,11,3,"+");
  ra8876lite.putFloat_Rotate(10,384,-123456.1234,11,4,"+");
  
  ra8876lite.putFloat_Rotate(10,418,1.1,7,1,"0");
  ra8876lite.putFloat_Rotate(10,452,483647.12,11,2,"0");
  ra8876lite.putFloat_Rotate(10,486,-12345.123,11,3,"0");
  ra8876lite.putFloat_Rotate(10,520,-123456.1234,11,4,"0");
  delay(2000);
  
  ra8876lite.canvasImageStartAddress(PAGE1_START_ADDR);
  ra8876lite.canvasImageWidth(SCREEN_WIDTH);
  ra8876lite.activeWindowXY(0,0);
  ra8876lite.activeWindowWH(SCREEN_WIDTH,SCREEN_HEIGHT); 
  ra8876lite.drawSquareFill_Rotate(0, 0, 599, 799, COLOR65K_BLUE);
  
  ra8876lite.setTextParameter1(RA8876_SELECT_INTERNAL_CGROM,RA8876_CHAR_HEIGHT_32,RA8876_SELECT_8859_1);//cch
  ra8876lite.setTextParameter2_Rotate(RA8876_TEXT_FULL_ALIGN_DISABLE, RA8876_TEXT_CHROMA_KEY_DISABLE,RA8876_TEXT_WIDTH_ENLARGEMENT_X1,RA8876_TEXT_HEIGHT_ENLARGEMENT_X1);
  ra8876lite.textColor(COLOR65K_WHITE,COLOR65K_BLACK);
  
  ra8876lite.putHex_Rotate(10,10,1,4,"n");
  ra8876lite.putHex_Rotate(10,44,255,6,"n");
  ra8876lite.putHex_Rotate(10,78,0xa7c8,6,"n");
  ra8876lite.putHex_Rotate(10,112,0xdd11ff55,10,"n");
  
  ra8876lite.putHex_Rotate(10,146,1,4,"0");
  ra8876lite.putHex_Rotate(10,180,255,6,"0");
  ra8876lite.putHex_Rotate(10,214,0xa7c8,6,"0");
  ra8876lite.putHex_Rotate(10,248,0xdd11ff55,10,"0");
  
  ra8876lite.putHex_Rotate(10,282,1,4,"#");
  ra8876lite.putHex_Rotate(10,316,255,6,"#");
  ra8876lite.putHex_Rotate(10,350,0xa7c8,6,"#");
  ra8876lite.putHex_Rotate(10,384,0xdd11ff55,10,"#");
  
  ra8876lite.putHex_Rotate(10,418,1,4,"x");
  ra8876lite.putHex_Rotate(10,452,255,6,"x");
  ra8876lite.putHex_Rotate(10,486,0xa7c8,6,"x");
  ra8876lite.putHex_Rotate(10,520,0xdd11ff55,10,"x");
  delay(2000);
  
  while(1);
}


void lcdPutChar8x12(unsigned short x,unsigned short y,unsigned short fgcolor,unsigned short bgcolor,boolean bg_transparent,unsigned char code)
{ unsigned short i=0;
  unsigned short j=0;
  unsigned char tmp_char=0;

  for (i=0;i<12;i++)
  {
    tmp_char = ascii_table_8x12[((code-0x20)*12)+i];//minus 32 offset, because this table from ascii table "space" 
   for (j=0;j<8;j++)
   {
    if ( (tmp_char >>7-j) & 0x01 == 0x01)
        ra8876lite.putPixel_16bpp(x+j,y+i,fgcolor); //
    else
    {   
        if(!bg_transparent)
        ra8876lite.putPixel_16bpp(x+j,y+i,bgcolor); //
    } 
   }
  }
}

void lcdPutString8x12(unsigned short x,unsigned short y, unsigned short fgcolor, unsigned short bgcolor,boolean bg_transparent,char *ptr)
{unsigned short i = 0;
  //screen width = 800,  800/8 = 100 
  //if string more then 100 fonts, no show
  while ((*ptr != 0) & (i < 100))
  {
    lcdPutChar8x12(x, y, fgcolor, bgcolor,bg_transparent, *ptr);
    x += 8;
    ptr++;
    i++;  
  }
}

void lcdPutChar16x24(unsigned short x,unsigned short y,unsigned short fgcolor,unsigned short bgcolor,boolean bg_transparent,unsigned char code)
{ unsigned short i=0;
  unsigned short j=0;
  unsigned long array_addr =0;
  unsigned int tmp_char=0;

  for (i=0;i<24;i++)
  {
    //minus 32 offset, because this table from ascii table "space"  
    array_addr = ((code-0x20)*2*24)+(i*2); 
    tmp_char = ascii_table_16x24[array_addr]<<8|ascii_table_16x24[array_addr+1];
   for (j=0;j<16;j++)
   {
    if ( (tmp_char >>15-j) & 0x01 == 0x01)
        ra8876lite.putPixel_16bpp(x+j,y+i,fgcolor); //
    else
       {
        if(!bg_transparent)
        ra8876lite.putPixel_16bpp(x+j,y+i,bgcolor); // 
       }
   }
  }
}

void lcdPutString16x24(unsigned short x,unsigned short y, unsigned short fgcolor, unsigned short bgcolor,boolean bg_transparent,char *ptr)
{unsigned short i = 0;
  //screen width = 800,  800/16 = 50 
  //if string more then 50 fonts, no show
  while ((*ptr != 0) & (i < 50))
  {
    lcdPutChar16x24(x, y, fgcolor, bgcolor,bg_transparent, *ptr);
    x += 16;
    ptr++;
    i++;  
  }
}

void lcdPutChar32x48(unsigned short x,unsigned short y,unsigned short fgcolor,unsigned short bgcolor,boolean bg_transparent,unsigned char code)
{ unsigned short i=0;
  unsigned short j=0;
  unsigned long array_addr =0;
  unsigned long tmp_char=0;

  for (i=0;i<48;i++)
  {
    //minus 32 offset, because this table from ascii table "space"  
    array_addr = ((code-0x20)*4*48)+(i*4); 
    tmp_char = ascii_table_32x48[array_addr]<<24|ascii_table_32x48[array_addr+1]<<16|ascii_table_32x48[array_addr+2]<<8|ascii_table_32x48[array_addr+3];
    
     for (j=0;j<32;j++)
     {
     if ( (tmp_char >> (31-j)) & 0x01 == 0x01)
        ra8876lite.putPixel_16bpp(x+j,y+i,fgcolor); //
     else
         {
         if(!bg_transparent)
         ra8876lite.putPixel_16bpp(x+j,y+i,bgcolor); // 
         }
      } 
  }
}

void lcdPutString32x48(unsigned short x,unsigned short y, unsigned short fgcolor, unsigned short bgcolor,boolean bg_transparent,char *ptr)
{unsigned short i = 0;
  //screen width = 800,  800/32 = 25 
  //if string more then 25 fonts, no show
  while ((*ptr != 0) & (i < 25))
  {
    lcdPutChar32x48(x, y, fgcolor, bgcolor,bg_transparent, *ptr);
    x += 32;
    ptr++;
    i++;  
  }
}


/*graphic font rotate function*/
void lcdPutChar8x12_Rotate(unsigned short x,unsigned short y,unsigned short fgcolor,unsigned short bgcolor,boolean bg_transparent,unsigned char code)
{ unsigned short i=0;
  unsigned short j=0;
  unsigned char tmp_char=0;

  for (i=0;i<12;i++)
  {
    tmp_char = ascii_table_8x12[((code-0x20)*12)+i];//minus 32 offset, because this table from ascii table "space" 
   for (j=0;j<8;j++)
   {
    if ( (tmp_char >>7-j) & 0x01 == 0x01)
        ra8876lite.putPixel_16bpp_Rotate(x+j,y+i,fgcolor); //
    else
    {   
        if(!bg_transparent)
        ra8876lite.putPixel_16bpp_Rotate(x+j,y+i,bgcolor); //
    } 
   }
  }
}

void lcdPutString8x12_Rotate(unsigned short x,unsigned short y, unsigned short fgcolor, unsigned short bgcolor,boolean bg_transparent,char *ptr)
{unsigned short i = 0;
  //screen width = 800,  800/8 = 100 
  //if string more then 100 fonts, no show
  while ((*ptr != 0) & (i < 100))
  {
    lcdPutChar8x12_Rotate(x, y, fgcolor, bgcolor,bg_transparent, *ptr);
    x += 8;
    ptr++;
    i++;  
  }
}

void lcdPutChar16x24_Rotate(unsigned short x,unsigned short y,unsigned short fgcolor,unsigned short bgcolor,boolean bg_transparent,unsigned char code)
{ unsigned short i=0;
  unsigned short j=0;
  unsigned long array_addr =0;
  unsigned int tmp_char=0;

  for (i=0;i<24;i++)
  {
    //minus 32 offset, because this table from ascii table "space"  
    array_addr = ((code-0x20)*2*24)+(i*2); 
    tmp_char = ascii_table_16x24[array_addr]<<8|ascii_table_16x24[array_addr+1];
   for (j=0;j<16;j++)
   {
    if ( (tmp_char >>15-j) & 0x01 == 0x01)
        ra8876lite.putPixel_16bpp_Rotate(x+j,y+i,fgcolor); //
    else
       {
        if(!bg_transparent)
        ra8876lite.putPixel_16bpp_Rotate(x+j,y+i,bgcolor); // 
       }
   }
  }
}

void lcdPutString16x24_Rotate(unsigned short x,unsigned short y, unsigned short fgcolor, unsigned short bgcolor,boolean bg_transparent,char *ptr)
{unsigned short i = 0;
  //screen width = 800,  800/16 = 50 
  //if string more then 50 fonts, no show
  while ((*ptr != 0) & (i < 50))
  {
    lcdPutChar16x24_Rotate(x, y, fgcolor, bgcolor,bg_transparent, *ptr);
    x += 16;
    ptr++;
    i++;  
  }
}

void lcdPutChar32x48_Rotate(unsigned short x,unsigned short y,unsigned short fgcolor,unsigned short bgcolor,boolean bg_transparent,unsigned char code)
{ unsigned short i=0;
  unsigned short j=0;
  unsigned long array_addr =0;
  unsigned long tmp_char=0;

  for (i=0;i<48;i++)
  {
    //minus 32 offset, because this table from ascii table "space"  
    array_addr = ((code-0x20)*4*48)+(i*4); 
    tmp_char = ascii_table_32x48[array_addr]<<24|ascii_table_32x48[array_addr+1]<<16|ascii_table_32x48[array_addr+2]<<8|ascii_table_32x48[array_addr+3];
    
     for (j=0;j<32;j++)
     {
     if ( (tmp_char >> (31-j)) & 0x01 == 0x01)
        ra8876lite.putPixel_16bpp_Rotate(x+j,y+i,fgcolor); //
     else
         {
         if(!bg_transparent)
         ra8876lite.putPixel_16bpp_Rotate(x+j,y+i,bgcolor); // 
         }
      } 
  }
}

void lcdPutString32x48_Rotate(unsigned short x,unsigned short y, unsigned short fgcolor, unsigned short bgcolor,boolean bg_transparent,char *ptr)
{unsigned short i = 0;
  //screen width = 800,  800/32 = 25 
  //if string more then 25 fonts, no show
  while ((*ptr != 0) & (i < 25))
  {
    lcdPutChar32x48_Rotate(x, y, fgcolor, bgcolor,bg_transparent, *ptr);
    x += 32;
    ptr++;
    i++;  
  }
}
