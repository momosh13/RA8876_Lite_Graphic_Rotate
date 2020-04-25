//**************************************************************//
/*
File Name : Ra8876_Lite.cpp                                   
Author    : RAiO Application Team                             
Edit Date : 09/13/2017
Version   : v2.0  1.modify bte_DestinationMemoryStartAddr bug 
                  2.modify ra8876SdramInitial Auto_Refresh
                  3.modify ra8876PllInitial 
            v2.1  add lcd rotate application
*/
//**************************************************************//
#include <SPI.h>
#include "Ra8876_Lite.h"

#define mydebug
//**************************************************************//
 Ra8876_Lite::Ra8876_Lite(int xnscs,int xnreset)
{
  _xnscs = xnscs;
  _xnreset = xnreset;
}
//**************************************************************//
//**************************************************************//
boolean Ra8876_Lite::begin(void) 
{ 
  ru8 temp;
  //initialize the bus for Due
  SPI.begin(_xnscs);
  //On the Due, the system clock can be divided by values from 1 to 255. 
  //The default value is 21, which sets the clock to 4 MHz like other Arduino boards
  SPI.setClockDivider(_xnscs,21);
  SPI.setDataMode(_xnscs,SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);
  
  //ra8876 hardware reset
  pinMode(_xnreset, OUTPUT); 
  digitalWrite(_xnreset, HIGH);
  delay(1);
  digitalWrite(_xnreset, LOW);
  delay(1);
  digitalWrite(_xnreset, HIGH);
  delay(10);
  if(!checkIcReady())
  {return false;}
  
  #ifdef mydebug
  Serial.println("mydebug"); 
  #endif
  
  //read ID code must disable pll, 01h bit7 set 0
  lcdRegDataWrite(0x01,0x08);
  delay(1);
  if ((lcdRegDataRead(0xff) != 0x76)&&(lcdRegDataRead(0xff) != 0x77))
  {Serial.println("RA8876 or RA8877 not found!");
    return false;}
  else
  {Serial.println("RA8876 or RA8877 connect pass!");}
  
  if(!ra8876Initialize())
  {Serial.println("ra8876 or RA8877 initial fail!");
   return false;}
  else
  {Serial.println("RA8876 or RA8877 initial Pass!");}
  
  SPI.setClockDivider(_xnscs,2);
  return true;
}
//**************************************************************//

boolean Ra8876_Lite::ra8876Initialize(void)
{ ru8 temp;
   
  if(!ra8876PllInitial())
  {Serial.println("PLL initial fail!");
  return false;}
  
  if(!ra8876SdramInitial())
  {Serial.println("SDRAM initial fail!");
  return false;}
  
  lcdRegWrite(RA8876_CCR);//01h
  lcdDataWrite(RA8876_PLL_ENABLE<<7|RA8876_WAIT_NO_MASK<<6|RA8876_KEY_SCAN_DISABLE<<5|RA8876_TFT_OUTPUT24<<3
  |RA8876_I2C_MASTER_DISABLE<<2|RA8876_SERIAL_IF_ENABLE<<1|RA8876_HOST_DATA_BUS_SERIAL);

  lcdRegWrite(RA8876_MACR);//02h
  lcdDataWrite(RA8876_DIRECT_WRITE<<6|RA8876_READ_MEMORY_LRTB<<4|RA8876_WRITE_MEMORY_LRTB<<1);

  lcdRegWrite(RA8876_ICR);//03h
  lcdDataWrite(RA8877_LVDS_FORMAT<<3|RA8876_GRAPHIC_MODE<<2|RA8876_MEMORY_SELECT_IMAGE);

  lcdRegWrite(RA8876_MPWCTR);//10h
  lcdDataWrite(RA8876_PIP1_WINDOW_DISABLE<<7|RA8876_PIP2_WINDOW_DISABLE<<6|RA8876_SELECT_CONFIG_PIP1<<4
  |RA8876_IMAGE_COLOCR_DEPTH_16BPP<<2|TFT_MODE);

  lcdRegWrite(RA8876_PIPCDEP);//11h
  lcdDataWrite(RA8876_PIP1_COLOR_DEPTH_16BPP<<2|RA8876_PIP2_COLOR_DEPTH_16BPP);
  
  lcdRegWrite(RA8876_AW_COLOR);//5Eh
  lcdDataWrite(RA8876_CANVAS_BLOCK_MODE<<2|RA8876_CANVAS_COLOR_DEPTH_16BPP);
  
  lcdRegDataWrite(RA8876_BTE_COLR,RA8876_S0_COLOR_DEPTH_16BPP<<5|RA8876_S1_COLOR_DEPTH_16BPP<<2|RA8876_S0_COLOR_DEPTH_16BPP);//92h
  
  /*TFT timing configure*/
  lcdRegWrite(RA8876_DPCR);//12h
  lcdDataWrite(XPCLK_INV<<7|RA8876_DISPLAY_OFF<<6|RA8876_OUTPUT_RGB);
 
  lcdRegWrite(RA8876_PCSR);//13h
  lcdDataWrite(XHSYNC_INV<<7|XVSYNC_INV<<6|XDE_INV<<5);
    
  lcdHorizontalWidthVerticalHeight(HDW,VDH);
  lcdHorizontalNonDisplay(HND);
  lcdHsyncStartPosition(HST);
  lcdHsyncPulseWidth(HPW);
  lcdVerticalNonDisplay(VND);
  lcdVsyncStartPosition(VST);
  lcdVsyncPulseWidth(VPW);
  
  /*image buffer configure*/
  displayImageStartAddress(PAGE1_START_ADDR);
  displayImageWidth(SCREEN_WIDTH);
  displayWindowStartXY(0,0);
  canvasImageStartAddress(PAGE1_START_ADDR);
  canvasImageWidth(SCREEN_WIDTH);
  activeWindowXY(0,0);
  activeWindowWH(SCREEN_WIDTH,SCREEN_HEIGHT);
  return true;
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::lcdRegWrite(ru8 reg) 
{
  SPI.transfer(_xnscs,RA8876_SPI_CMDWRITE,SPI_CONTINUE);
  SPI.transfer(_xnscs,reg);
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::lcdDataWrite(ru8 data) 
{
  SPI.transfer(_xnscs,RA8876_SPI_DATAWRITE,SPI_CONTINUE);
  SPI.transfer(_xnscs,data);
}
//**************************************************************//
//**************************************************************//
ru8 Ra8876_Lite::lcdDataRead(void) 
{
  SPI.transfer(_xnscs,RA8876_SPI_DATAREAD,SPI_CONTINUE);
  ru8 data = SPI.transfer(_xnscs,0xff);
  return data;
}
//**************************************************************//
//**************************************************************//
ru8 Ra8876_Lite::lcdStatusRead(void) 
{
  SPI.transfer(_xnscs,RA8876_SPI_STATUSREAD,SPI_CONTINUE);
  ru8 data= SPI.transfer(_xnscs,0xff);
  return data;
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::lcdRegDataWrite(ru8 reg,ru8 data)
{
  lcdRegWrite(reg);
  lcdDataWrite(data);
}
//**************************************************************//
//**************************************************************//
ru8 Ra8876_Lite::lcdRegDataRead(ru8 reg)
{
  lcdRegWrite(reg);
  return lcdDataRead();
}

//**************************************************************//
// support SPI interface to write 16bpp data after Regwrite 04h
//**************************************************************//
void Ra8876_Lite::lcdDataWrite16bbp(ru16 data) 
{
  SPI.transfer(_xnscs,RA8876_SPI_DATAWRITE,SPI_CONTINUE);
  SPI.transfer(_xnscs,data,SPI_CONTINUE);
  SPI.transfer(_xnscs,data>>8);
}

//**************************************************************//
//RA8876 register 
//**************************************************************//
/*[Status Register] bit7  Host Memory Write FIFO full
0: Memory Write FIFO is not full.
1: Memory Write FIFO is full.
Only when Memory Write FIFO is not full, MPU may write another one pixel.*/ 
//**************************************************************//
void Ra8876_Lite::checkWriteFifoNotFull(void)
{  ru16 i;  
   for(i=0;i<10000;i++) //Please according to your usage to modify i value.
   {
    if( (lcdStatusRead()&0x80)==0 ){break;}
   }
}

//**************************************************************//
/*[Status Register] bit6  Host Memory Write FIFO empty
0: Memory Write FIFO is not empty.
1: Memory Write FIFO is empty.
When Memory Write FIFO is empty, MPU may write 8bpp data 64
pixels, or 16bpp data 32 pixels, 24bpp data 16 pixels directly.*/
//**************************************************************//
void Ra8876_Lite::checkWriteFifoEmpty(void)
{ ru16 i;
   for(i=0;i<10000;i++)   //Please according to your usage to modify i value.
   {
    if( (lcdStatusRead()&0x40)==0x40 ){break;}
   }
}

//**************************************************************//
/*[Status Register] bit5  Host Memory Read FIFO full
0: Memory Read FIFO is not full.
1: Memory Read FIFO is full.
When Memory Read FIFO is full, MPU may read 8bpp data 32
pixels, or 16bpp data 16 pixels, 24bpp data 8 pixels directly.*/
//**************************************************************//
void Ra8876_Lite::checkReadFifoNotFull(void)
{ ru16 i;
  for(i=0;i<10000;i++)  //Please according to your usage to modify i value.
  {if( (lcdStatusRead()&0x20)==0x00 ){break;}}
}

//**************************************************************//
/*[Status Register] bit4   Host Memory Read FIFO empty
0: Memory Read FIFO is not empty.
1: Memory Read FIFO is empty.*/
//**************************************************************//
void Ra8876_Lite::checkReadFifoNotEmpty(void)
{ ru16 i;
  for(i=0;i<10000;i++)// //Please according to your usage to modify i value. 
  {if( (lcdStatusRead()&0x10)==0x00 ){break;}}
}

//**************************************************************//
/*[Status Register] bit3   Core task is busy
Following task is running:
BTE, Geometry engine, Serial flash DMA, Text write or Graphic write
0: task is done or idle.   1: task is busy*/
//**************************************************************//
void Ra8876_Lite::check2dBusy(void)
{  ru32 i;
   for(i=0;i<1000000;i++)   //Please according to your usage to modify i value.
   {
    delayMicroseconds(1);
    if( (lcdStatusRead()&0x08)==0x00 )
    {break;}
   }
}  

//**************************************************************//
/*[Status Register] bit2   SDRAM ready for access
0: SDRAM is not ready for access   1: SDRAM is ready for access*/	
//**************************************************************//
boolean Ra8876_Lite::checkSdramReady(void)
{ru32 i;
 for(i=0;i<1000000;i++) //Please according to your usage to modify i value.
 { 
   delayMicroseconds(1);
   if( (lcdStatusRead()&0x04)==0x04 )
    {return true;}
    
 }
 return false;
}

//**************************************************************//
/*[Status Register] bit1  Operation mode status
0: Normal operation state  1: Inhibit operation state
Inhibit operation state means internal reset event keep running or
initial display still running or chip enter power saving state.	*/
//**************************************************************//
boolean Ra8876_Lite::checkIcReady(void)
{ru32 i;
  for(i=0;i<1000000;i++)  //Please according to your usage to modify i value.
   {
     delayMicroseconds(1);
     if( (lcdStatusRead()&0x02)==0x00 )
     {return true;}     
   }
   return false;
}
//**************************************************************//
//**************************************************************//
//[05h] [06h] [07h] [08h] [09h] [0Ah]
//------------------------------------//----------------------------------*/
boolean Ra8876_Lite::ra8876PllInitial(void) 
{
/*(1) 10MHz <= OSC_FREQ <= 15MHz 
  (2) 10MHz <= (OSC_FREQ/PLLDIVM) <= 40MHz
  (3) 250MHz <= [OSC_FREQ/(PLLDIVM+1)]x(PLLDIVN+1) <= 600MHz
PLLDIVM:0
PLLDIVN:1~63
PLLDIVK:CPLL & MPLL = 1/2/4/8.SPLL = 1/2/4/8/16/32/64/128.
ex:
 OSC_FREQ = 10MHz
 Set X_DIVK=2
 Set X_DIVM=0
 => (X_DIVN+1)=(XPLLx4)/10*/
ru16 x_Divide,PLLC1,PLLC2;
ru16 pll_m_lo, pll_m_hi;
ru8 temp;

	// Set tft output pixel clock
		if(SCAN_FREQ>=79)								//&&(SCAN_FREQ<=100))
		{
			lcdRegDataWrite(0x05,0x04);				//PLL Divided by 4
			lcdRegDataWrite(0x06,(SCAN_FREQ*4/OSC_FREQ)-1);
		}
		else if((SCAN_FREQ>=63)&&(SCAN_FREQ<=78))
		{
			lcdRegDataWrite(0x05,0x05);				//PLL Divided by 4
			lcdRegDataWrite(0x06,(SCAN_FREQ*8/OSC_FREQ)-1);
		}
		else if((SCAN_FREQ>=40)&&(SCAN_FREQ<=62))
		{								  	
			lcdRegDataWrite(0x05,0x06);				//PLL Divided by 8
			lcdRegDataWrite(0x06,(SCAN_FREQ*8/OSC_FREQ)-1);
		}
		else if((SCAN_FREQ>=32)&&(SCAN_FREQ<=39))
		{								  	
			lcdRegDataWrite(0x05,0x07);				//PLL Divided by 8
			lcdRegDataWrite(0x06,(SCAN_FREQ*16/OSC_FREQ)-1);
		}
		else if((SCAN_FREQ>=16)&&(SCAN_FREQ<=31))
		{								  	
			lcdRegDataWrite(0x05,0x16);				//PLL Divided by 16
			lcdRegDataWrite(0x06,(SCAN_FREQ*16/OSC_FREQ)-1);
		}
		else if((SCAN_FREQ>=8)&&(SCAN_FREQ<=15))
		{
			lcdRegDataWrite(0x05,0x26);				//PLL Divided by 32
			lcdRegDataWrite(0x06,(SCAN_FREQ*32/OSC_FREQ)-1);
		}
		else if((SCAN_FREQ>0)&&(SCAN_FREQ<=7))
		{
			lcdRegDataWrite(0x05,0x36);				//PLL Divided by 64
			lcdRegDataWrite(0x06,(SCAN_FREQ*64/OSC_FREQ)-1);
		}								    

	
		// Set internal Buffer Ram clock
		if(DRAM_FREQ>=158)							//
		{
			lcdRegDataWrite(0x07,0x02);				//PLL Divided by 4
			lcdRegDataWrite(0x08,(DRAM_FREQ*2/OSC_FREQ)-1);
		}
		else if((DRAM_FREQ>=125)&&(DRAM_FREQ<=157))							
		{
			lcdRegDataWrite(0x07,0x03);				//PLL Divided by 4
			lcdRegDataWrite(0x08,(DRAM_FREQ*4/OSC_FREQ)-1);
		}
		else if((DRAM_FREQ>=79)&&(DRAM_FREQ<=124))					
		{
			lcdRegDataWrite(0x07,0x04);				//PLL Divided by 4
			lcdRegDataWrite(0x08,(DRAM_FREQ*4/OSC_FREQ)-1);
		}
		else if((DRAM_FREQ>=63)&&(DRAM_FREQ<=78))					
		{
			lcdRegDataWrite(0x07,0x05);				//PLL Divided by 4
			lcdRegDataWrite(0x08,(DRAM_FREQ*8/OSC_FREQ)-1);
		}
		else if((DRAM_FREQ>=40)&&(DRAM_FREQ<=62))
		{								  	
			lcdRegDataWrite(0x07,0x06);				//PLL Divided by 8
			lcdRegDataWrite(0x08,(DRAM_FREQ*8/OSC_FREQ)-1);
		}
		else if((DRAM_FREQ>=32)&&(DRAM_FREQ<=39))
		{								  	
			lcdRegDataWrite(0x07,0x07);				//PLL Divided by 16
			lcdRegDataWrite(0x08,(DRAM_FREQ*16/OSC_FREQ)-1);
		}
		else if(DRAM_FREQ<=31)
		{
			lcdRegDataWrite(0x07,0x06);				//PLL Divided by 8
			lcdRegDataWrite(0x08,(30*8/OSC_FREQ)-1);	//set to 30MHz if out off range
		}


		// Set Core clock
		if(CORE_FREQ>=158)
		{
			lcdRegDataWrite(0x09,0x02);				//PLL Divided by 2
			lcdRegDataWrite(0x0A,(CORE_FREQ*2/OSC_FREQ)-1);
		}
		else if((CORE_FREQ>=125)&&(CORE_FREQ<=157))
		{
			lcdRegDataWrite(0x09,0x03);				//PLL Divided by 4
			lcdRegDataWrite(0x0A,(CORE_FREQ*4/OSC_FREQ)-1);
		}
		else if((CORE_FREQ>=79)&&(CORE_FREQ<=124))					
		{
			lcdRegDataWrite(0x09,0x04);				//PLL Divided by 4
			lcdRegDataWrite(0x0A,(CORE_FREQ*4/OSC_FREQ)-1);
		}
		else if((CORE_FREQ>=63)&&(CORE_FREQ<=78))					
		{
			lcdRegDataWrite(0x09,0x05);				//PLL Divided by 8
			lcdRegDataWrite(0x0A,(CORE_FREQ*8/OSC_FREQ)-1);
		}
		else if((CORE_FREQ>=40)&&(CORE_FREQ<=62))
		{								  	
			lcdRegDataWrite(0x09,0x06);				//PLL Divided by 8
			lcdRegDataWrite(0x0A,(CORE_FREQ*8/OSC_FREQ)-1);
		}
		else if((CORE_FREQ>=32)&&(CORE_FREQ<=39))
		{								  	
			lcdRegDataWrite(0x09,0x06);				//PLL Divided by 8
			lcdRegDataWrite(0x0A,(CORE_FREQ*8/OSC_FREQ)-1);
		}
		else if(CORE_FREQ<=31)
		{
			lcdRegDataWrite(0x09,0x06);				//PLL Divided by 8
			lcdRegDataWrite(0x0A,(30*8/OSC_FREQ)-1);	//set to 30MHz if out off range
		}

  delay(1);
  lcdRegWrite(0x01);
  lcdDataWrite(0x80);
  delay(2);//wait for pll stable
  if((lcdDataRead()&0x80)==0x80)
  return true;
  else
  return false; 
}

//**************************************************************//
boolean Ra8876_Lite::ra8876SdramInitial(void)
{
ru8	CAS_Latency;
ru16	Auto_Refresh;

#ifdef IS42SM16160D
  if(DRAM_FREQ<=133)	
  CAS_Latency=2;
  else 				
  CAS_Latency=3;

  Auto_Refresh=(64*DRAM_FREQ*1000)/(8192);
  Auto_Refresh=Auto_Refresh-2; 
  lcdRegDataWrite(0xe0,0xf9);        
  lcdRegDataWrite(0xe1,CAS_Latency);      //CAS:2=0x02，CAS:3=0x03
  lcdRegDataWrite(0xe2,Auto_Refresh);
  lcdRegDataWrite(0xe3,Auto_Refresh>>8);
  lcdRegDataWrite(0xe4,0x09);
 #endif

 #ifdef IS42S16320B	
  if(DRAM_FREQ<=133)	
  CAS_Latency=2;
  else 				
  CAS_Latency=3;	
  
  Auto_Refresh=(64*DRAM_FREQ*1000)/(8192);
  Auto_Refresh=Auto_Refresh-2; 
  lcdRegDataWrite(0xe0,0x32);	
  lcdRegDataWrite(0xe1,CAS_Latency);
  lcdRegDataWrite(0xe2,Auto_Refresh);
  lcdRegDataWrite(0xe3,Auto_Refresh>>8);
  lcdRegDataWrite(0xe4,0x09);
 #endif

#ifdef IS42S16400F
  if(DRAM_FREQ<143)	
  CAS_Latency=2;
  else 
  CAS_Latency=3;
  
  Auto_Refresh=(64*DRAM_FREQ*1000)/(4096);
  Auto_Refresh=Auto_Refresh-2; 
  lcdRegDataWrite(0xe0,0x28);      
  lcdRegDataWrite(0xe1,CAS_Latency);      //CAS:2=0x02，CAS:3=0x03
  lcdRegDataWrite(0xe2,Auto_Refresh);
  lcdRegDataWrite(0xe3,Auto_Refresh>>8);
  lcdRegDataWrite(0xe4,0x01);
 #endif

 #ifdef M12L32162A
  CAS_Latency=3;
  Auto_Refresh=(64*DRAM_FREQ*1000)/(4096);
  Auto_Refresh=Auto_Refresh-2; 
  lcdRegDataWrite(0xe0,0x08);      
  lcdRegDataWrite(0xe1,CAS_Latency);      //CAS:2=0x02，CAS:3=0x03
  lcdRegDataWrite(0xe2,Auto_Refresh);
  lcdRegDataWrite(0xe3,Auto_Refresh>>8);
  lcdRegDataWrite(0xe4,0x09);
 #endif

 #ifdef M12L2561616A
  CAS_Latency=3;	
  Auto_Refresh=(64*DRAM_FREQ*1000)/(8192);
  Auto_Refresh=Auto_Refresh-2; 
  lcdRegDataWrite(0xe0,0x31);      
  lcdRegDataWrite(0xe1,CAS_Latency);      //CAS:2=0x02，CAS:3=0x03
  lcdRegDataWrite(0xe2,Auto_Refresh);
  lcdRegDataWrite(0xe3,Auto_Refresh>>8);
  lcdRegDataWrite(0xe4,0x01);
 #endif

 #ifdef M12L64164A
  CAS_Latency=3;
  Auto_Refresh=(64*DRAM_FREQ*1000)/(4096);
  Auto_Refresh=Auto_Refresh-2; 
  lcdRegDataWrite(0xe0,0x28);      
  lcdRegDataWrite(0xe1,CAS_Latency);      //CAS:2=0x02，CAS:3=0x03
  lcdRegDataWrite(0xe2,Auto_Refresh);
  lcdRegDataWrite(0xe3,Auto_Refresh>>8);
  lcdRegDataWrite(0xe4,0x09);
 #endif

 #ifdef W9825G6JH
  CAS_Latency=3;
  Auto_Refresh=(64*DRAM_FREQ*1000)/(4096);
  Auto_Refresh=Auto_Refresh-2; 
  lcdRegDataWrite(0xe0,0x31);      
  lcdRegDataWrite(0xe1,CAS_Latency);      //CAS:2=0x02，CAS:3=0x03
  lcdRegDataWrite(0xe2,Auto_Refresh);
  lcdRegDataWrite(0xe3,Auto_Refresh>>8);
  lcdRegDataWrite(0xe4,0x01);
 #endif

 #ifdef W9812G6JH
  CAS_Latency=3;
  Auto_Refresh=(64*DRAM_FREQ*1000)/(4096);	
  Auto_Refresh=Auto_Refresh-2; 
  lcdRegDataWrite(0xe0,0x29);      
  lcdRegDataWrite(0xe1,CAS_Latency);      //CAS:2=0x02，CAS:3=0x03
  lcdRegDataWrite(0xe2,Auto_Refresh);
  lcdRegDataWrite(0xe3,Auto_Refresh>>8);
  lcdRegDataWrite(0xe4,0x01);
 #endif

 #ifdef MT48LC4M16A
  CAS_Latency=3;
  Auto_Refresh=(64*DRAM_FREQ*1000)/(4096);
	Auto_Refresh=Auto_Refresh-2; 
  lcdRegDataWrite(0xe0,0x28);      
  lcdRegDataWrite(0xe1,CAS_Latency);      //CAS:2=0x02，CAS:3=0x03
  lcdRegDataWrite(0xe2,Auto_Refresh);
  lcdRegDataWrite(0xe3,Auto_Refresh>>8);
  lcdRegDataWrite(0xe4,0x01);
 #endif

 #ifdef K4S641632N
  CAS_Latency=3;
  Auto_Refresh=(64*DRAM_FREQ*1000)/(4096);
  Auto_Refresh=Auto_Refresh-2; 
  lcdRegDataWrite(0xe0,0x28);      
  lcdRegDataWrite(0xe1,CAS_Latency);      //CAS:2=0x02，CAS:3=0x03
  lcdRegDataWrite(0xe2,Auto_Refresh);
  lcdRegDataWrite(0xe3,Auto_Refresh>>8);
  lcdRegDataWrite(0xe4,0x01);
#endif

#ifdef K4S281632K
  CAS_Latency=3;
  Auto_Refresh=(64*DRAM_FREQ*1000)/(4096);	
  Auto_Refresh=Auto_Refresh-2; 
  lcdRegDataWrite(0xe0,0x29);      
  lcdRegDataWrite(0xe1,CAS_Latency);      //CAS:2=0x02，CAS:3=0x03
  lcdRegDataWrite(0xe2,Auto_Refresh);
  lcdRegDataWrite(0xe3,Auto_Refresh>>8);
  lcdRegDataWrite(0xe4,0x01);
 #endif
  checkSdramReady();
}
 //**************************************************************//
//**************************************************************//
 void Ra8876_Lite::displayOn(boolean on)
 {
  if(on)
   lcdRegDataWrite(RA8876_DPCR, XPCLK_INV<<7|RA8876_DISPLAY_ON<<6|RA8876_OUTPUT_RGB);
  else
   lcdRegDataWrite(RA8876_DPCR, XPCLK_INV<<7|RA8876_DISPLAY_OFF<<6|RA8876_OUTPUT_RGB);
   
  delay(20);
 }
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::lcdHorizontalWidthVerticalHeight(ru16 width,ru16 height)
{unsigned char temp;
   temp=(width/8)-1;
   lcdRegDataWrite(RA8876_HDWR,temp);
   temp=width%8;
   lcdRegDataWrite(RA8876_HDWFTR,temp);
   temp=height-1;
   lcdRegDataWrite(RA8876_VDHR0,temp);
   temp=(height-1)>>8;
   lcdRegDataWrite(RA8876_VDHR1,temp);
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::lcdHorizontalNonDisplay(ru16 numbers)
{ru8 temp;
 if(numbers<8)
  {
   lcdRegDataWrite(RA8876_HNDR,0x00);
   lcdRegDataWrite(RA8876_HNDFTR,numbers);
  }
 else
  {
  temp=(numbers/8)-1;
  lcdRegDataWrite(RA8876_HNDR,temp);
  temp=numbers%8;
  lcdRegDataWrite(RA8876_HNDFTR,temp);
  }	
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::lcdHsyncStartPosition(ru16 numbers)
{ru8 temp;
 if(numbers<8)
  {
   lcdRegDataWrite(RA8876_HSTR,0x00);
  }
  else
  {
   temp=(numbers/8)-1;
   lcdRegDataWrite(RA8876_HSTR,temp);
  }
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::lcdHsyncPulseWidth(ru16 numbers)
{ru8 temp;
 if(numbers<8)
  {
   lcdRegDataWrite(RA8876_HPWR,0x00);
  }
  else
  {
   temp=(numbers/8)-1;
   lcdRegDataWrite(RA8876_HPWR,temp);
  }
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::lcdVerticalNonDisplay(ru16 numbers)
{ru8 temp;
  temp=numbers-1;
  lcdRegDataWrite(RA8876_VNDR0,temp);
  lcdRegDataWrite(RA8876_VNDR1,temp>>8);
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::lcdVsyncStartPosition(ru16 numbers)
{ru8 temp;
  temp=numbers-1;
  lcdRegDataWrite(RA8876_VSTR,temp);
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::lcdVsyncPulseWidth(ru16 numbers)
{ru8 temp;
  temp=numbers-1;
  lcdRegDataWrite(RA8876_VPWR,temp);
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::displayImageStartAddress(ru32 addr)	
{
  lcdRegDataWrite(RA8876_MISA0,addr);//20h
  lcdRegDataWrite(RA8876_MISA1,addr>>8);//21h 
  lcdRegDataWrite(RA8876_MISA2,addr>>16);//22h  
  lcdRegDataWrite(RA8876_MISA3,addr>>24);//23h 
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::displayImageWidth(ru16 width)	
{
  lcdRegDataWrite(RA8876_MIW0,width); //24h
  lcdRegDataWrite(RA8876_MIW1,width>>8); //25h 
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::displayWindowStartXY(ru16 x0,ru16 y0)	
{
  lcdRegDataWrite(RA8876_MWULX0,x0);//26h
  lcdRegDataWrite(RA8876_MWULX1,x0>>8);//27h
  lcdRegDataWrite(RA8876_MWULY0,y0);//28h
  lcdRegDataWrite(RA8876_MWULY1,y0>>8);//29h
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::canvasImageStartAddress(ru32 addr)	
{
  lcdRegDataWrite(RA8876_CVSSA0,addr);//50h
  lcdRegDataWrite(RA8876_CVSSA1,addr>>8);//51h
  lcdRegDataWrite(RA8876_CVSSA2,addr>>16);//52h
  lcdRegDataWrite(RA8876_CVSSA3,addr>>24);//53h  
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::canvasImageWidth(ru16 width)	
{
  lcdRegDataWrite(RA8876_CVS_IMWTH0,width);//54h
  lcdRegDataWrite(RA8876_CVS_IMWTH1,width>>8); //55h
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::activeWindowXY(ru16 x0,ru16 y0)	
{
  lcdRegDataWrite(RA8876_AWUL_X0,x0);//56h
  lcdRegDataWrite(RA8876_AWUL_X1,x0>>8);//57h 
  lcdRegDataWrite(RA8876_AWUL_Y0,y0);//58h
  lcdRegDataWrite(RA8876_AWUL_Y1,y0>>8);//59h 
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::activeWindowWH(ru16 width,ru16 height)	
{
  lcdRegDataWrite(RA8876_AW_WTH0,width);//5ah
  lcdRegDataWrite(RA8876_AW_WTH1,width>>8);//5bh
  lcdRegDataWrite(RA8876_AW_HT0,height);//5ch
  lcdRegDataWrite(RA8876_AW_HT1,height>>8);//5dh  
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite:: setPixelCursor(ru16 x,ru16 y)
{
  lcdRegDataWrite(RA8876_CURH0,x); //5fh
  lcdRegDataWrite(RA8876_CURH1,x>>8);//60h
  lcdRegDataWrite(RA8876_CURV0,y);//61h
  lcdRegDataWrite(RA8876_CURV1,y>>8);//62h
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::bte_Source0_MemoryStartAddr(ru32 addr)	
{
  lcdRegDataWrite(RA8876_S0_STR0,addr);//93h
  lcdRegDataWrite(RA8876_S0_STR1,addr>>8);//94h
  lcdRegDataWrite(RA8876_S0_STR2,addr>>16);//95h
  lcdRegDataWrite(RA8876_S0_STR3,addr>>24);////96h
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::bte_Source0_ImageWidth(ru16 width)	
{
  lcdRegDataWrite(RA8876_S0_WTH0,width);//97h
  lcdRegDataWrite(RA8876_S0_WTH1,width>>8);//98h
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::bte_Source0_WindowStartXY(ru16 x0,ru16 y0)	
{
  lcdRegDataWrite(RA8876_S0_X0,x0);//99h
  lcdRegDataWrite(RA8876_S0_X1,x0>>8);//9ah
  lcdRegDataWrite(RA8876_S0_Y0,y0);//9bh
  lcdRegDataWrite(RA8876_S0_Y1,y0>>8);//9ch
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::bte_Source1_MemoryStartAddr(ru32 addr)	
{
  lcdRegDataWrite(RA8876_S1_STR0,addr);//9dh
  lcdRegDataWrite(RA8876_S1_STR1,addr>>8);//9eh
  lcdRegDataWrite(RA8876_S1_STR2,addr>>16);//9fh
  lcdRegDataWrite(RA8876_S1_STR3,addr>>24);//a0h
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::bte_Source1_ImageWidth(ru16 width)	
{
  lcdRegDataWrite(RA8876_S1_WTH0,width);//a1h
  lcdRegDataWrite(RA8876_S1_WTH1,width>>8);//a2h
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::bte_Source1_WindowStartXY(ru16 x0,ru16 y0)	
{
  lcdRegDataWrite(RA8876_S1_X0,x0);//a3h
  lcdRegDataWrite(RA8876_S1_X1,x0>>8);//a4h
  lcdRegDataWrite(RA8876_S1_Y0,y0);//a5h
  lcdRegDataWrite(RA8876_S1_Y1,y0>>8);//a6h
}
//**************************************************************//
//**************************************************************//
void  Ra8876_Lite::bte_DestinationMemoryStartAddr(ru32 addr)	
{
  lcdRegDataWrite(RA8876_DT_STR0,addr);//a7h
  lcdRegDataWrite(RA8876_DT_STR1,addr>>8);//a8h
  lcdRegDataWrite(RA8876_DT_STR2,addr>>16);//a9h
  lcdRegDataWrite(RA8876_DT_STR3,addr>>24);//aah
}
//**************************************************************//
//**************************************************************//
void  Ra8876_Lite::bte_DestinationImageWidth(ru16 width)	
{
  lcdRegDataWrite(RA8876_DT_WTH0,width);//abh
  lcdRegDataWrite(RA8876_DT_WTH1,width>>8);//ach
}
//**************************************************************//
//**************************************************************//
void  Ra8876_Lite::bte_DestinationWindowStartXY(ru16 x0,ru16 y0)	
{
  lcdRegDataWrite(RA8876_DT_X0,x0);//adh
  lcdRegDataWrite(RA8876_DT_X1,x0>>8);//aeh
  lcdRegDataWrite(RA8876_DT_Y0,y0);//afh
  lcdRegDataWrite(RA8876_DT_Y1,y0>>8);//b0h
}
//**************************************************************//
//**************************************************************//
void  Ra8876_Lite::bte_WindowSize(ru16 width, ru16 height)
{
  lcdRegDataWrite(RA8876_BTE_WTH0,width);//b1h
  lcdRegDataWrite(RA8876_BTE_WTH1,width>>8);//b2h
  lcdRegDataWrite(RA8876_BTE_HIG0,height);//b3h
  lcdRegDataWrite(RA8876_BTE_HIG1,height>>8);//b4h
}

//**************************************************************//
/*These 8 bits determine prescaler value for Timer 0 and 1.*/
/*Time base is “Core_Freq / (Prescaler + 1)”*/
//**************************************************************//
void Ra8876_Lite::pwm_Prescaler(ru8 prescaler)
{
  lcdRegDataWrite(RA8876_PSCLR,prescaler);//84h
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::pwm_ClockMuxReg(ru8 pwm1_clk_div, ru8 pwm0_clk_div, ru8 xpwm1_ctrl, ru8 xpwm0_ctrl)
{
  lcdRegDataWrite(RA8876_PMUXR,pwm1_clk_div<<6|pwm0_clk_div<<4|xpwm1_ctrl<<2|xpwm0_ctrl);//85h
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::pwm_Configuration(ru8 pwm1_inverter,ru8 pwm1_auto_reload,ru8 pwm1_start,ru8 
                       pwm0_dead_zone, ru8 pwm0_inverter, ru8 pwm0_auto_reload,ru8 pwm0_start)
 {
  lcdRegDataWrite(RA8876_PCFGR,pwm1_inverter<<6|pwm1_auto_reload<<5|pwm1_start<<4|pwm0_dead_zone<<3|
                  pwm0_inverter<<2|pwm0_auto_reload<<1|pwm0_start);//86h                
 }   
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::pwm0_Duty(ru16 duty)
{
  lcdRegDataWrite(RA8876_TCMPB0L,duty);//88h 
  lcdRegDataWrite(RA8876_TCMPB0H,duty>>8);//89h 
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::pwm0_ClocksPerPeriod(ru16 clocks_per_period)
{
  lcdRegDataWrite(RA8876_TCNTB0L,clocks_per_period);//8ah
  lcdRegDataWrite(RA8876_TCNTB0H,clocks_per_period>>8);//8bh
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::pwm1_Duty(ru16 duty)
{
  lcdRegDataWrite(RA8876_TCMPB1L,duty);//8ch 
  lcdRegDataWrite(RA8876_TCMPB1H,duty>>8);//8dh
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::pwm1_ClocksPerPeriod(ru16 clocks_per_period)
{
  lcdRegDataWrite(RA8876_TCNTB1L,clocks_per_period);//8eh
  lcdRegDataWrite(RA8876_TCNTB1F,clocks_per_period>>8);//8fh
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite:: ramAccessPrepare(void)
{
  lcdRegWrite(RA8876_MRWDP); //04h
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::foreGroundColor16bpp(ru16 color)
{
  lcdRegDataWrite(RA8876_FGCR,color>>8);//d2h
  lcdRegDataWrite(RA8876_FGCG,color>>3);//d3h
  lcdRegDataWrite(RA8876_FGCB,color<<3);//d4h
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::backGroundColor16bpp(ru16 color)
{
  lcdRegDataWrite(RA8876_BGCR,color>>8);//d5h
  lcdRegDataWrite(RA8876_BGCG,color>>3);//d6h
  lcdRegDataWrite(RA8876_BGCB,color<<3);//d7h
}

 /*graphic function*/
//**************************************************************//
/* If use ra8876 graphic mode function turns on */
//**************************************************************//
 void Ra8876_Lite::graphicMode(boolean on)
 {
  if(on)
   lcdRegDataWrite(RA8876_ICR,RA8877_LVDS_FORMAT<<3|RA8876_GRAPHIC_MODE<<2|RA8876_MEMORY_SELECT_IMAGE);//03h  //switch to graphic mode
  else
   lcdRegDataWrite(RA8876_ICR,RA8877_LVDS_FORMAT<<3|RA8876_TEXT_MODE<<2|RA8876_MEMORY_SELECT_IMAGE);//03h  //switch back to text mode
 }
//**************************************************************//
/*support serial IF to write 16bpp pixel*/
//**************************************************************//
void Ra8876_Lite:: putPixel_16bpp(ru16 x,ru16 y,ru16 color)
{
 setPixelCursor(x,y);
 ramAccessPrepare();
 //checkWriteFifoNotFull();//if high speed mcu and without Xnwait check
 //lcdDataWrite(color);
 //lcdDataWrite(color>>8);
 lcdDataWrite16bbp(color);
 //checkWriteFifoEmpty();//if high speed mcu and without Xnwait check
}
//**************************************************************//
/*support ra8876 serial IF to write 16bpp(RGB565) picture data for user operation */
//**************************************************************//
void Ra8876_Lite:: putPicture_16bpp(ru16 x,ru16 y,ru16 width, ru16 height)
{ru16 i,j;
 activeWindowXY(x,y);
 activeWindowWH(width,height);
 setPixelCursor(x,y);
 ramAccessPrepare();
}
//**************************************************************//
/*support serial IF to write 16bpp(RGB565) picture data byte format from data pointer*/
//**************************************************************//
void Ra8876_Lite:: putPicture_16bpp(ru16 x,ru16 y,ru16 width, ru16 height, const unsigned char *data)
{ru16 i,j;

 activeWindowXY(x,y);
 activeWindowWH(width,height);
 setPixelCursor(x,y);
 ramAccessPrepare();
 for(j=0;j<height;j++)
 {
  for(i=0;i<width;i++)
  {
   //checkWriteFifoNotFull();//if high speed mcu and without Xnwait check
   lcdDataWrite(*data);
   data++;
   //checkWriteFifoNotFull();//if high speed mcu and without Xnwait check
   lcdDataWrite(*data);
   data++;
  }
 } 
 checkWriteFifoEmpty();//if high speed mcu and without Xnwait check
 activeWindowXY(0,0);
 activeWindowWH(SCREEN_WIDTH,SCREEN_HEIGHT);
}
//**************************************************************//
/*support serial IF to write 16bpp(RGB565) picture data word format from data pointer*/
//**************************************************************//
void Ra8876_Lite:: putPicture_16bpp(ru16 x,ru16 y,ru16 width, ru16 height, const unsigned short *data)
{ru16 i,j;
 activeWindowXY(x,y);
 activeWindowWH(width,height);
 setPixelCursor(x,y);
 ramAccessPrepare();
 for(j=0;j<height;j++)
 {
  for(i=0;i<width;i++)
  {
   //checkWriteFifoNotFull();//if high speed mcu and without Xnwait check
   lcdDataWrite16bbp(*data);
   data++;
   //checkWriteFifoEmpty();//if high speed mcu and without Xnwait check
  }
 } 
 checkWriteFifoEmpty();//if high speed mcu and without Xnwait check
 activeWindowXY(0,0);
 activeWindowWH(SCREEN_WIDTH,SCREEN_HEIGHT);
}

/*text function*/
//**************************************************************//
/* If use ra8876 text mode function turns on */
//**************************************************************//
void Ra8876_Lite::textMode(boolean on)
 {
  if(on)
   lcdRegDataWrite(RA8876_ICR,RA8877_LVDS_FORMAT<<3|RA8876_TEXT_MODE<<2|RA8876_MEMORY_SELECT_IMAGE);//03h  //switch to text mode
  else
   lcdRegDataWrite(RA8876_ICR,RA8877_LVDS_FORMAT<<3|RA8876_GRAPHIC_MODE<<2|RA8876_MEMORY_SELECT_IMAGE);//03h  //switch back to graphic mode

 }
 //**************************************************************//
 //**************************************************************//
void Ra8876_Lite::textColor(ru16 foreground_color,ru16 background_color)
 {
   foreGroundColor16bpp(foreground_color);
   backGroundColor16bpp(background_color);
 }
 //**************************************************************//
 //**************************************************************//
void Ra8876_Lite:: setTextCursor(ru16 x,ru16 y)
{
  lcdRegDataWrite(RA8876_F_CURX0,x); //63h
  lcdRegDataWrite(RA8876_F_CURX1,x>>8);//64h
  lcdRegDataWrite(RA8876_F_CURY0,y);//65h
  lcdRegDataWrite(RA8876_F_CURY1,y>>8);//66h
}
 //**************************************************************//
 /* source_select = 0 : internal CGROM,  source_select = 1: external CGROM, source_select = 2: user-define*/
 /* size_select = 0 : 8*16/16*16, size_select = 1 : 12*24/24*24, size_select = 2 : 16*32/32*32  */
 /* iso_select = 0 : iso8859-1, iso_select = 1 : iso8859-2, iso_select = 2 : iso8859-4, iso_select = 3 : iso8859-5*/
 //**************************************************************//
 void Ra8876_Lite::setTextParameter1(ru8 source_select,ru8 size_select,ru8 iso_select)//cch
 {
   lcdRegDataWrite(RA8876_CCR0,source_select<<6|size_select<<4|iso_select);//cch
 }
 //**************************************************************//
 /*align = 0 : full alignment disable, align = 1 : full alignment enable*/
 /*chroma_key = 0 : text with chroma key disable, chroma_key = 1 : text with chroma key enable*/
 /* width_enlarge and height_enlarge can be set 0~3, (00b: X1) (01b : X2)  (10b : X3)  (11b : X4)*/
 //**************************************************************//
void Ra8876_Lite::setTextParameter2(ru8 align, ru8 chroma_key, ru8 width_enlarge, ru8 height_enlarge)
{
  lcdRegDataWrite(RA8876_CCR1,align<<7|chroma_key<<6|width_enlarge<<2|height_enlarge);//cdh
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::genitopCharacterRomParameter(ru8 scs_select, ru8 clk_div, ru8 rom_select, ru8 character_select, ru8 gt_width)
{ 
  if(scs_select==0)
  lcdRegDataWrite(RA8876_SFL_CTRL,RA8876_SERIAL_FLASH_SELECT0<<7|RA8876_SERIAL_FLASH_FONT_MODE<<6|RA8876_SERIAL_FLASH_ADDR_24BIT<<5|RA8876_FOLLOW_RA8876_MODE<<4|RA8876_SPI_FAST_READ_8DUMMY);//b7h
  if(scs_select==1)
  lcdRegDataWrite(RA8876_SFL_CTRL,RA8876_SERIAL_FLASH_SELECT1<<7|RA8876_SERIAL_FLASH_FONT_MODE<<6|RA8876_SERIAL_FLASH_ADDR_24BIT<<5|RA8876_FOLLOW_RA8876_MODE<<4|RA8876_SPI_FAST_READ_8DUMMY);//b7h
  
  lcdRegDataWrite(RA8876_SPI_DIVSOR,clk_div);//bbh 
  
  lcdRegDataWrite(RA8876_GTFNT_SEL,rom_select<<5);//ceh
  lcdRegDataWrite(RA8876_GTFNT_CR,character_select<<3|gt_width);//cfh
}
//**************************************************************//
//support ra8876 internal font and external string font code write from data pointer
//**************************************************************//
void Ra8876_Lite:: putString(ru16 x0,ru16 y0, char *str)
{
  textMode(true);
  setTextCursor(x0,y0);
  ramAccessPrepare();
  while(*str != '\0')
  {
  checkWriteFifoNotFull();  
  lcdDataWrite(*str);
  ++str; 
  } 
  check2dBusy();
  textMode(false);
}
/*put value,base on sprintf*/
//**************************************************************//
//vaule: -2147483648(-2^31) ~ 2147483647(2^31-1)
//len: 1~11 minimum output length
/*
  [flag] 
    n:靠右　
    -:靠左
    +:輸出正負號
    (space):當不輸出正負號時，就輸出空白
    0:在開頭處(左側) 補 0，而非補空白。 
 */
//**************************************************************//
void Ra8876_Lite:: putDec(ru16 x0,ru16 y0,rs32 vaule,ru8 len,const char *flag)
{
  char char_buffer[12];
  switch(len)
  {
    case 1:
           if(flag=="n")
           {sprintf(char_buffer ,"%1d", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="-")
           {sprintf(char_buffer ,"%-1d", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="+")
           {sprintf(char_buffer ,"%+1d", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="0")
           {sprintf(char_buffer ,"%01d", vaule); putString(x0,y0,char_buffer);}
           break;
    case 2:
           if(flag=="n")
           {sprintf(char_buffer ,"%2d", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="-")
           { sprintf(char_buffer ,"%-2d", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="+")
           { sprintf(char_buffer ,"%+2d", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="0")
           { sprintf(char_buffer ,"%02d", vaule); putString(x0,y0,char_buffer);}
           break; 
    case 3: 
           if(flag=="n")
           {sprintf(char_buffer ,"%3d", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="-")
           {sprintf(char_buffer ,"%-3d", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="+")
           {sprintf(char_buffer ,"%+3d", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="0")
           {sprintf(char_buffer ,"%03d", vaule); putString(x0,y0,char_buffer);}
           break;
    case 4: 
           if(flag=="n")
           {sprintf(char_buffer ,"%4d", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="-")
           { sprintf(char_buffer ,"%-4d", vaule);  putString(x0,y0,char_buffer);}
           else if(flag=="+")
           { sprintf(char_buffer ,"%+4d", vaule);  putString(x0,y0,char_buffer);}
           else if(flag=="0")
           { sprintf(char_buffer ,"%04d", vaule);  putString(x0,y0,char_buffer);}
           break;
    case 5: 
           if(flag=="n")
           {sprintf(char_buffer ,"%5d", vaule); putString(x0,y0,char_buffer);}    
           else if(flag=="-")
           {sprintf(char_buffer ,"%-5d", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="+")
           {sprintf(char_buffer ,"%+5d", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="0")
           {sprintf(char_buffer ,"%05d", vaule); putString(x0,y0,char_buffer);}
           break;
    case 6: 
           if(flag=="n")
           {sprintf(char_buffer ,"%6d", vaule); putString(x0,y0,char_buffer);}    
           else if(flag=="-")
           {sprintf(char_buffer ,"%-6d", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="+")
           {sprintf(char_buffer ,"%+6d", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="0")
           {sprintf(char_buffer ,"%06d", vaule); putString(x0,y0,char_buffer);}
           break; 
    case 7: 
           if(flag=="n")
           {sprintf(char_buffer ,"%7d", vaule); putString(x0,y0,char_buffer);} 
           else if(flag=="-")
           {sprintf(char_buffer ,"%-7d", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="+")
           {sprintf(char_buffer ,"%+7d", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="0")
           {sprintf(char_buffer ,"%07d", vaule); putString(x0,y0,char_buffer);}
           break;
    case 8: 
           if(flag=="n")
           {sprintf(char_buffer ,"%8d", vaule); putString(x0,y0,char_buffer);}     
           else if(flag=="-")
           {sprintf(char_buffer ,"%-8d", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="+")
           {sprintf(char_buffer ,"%+8d", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="0")
           {sprintf(char_buffer ,"%08d", vaule); putString(x0,y0,char_buffer);}
           break;
    case 9:
           if(flag=="n")
           {sprintf(char_buffer ,"%9d", vaule); putString(x0,y0,char_buffer);}  
           else if(flag=="-")
           {sprintf(char_buffer ,"%-9d", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="+")
           {sprintf(char_buffer ,"%+9d", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="0")
           {sprintf(char_buffer ,"%09d", vaule); putString(x0,y0,char_buffer);}
           break;
    case 10:
           if(flag=="n")
           {sprintf(char_buffer ,"%10d", vaule); putString(x0,y0,char_buffer);}  
           else if(flag=="-")
           {sprintf(char_buffer ,"%-10d", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="+")
           {sprintf(char_buffer ,"%+10d", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="0")
           {sprintf(char_buffer ,"%010d", vaule); putString(x0,y0,char_buffer);}
           break;
    case 11: 
           if(flag=="n")
           {sprintf(char_buffer ,"%11d", vaule); putString(x0,y0,char_buffer);} 
           else if(flag=="-")
           {sprintf(char_buffer ,"%-11d", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="+")
           {sprintf(char_buffer ,"%+11d", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="0")
           {sprintf(char_buffer ,"%011d", vaule); putString(x0,y0,char_buffer);}
           break;       
    default:   
           break;    
  }
}

//**************************************************************//
//vaule: (3.4E-38) ~ (3.4E38)
//len: 1~11 minimum output length
//precision: right side of point numbers 1~4 
/*
  [flag] 
    n:靠右　
    -:靠左
    +:輸出正負號
    (space):當不輸出正負號時，就輸出空白
    #: 強制輸出小數點
    0:在開頭處(左側) 補 0，而非補空白。
    
 */
 //arduino Floats have only 6-7 decimal digits of precision. That means the total number of digits, not the number to the right of the decimal point. 
 //Unlike other platforms, where you can get more precision by using a double (e.g. up to 15 digits), on the Arduino, double is the same size as float.
//**************************************************************//
void Ra8876_Lite:: putFloat(ru16 x0,ru16 y0,double vaule,ru8 len,ru8 precision, const char *flag)
{
  char char_buffer[20];
  switch(len)
  {
    case 1:
           if(flag=="n")
           {
             if(precision==1)
             sprintf(char_buffer ,"%1.1f", vaule); putString(x0,y0,char_buffer);
             if(precision==2)
             sprintf(char_buffer ,"%1.2f", vaule); putString(x0,y0,char_buffer);
             if(precision==3)
             sprintf(char_buffer ,"%1.3f", vaule); putString(x0,y0,char_buffer);
             if(precision==4)
             sprintf(char_buffer ,"%1.4f", vaule); putString(x0,y0,char_buffer);
           }
           else if(flag=="-")
           {
            if(precision==1)
            sprintf(char_buffer ,"%-1.1f", vaule); putString(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%-1.2f", vaule); putString(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%-1.3f", vaule); putString(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%-1.4f", vaule); putString(x0,y0,char_buffer);
           }
           else if(flag=="+")
           {
            if(precision==1)
            sprintf(char_buffer ,"%+1.1f", vaule); putString(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%+1.2f", vaule); putString(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%+1.3f", vaule); putString(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%+1.4f", vaule); putString(x0,y0,char_buffer);
           }
           else if(flag=="0")
           {
            if(precision==1)
            sprintf(char_buffer ,"%01.1f", vaule); putString(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%01.2f", vaule); putString(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%01.3f", vaule); putString(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%01.4f", vaule); putString(x0,y0,char_buffer);
           }
           break;
    case 2:
           if(flag=="n")
           {
           if(precision==1)  
           sprintf(char_buffer ,"%2.1f", vaule); putString(x0,y0,char_buffer);
           if(precision==2)  
           sprintf(char_buffer ,"%2.2f", vaule); putString(x0,y0,char_buffer);
           if(precision==3)  
           sprintf(char_buffer ,"%2.3f", vaule); putString(x0,y0,char_buffer);
           if(precision==4)  
           sprintf(char_buffer ,"%2.4f", vaule); putString(x0,y0,char_buffer);
           }
           else if(flag=="-")
           { 
           if(precision==1) 
           sprintf(char_buffer ,"%-2.1f", vaule); putString(x0,y0,char_buffer);
           if(precision==2) 
           sprintf(char_buffer ,"%-2.2f", vaule); putString(x0,y0,char_buffer);
           if(precision==3) 
           sprintf(char_buffer ,"%-2.3f", vaule); putString(x0,y0,char_buffer);
           if(precision==4) 
           sprintf(char_buffer ,"%-2.4f", vaule); putString(x0,y0,char_buffer);
           }
           else if(flag=="+")
           { 
           if(precision==1)
           sprintf(char_buffer ,"%+2.1f", vaule); putString(x0,y0,char_buffer);
           if(precision==2)
           sprintf(char_buffer ,"%+2.2f", vaule); putString(x0,y0,char_buffer);
           if(precision==3)
           sprintf(char_buffer ,"%+2.3f", vaule); putString(x0,y0,char_buffer);
           if(precision==4)
           sprintf(char_buffer ,"%+2.4f", vaule); putString(x0,y0,char_buffer);
           }
           else if(flag=="0")
           { 
           if(precision==1)
           sprintf(char_buffer ,"%02.1f", vaule); putString(x0,y0,char_buffer);
           if(precision==2)
           sprintf(char_buffer ,"%02.2f", vaule); putString(x0,y0,char_buffer);
           if(precision==3)
           sprintf(char_buffer ,"%02.3f", vaule); putString(x0,y0,char_buffer);
           if(precision==4)
           sprintf(char_buffer ,"%02.4f", vaule); putString(x0,y0,char_buffer);
           }
           break; 
    case 3: 
           if(flag=="n")
           {
            if(precision==1)
            sprintf(char_buffer ,"%3.1f", vaule); putString(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%3.2f", vaule); putString(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%3.3f", vaule); putString(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%3.4f", vaule); putString(x0,y0,char_buffer);
           }
           else if(flag=="-")
           {
           if(precision==1)
           sprintf(char_buffer ,"%-3.1f", vaule); putString(x0,y0,char_buffer);
           if(precision==2)
           sprintf(char_buffer ,"%-3.2f", vaule); putString(x0,y0,char_buffer);
           if(precision==3)
           sprintf(char_buffer ,"%-3.3f", vaule); putString(x0,y0,char_buffer);
           if(precision==4)
           sprintf(char_buffer ,"%-3.4f", vaule); putString(x0,y0,char_buffer);
           }
           else if(flag=="+")
           {
           if(precision==1)
           sprintf(char_buffer ,"%+3.1f", vaule); putString(x0,y0,char_buffer);
           if(precision==2)
           sprintf(char_buffer ,"%+3.2f", vaule); putString(x0,y0,char_buffer);
           if(precision==3)
           sprintf(char_buffer ,"%+3.3f", vaule); putString(x0,y0,char_buffer);
           if(precision==4)
           sprintf(char_buffer ,"%+3.4f", vaule); putString(x0,y0,char_buffer);
           }
           else if(flag=="0")
           {
           if(precision==1)
           sprintf(char_buffer ,"%03.1f", vaule); putString(x0,y0,char_buffer);
           if(precision==2)
           sprintf(char_buffer ,"%03.2f", vaule); putString(x0,y0,char_buffer);
           if(precision==3)
           sprintf(char_buffer ,"%03.3f", vaule); putString(x0,y0,char_buffer);
           if(precision==4)
           sprintf(char_buffer ,"%03.4f", vaule); putString(x0,y0,char_buffer);
           }
           break;
    case 4: 
           if(flag=="n")
           {
            if(precision==1)
            sprintf(char_buffer ,"%4.1f", vaule); putString(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%4.2f", vaule); putString(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%4.3f", vaule); putString(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%4.4f", vaule); putString(x0,y0,char_buffer);
           }
           else if(flag=="-")
           { 
            if(precision==1)
            sprintf(char_buffer ,"%-4.1f", vaule);  putString(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%-4.2f", vaule);  putString(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%-4.3f", vaule);  putString(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%-4.4f", vaule);  putString(x0,y0,char_buffer);
           }
           else if(flag=="+")
           {   
            if(precision==1)
            sprintf(char_buffer ,"%+4.1f", vaule);  putString(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%+4.2f", vaule);  putString(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%+4.3f", vaule);  putString(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%+4.4f", vaule);  putString(x0,y0,char_buffer);
           }
           else if(flag=="0")
           { 
            if(precision==1)
            sprintf(char_buffer ,"%04.1f", vaule);  putString(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%04.2f", vaule);  putString(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%04.3f", vaule);  putString(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%04.4f", vaule);  putString(x0,y0,char_buffer);
           }
           break;
    case 5: 
           if(flag=="n")
           {
            if(precision==1)
            sprintf(char_buffer ,"%5.1f", vaule); putString(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%5.2f", vaule); putString(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%5.3f", vaule); putString(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%5.4f", vaule); putString(x0,y0,char_buffer);
           }    
           else if(flag=="-")
           {
            if(precision==1)
            sprintf(char_buffer ,"%-5.1f", vaule); putString(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%-5.2f", vaule); putString(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%-5.3f", vaule); putString(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%-5.4f", vaule); putString(x0,y0,char_buffer);
           }
           else if(flag=="+")
           {
            if(precision==1)
            sprintf(char_buffer ,"%+5.1f", vaule); putString(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%+5.2f", vaule); putString(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%+5.3f", vaule); putString(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%+5.4f", vaule); putString(x0,y0,char_buffer);
           }
           else if(flag=="0")
           {
            if(precision==1)
            sprintf(char_buffer ,"%05.1f", vaule); putString(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%05.2f", vaule); putString(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%05.3f", vaule); putString(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%05.4f", vaule); putString(x0,y0,char_buffer);
           }
           break;
    case 6: 
           if(flag=="n")
           {
            if(precision==1)
            sprintf(char_buffer ,"%6.1f", vaule); putString(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%6.2f", vaule); putString(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%6.3f", vaule); putString(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%6.4f", vaule); putString(x0,y0,char_buffer);
           }    
           else if(flag=="-")
           {
            if(precision==1)
            sprintf(char_buffer ,"%-6.1f", vaule); putString(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%-6.2f", vaule); putString(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%-6.3f", vaule); putString(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%-6.4f", vaule); putString(x0,y0,char_buffer);
           }
           else if(flag=="+")
           {
            if(precision==1)
            sprintf(char_buffer ,"%+6.1f", vaule); putString(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%+6.2f", vaule); putString(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%+6.3f", vaule); putString(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%+6.4f", vaule); putString(x0,y0,char_buffer);
           }
           else if(flag=="0")
           {
            if(precision==1)
            sprintf(char_buffer ,"%06.1f", vaule); putString(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%06.2f", vaule); putString(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%06.3f", vaule); putString(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%06.4f", vaule); putString(x0,y0,char_buffer);
           }
           break; 
    case 7: 
           if(flag=="n")
           {
            if(precision==1)
            sprintf(char_buffer ,"%7.1f", vaule); putString(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%7.2f", vaule); putString(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%7.3f", vaule); putString(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%7.4f", vaule); putString(x0,y0,char_buffer);
           } 
           else if(flag=="-")
           {
            if(precision==1)
            sprintf(char_buffer ,"%-7.1f", vaule); putString(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%-7.2f", vaule); putString(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%-7.3f", vaule); putString(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%-7.4f", vaule); putString(x0,y0,char_buffer);
           }
           else if(flag=="+")
           {
            if(precision==1)
            sprintf(char_buffer ,"%+7.1f", vaule); putString(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%+7.2f", vaule); putString(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%+7.3f", vaule); putString(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%+7.4f", vaule); putString(x0,y0,char_buffer);
           }
           else if(flag=="0")
           {
            if(precision==1)
            sprintf(char_buffer ,"%07.1f", vaule); putString(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%07.2f", vaule); putString(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%07.3f", vaule); putString(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%07.4f", vaule); putString(x0,y0,char_buffer);
           }
           break;
    case 8: 
           if(flag=="n")
           {
            if(precision==1)
            sprintf(char_buffer ,"%8.1f", vaule); putString(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%8.2f", vaule); putString(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%8.3f", vaule); putString(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%8.4f", vaule); putString(x0,y0,char_buffer);
           }     
           else if(flag=="-")
           {
            if(precision==1)
            sprintf(char_buffer ,"%-8.1f", vaule); putString(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%-8.2f", vaule); putString(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%-8.3f", vaule); putString(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%-8.4f", vaule); putString(x0,y0,char_buffer);
           }
           else if(flag=="+")
           {
            if(precision==1)
            sprintf(char_buffer ,"%+8.1f", vaule); putString(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%+8.2f", vaule); putString(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%+8.3f", vaule); putString(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%+8.4f", vaule); putString(x0,y0,char_buffer);
           }
           else if(flag=="0")
           {
            if(precision==1)
            sprintf(char_buffer ,"%08.1f", vaule); putString(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%08.2f", vaule); putString(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%08.3f", vaule); putString(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%08.4f", vaule); putString(x0,y0,char_buffer);
           }
           break;
    case 9:
           if(flag=="n")
           {
            if(precision==1)
            sprintf(char_buffer ,"%9.1f", vaule); putString(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%9.2f", vaule); putString(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%9.3f", vaule); putString(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%9.4f", vaule); putString(x0,y0,char_buffer);
           }  
           else if(flag=="-")
           {
            if(precision==1)
            sprintf(char_buffer ,"%-9.1f", vaule); putString(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%-9.2f", vaule); putString(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%-9.3f", vaule); putString(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%-9.4f", vaule); putString(x0,y0,char_buffer);
           }
           else if(flag=="+")
           {
            if(precision==1)
            sprintf(char_buffer ,"%+9.1f", vaule); putString(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%+9.2f", vaule); putString(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%+9.3f", vaule); putString(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%+9.4f", vaule); putString(x0,y0,char_buffer);
           }
           else if(flag=="0")
           {
            if(precision==1)
            sprintf(char_buffer ,"%09.1f", vaule); putString(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%09.2f", vaule); putString(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%09.3f", vaule); putString(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%09.4f", vaule); putString(x0,y0,char_buffer);
           }
           break;
    case 10:
           if(flag=="n")
           {
            if(precision==1)
            sprintf(char_buffer ,"%10.1f", vaule); putString(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%10.2f", vaule); putString(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%10.3f", vaule); putString(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%10.4f", vaule); putString(x0,y0,char_buffer);
           }  
           else if(flag=="-")
           {
            if(precision==1)
            sprintf(char_buffer ,"%-10.1f", vaule); putString(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%-10.2f", vaule); putString(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%-10.3f", vaule); putString(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%-10.4f", vaule); putString(x0,y0,char_buffer);
           }
           else if(flag=="+")
           {
            if(precision==1)
            sprintf(char_buffer ,"%+10.1f", vaule); putString(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%+10.2f", vaule); putString(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%+10.3f", vaule); putString(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%+10.4f", vaule); putString(x0,y0,char_buffer);
           }
           else if(flag=="0")
           {
            if(precision==1)
            sprintf(char_buffer ,"%010.1f", vaule); putString(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%010.2f", vaule); putString(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%010.3f", vaule); putString(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%010.4f", vaule); putString(x0,y0,char_buffer);
           }
           break;
    case 11: 
           if(flag=="n")
           {
            if(precision==1)
            sprintf(char_buffer ,"%11.1f", vaule); putString(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%11.2f", vaule); putString(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%11.3f", vaule); putString(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%11.4f", vaule); putString(x0,y0,char_buffer);
           } 
           else if(flag=="-")
           {
            if(precision==1)
            sprintf(char_buffer ,"%-11.1f", vaule); putString(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%-11.2f", vaule); putString(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%-11.3f", vaule); putString(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%-11.4f", vaule); putString(x0,y0,char_buffer);
           }
           else if(flag=="+")
           {
            if(precision==1)
            sprintf(char_buffer ,"%+11.1f", vaule); putString(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%+11.2f", vaule); putString(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%+11.3f", vaule); putString(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%+11.4f", vaule); putString(x0,y0,char_buffer);
           }
           else if(flag=="0")
           {
            if(precision==1)
            sprintf(char_buffer ,"%011.1f", vaule); putString(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%011.2f", vaule); putString(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%011.3f", vaule); putString(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%011.4f", vaule); putString(x0,y0,char_buffer);
           }
           break;       
    default:   
           break;    
  }
}
//**************************************************************//
//vaule: 0x00000000 ~ 0xffffffff
//len: 1~11 minimum output length
/*
  [flag] 
    n:靠右,補空白　
    #:強制輸出 0x 作為開頭.
    0:在開頭處(左側) 補 0，而非補空白。
    x:強制輸出 0x 作為開頭，補 0。 
 */
//**************************************************************//
void Ra8876_Lite:: putHex(ru16 x0,ru16 y0,ru32 vaule,ru8 len,const char *flag)
{
  char char_buffer[12];
  switch(len)
  {
    case 1:
           if(flag=="n")
           {sprintf(char_buffer ,"%1x", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="0")
           {sprintf(char_buffer ,"%01x", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="#")
           {sprintf(char_buffer ,"%#1x", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="x")
           {sprintf(char_buffer ,"%#01x", vaule); putString(x0,y0,char_buffer);}
           break;
    case 2:
           if(flag=="n")
           {sprintf(char_buffer ,"%2x", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="0")
           { sprintf(char_buffer ,"%02x", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="#")
           { sprintf(char_buffer ,"%#2x", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="x")
           { sprintf(char_buffer ,"%#02x", vaule); putString(x0,y0,char_buffer);}
           break; 
    case 3: 
           if(flag=="n")
           {sprintf(char_buffer ,"%3x", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="0")
           {sprintf(char_buffer ,"%03x", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="#")
           {sprintf(char_buffer ,"%#3x", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="x")
           {sprintf(char_buffer ,"%#03x", vaule); putString(x0,y0,char_buffer);}
           break;
    case 4: 
           if(flag=="n")
           {sprintf(char_buffer ,"%4x", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="0")
           { sprintf(char_buffer ,"%04x", vaule);  putString(x0,y0,char_buffer);}
           else if(flag=="#")
           { sprintf(char_buffer ,"%#4x", vaule);  putString(x0,y0,char_buffer);}
           else if(flag=="x")
           { sprintf(char_buffer ,"%#04x", vaule);  putString(x0,y0,char_buffer);}
           break;
    case 5: 
           if(flag=="n")
           {sprintf(char_buffer ,"%5x", vaule); putString(x0,y0,char_buffer);}    
           else if(flag=="0")
           {sprintf(char_buffer ,"%05x", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="#")
           {sprintf(char_buffer ,"%#5x", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="x")
           {sprintf(char_buffer ,"%#05x", vaule); putString(x0,y0,char_buffer);}
           break;
    case 6: 
           if(flag=="n")
           {sprintf(char_buffer ,"%6x", vaule); putString(x0,y0,char_buffer);}    
           else if(flag=="0")
           {sprintf(char_buffer ,"%06x", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="#")
           {sprintf(char_buffer ,"%#6x", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="x")
           {sprintf(char_buffer ,"%#06x", vaule); putString(x0,y0,char_buffer);}
           break; 
    case 7: 
           if(flag=="n")
           {sprintf(char_buffer ,"%7x", vaule); putString(x0,y0,char_buffer);} 
           else if(flag=="0")
           {sprintf(char_buffer ,"%07x", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="#")
           {sprintf(char_buffer ,"%#7x", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="x")
           {sprintf(char_buffer ,"%#07x", vaule); putString(x0,y0,char_buffer);}
           break;
    case 8: 
           if(flag=="n")
           {sprintf(char_buffer ,"%8x", vaule); putString(x0,y0,char_buffer);}     
           else if(flag=="0")
           {sprintf(char_buffer ,"%08x", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="#")
           {sprintf(char_buffer ,"%#8x", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="x")
           {sprintf(char_buffer ,"%#08x", vaule); putString(x0,y0,char_buffer);}
           break;
   case 9: 
           if(flag=="n")
           {sprintf(char_buffer ,"%9x", vaule); putString(x0,y0,char_buffer);} 
           else if(flag=="0")
           {sprintf(char_buffer ,"%09x", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="#")
           {sprintf(char_buffer ,"%#9x", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="x")
           {sprintf(char_buffer ,"%#09x", vaule); putString(x0,y0,char_buffer);}
           break;
   case 10: 
           if(flag=="n")
           {sprintf(char_buffer ,"%10x", vaule); putString(x0,y0,char_buffer);}     
           else if(flag=="0")
           {sprintf(char_buffer ,"%010x", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="#")
           {sprintf(char_buffer ,"%#10x", vaule); putString(x0,y0,char_buffer);}
           else if(flag=="x")
           {sprintf(char_buffer ,"%#010x", vaule); putString(x0,y0,char_buffer);}
           break;
      
    default:   
           break;    
  }
}
 /*draw function*/
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::drawLine(ru16 x0, ru16 y0, ru16 x1, ru16 y1, ru16 color)
{
  foreGroundColor16bpp(color);
  lcdRegDataWrite(RA8876_DLHSR0,x0);//68h
  lcdRegDataWrite(RA8876_DLHSR1,x0>>8);//69h
  lcdRegDataWrite(RA8876_DLVSR0,y0);//6ah
  lcdRegDataWrite(RA8876_DLVSR1,y0>>8);//6bh
  lcdRegDataWrite(RA8876_DLHER0,x1);//6ch
  lcdRegDataWrite(RA8876_DLHER1,x1>>8);//6dh
  lcdRegDataWrite(RA8876_DLVER0,y1);//6eh
  lcdRegDataWrite(RA8876_DLVER1,y1>>8);//6fh        
  lcdRegDataWrite(RA8876_DCR0,RA8876_DRAW_LINE);//67h,0x80
  check2dBusy();
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::drawSquare(ru16 x0, ru16 y0, ru16 x1, ru16 y1, ru16 color)
{
  foreGroundColor16bpp(color);
  lcdRegDataWrite(RA8876_DLHSR0,x0);//68h
  lcdRegDataWrite(RA8876_DLHSR1,x0>>8);//69h
  lcdRegDataWrite(RA8876_DLVSR0,y0);//6ah
  lcdRegDataWrite(RA8876_DLVSR1,y0>>8);//6bh
  lcdRegDataWrite(RA8876_DLHER0,x1);//6ch
  lcdRegDataWrite(RA8876_DLHER1,x1>>8);//6dh
  lcdRegDataWrite(RA8876_DLVER0,y1);//6eh
  lcdRegDataWrite(RA8876_DLVER1,y1>>8);//6fh        
  lcdRegDataWrite(RA8876_DCR1,RA8876_DRAW_SQUARE);//76h,0xa0
  check2dBusy();
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::drawSquareFill(ru16 x0, ru16 y0, ru16 x1, ru16 y1, ru16 color)
{
  foreGroundColor16bpp(color);
  lcdRegDataWrite(RA8876_DLHSR0,x0);//68h
  lcdRegDataWrite(RA8876_DLHSR1,x0>>8);//69h
  lcdRegDataWrite(RA8876_DLVSR0,y0);//6ah
  lcdRegDataWrite(RA8876_DLVSR1,y0>>8);//6bh
  lcdRegDataWrite(RA8876_DLHER0,x1);//6ch
  lcdRegDataWrite(RA8876_DLHER1,x1>>8);//6dh
  lcdRegDataWrite(RA8876_DLVER0,y1);//6eh
  lcdRegDataWrite(RA8876_DLVER1,y1>>8);//6fh        
  lcdRegDataWrite(RA8876_DCR1,RA8876_DRAW_SQUARE_FILL);//76h,0xa0
  check2dBusy();
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::drawCircleSquare(ru16 x0, ru16 y0, ru16 x1, ru16 y1, ru16 xr, ru16 yr, ru16 color)
{
  foreGroundColor16bpp(color);
  lcdRegDataWrite(RA8876_DLHSR0,x0);//68h
  lcdRegDataWrite(RA8876_DLHSR1,x0>>8);//69h
  lcdRegDataWrite(RA8876_DLVSR0,y0);//6ah
  lcdRegDataWrite(RA8876_DLVSR1,y0>>8);//6bh
  lcdRegDataWrite(RA8876_DLHER0,x1);//6ch
  lcdRegDataWrite(RA8876_DLHER1,x1>>8);//6dh
  lcdRegDataWrite(RA8876_DLVER0,y1);//6eh
  lcdRegDataWrite(RA8876_DLVER1,y1>>8);//6fh    
  lcdRegDataWrite(RA8876_ELL_A0,xr);//77h    
  lcdRegDataWrite(RA8876_ELL_A1,xr>>8);//79h 
  lcdRegDataWrite(RA8876_ELL_B0,yr);//7ah    
  lcdRegDataWrite(RA8876_ELL_B1,yr>>8);//7bh
  lcdRegDataWrite(RA8876_DCR1,RA8876_DRAW_CIRCLE_SQUARE);//76h,0xb0
  check2dBusy();
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::drawCircleSquareFill(ru16 x0, ru16 y0, ru16 x1, ru16 y1, ru16 xr, ru16 yr, ru16 color)
{
  foreGroundColor16bpp(color);
  lcdRegDataWrite(RA8876_DLHSR0,x0);//68h
  lcdRegDataWrite(RA8876_DLHSR1,x0>>8);//69h
  lcdRegDataWrite(RA8876_DLVSR0,y0);//6ah
  lcdRegDataWrite(RA8876_DLVSR1,y0>>8);//6bh
  lcdRegDataWrite(RA8876_DLHER0,x1);//6ch
  lcdRegDataWrite(RA8876_DLHER1,x1>>8);//6dh
  lcdRegDataWrite(RA8876_DLVER0,y1);//6eh
  lcdRegDataWrite(RA8876_DLVER1,y1>>8);//6fh    
  lcdRegDataWrite(RA8876_ELL_A0,xr);//77h    
  lcdRegDataWrite(RA8876_ELL_A1,xr>>8);//78h 
  lcdRegDataWrite(RA8876_ELL_B0,yr);//79h    
  lcdRegDataWrite(RA8876_ELL_B1,yr>>8);//7ah
  lcdRegDataWrite(RA8876_DCR1,RA8876_DRAW_CIRCLE_SQUARE_FILL);//76h,0xf0
  check2dBusy();
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::drawTriangle(ru16 x0,ru16 y0,ru16 x1,ru16 y1,ru16 x2,ru16 y2,ru16 color)
{
  foreGroundColor16bpp(color);
  lcdRegDataWrite(RA8876_DLHSR0,x0);//68h
  lcdRegDataWrite(RA8876_DLHSR1,x0>>8);//69h
  lcdRegDataWrite(RA8876_DLVSR0,y0);//6ah
  lcdRegDataWrite(RA8876_DLVSR1,y0>>8);//6bh
  lcdRegDataWrite(RA8876_DLHER0,x1);//6ch
  lcdRegDataWrite(RA8876_DLHER1,x1>>8);//6dh
  lcdRegDataWrite(RA8876_DLVER0,y1);//6eh
  lcdRegDataWrite(RA8876_DLVER1,y1>>8);//6fh  
  lcdRegDataWrite(RA8876_DTPH0,x2);//70h
  lcdRegDataWrite(RA8876_DTPH1,x2>>8);//71h
  lcdRegDataWrite(RA8876_DTPV0,y2);//72h
  lcdRegDataWrite(RA8876_DTPV1,y2>>8);//73h  
  lcdRegDataWrite(RA8876_DCR0,RA8876_DRAW_TRIANGLE);//67h,0x82
  check2dBusy();
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::drawTriangleFill(ru16 x0,ru16 y0,ru16 x1,ru16 y1,ru16 x2,ru16 y2,ru16 color)
{
  foreGroundColor16bpp(color);
  lcdRegDataWrite(RA8876_DLHSR0,x0);//68h
  lcdRegDataWrite(RA8876_DLHSR1,x0>>8);//69h
  lcdRegDataWrite(RA8876_DLVSR0,y0);//6ah
  lcdRegDataWrite(RA8876_DLVSR1,y0>>8);//6bh
  lcdRegDataWrite(RA8876_DLHER0,x1);//6ch
  lcdRegDataWrite(RA8876_DLHER1,x1>>8);//6dh
  lcdRegDataWrite(RA8876_DLVER0,y1);//6eh
  lcdRegDataWrite(RA8876_DLVER1,y1>>8);//6fh  
  lcdRegDataWrite(RA8876_DTPH0,x2);//70h
  lcdRegDataWrite(RA8876_DTPH1,x2>>8);//71h
  lcdRegDataWrite(RA8876_DTPV0,y2);//72h
  lcdRegDataWrite(RA8876_DTPV1,y2>>8);//73h  
  lcdRegDataWrite(RA8876_DCR0,RA8876_DRAW_TRIANGLE_FILL);//67h,0xa2
  check2dBusy();
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::drawCircle(ru16 x0,ru16 y0,ru16 r,ru16 color)
{
  foreGroundColor16bpp(color);
  lcdRegDataWrite(RA8876_DEHR0,x0);//7bh
  lcdRegDataWrite(RA8876_DEHR1,x0>>8);//7ch
  lcdRegDataWrite(RA8876_DEVR0,y0);//7dh
  lcdRegDataWrite(RA8876_DEVR1,y0>>8);//7eh
  lcdRegDataWrite(RA8876_ELL_A0,r);//77h    
  lcdRegDataWrite(RA8876_ELL_A1,r>>8);//78h 
  lcdRegDataWrite(RA8876_ELL_B0,r);//79h    
  lcdRegDataWrite(RA8876_ELL_B1,r>>8);//7ah
  lcdRegDataWrite(RA8876_DCR1,RA8876_DRAW_CIRCLE);//76h,0x80
  check2dBusy();
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::drawCircleFill(ru16 x0,ru16 y0,ru16 r,ru16 color)
{
  foreGroundColor16bpp(color);
  lcdRegDataWrite(RA8876_DEHR0,x0);//7bh
  lcdRegDataWrite(RA8876_DEHR1,x0>>8);//7ch
  lcdRegDataWrite(RA8876_DEVR0,y0);//7dh
  lcdRegDataWrite(RA8876_DEVR1,y0>>8);//7eh
  lcdRegDataWrite(RA8876_ELL_A0,r);//77h    
  lcdRegDataWrite(RA8876_ELL_A1,r>>8);//78h 
  lcdRegDataWrite(RA8876_ELL_B0,r);//79h    
  lcdRegDataWrite(RA8876_ELL_B1,r>>8);//7ah
  lcdRegDataWrite(RA8876_DCR1,RA8876_DRAW_CIRCLE_FILL);//76h,0xc0
  check2dBusy();
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::drawEllipse(ru16 x0,ru16 y0,ru16 xr,ru16 yr,ru16 color)
{
   foreGroundColor16bpp(color);
  lcdRegDataWrite(RA8876_DEHR0,x0);//7bh
  lcdRegDataWrite(RA8876_DEHR1,x0>>8);//7ch
  lcdRegDataWrite(RA8876_DEVR0,y0);//7dh
  lcdRegDataWrite(RA8876_DEVR1,y0>>8);//7eh
  lcdRegDataWrite(RA8876_ELL_A0,xr);//77h    
  lcdRegDataWrite(RA8876_ELL_A1,xr>>8);//78h 
  lcdRegDataWrite(RA8876_ELL_B0,yr);//79h    
  lcdRegDataWrite(RA8876_ELL_B1,yr>>8);//7ah
  lcdRegDataWrite(RA8876_DCR1,RA8876_DRAW_ELLIPSE);//76h,0x80
  check2dBusy();
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::drawEllipseFill(ru16 x0,ru16 y0,ru16 xr,ru16 yr,ru16 color)
{
   foreGroundColor16bpp(color);
  lcdRegDataWrite(RA8876_DEHR0,x0);//7bh
  lcdRegDataWrite(RA8876_DEHR1,x0>>8);//7ch
  lcdRegDataWrite(RA8876_DEVR0,y0);//7dh
  lcdRegDataWrite(RA8876_DEVR1,y0>>8);//7eh
  lcdRegDataWrite(RA8876_ELL_A0,xr);//77h    
  lcdRegDataWrite(RA8876_ELL_A1,xr>>8);//78h 
  lcdRegDataWrite(RA8876_ELL_B0,yr);//79h    
  lcdRegDataWrite(RA8876_ELL_B1,yr>>8);//7ah
  lcdRegDataWrite(RA8876_DCR1,RA8876_DRAW_ELLIPSE_FILL);//76h,0xc0
  check2dBusy();
}

/*BTE function*/
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::bteMemoryCopy(ru32 s0_addr,ru16 s0_image_width,ru16 s0_x,ru16 s0_y,ru32 des_addr,ru16 des_image_width, 
                                ru16 des_x,ru16 des_y,ru16 copy_width,ru16 copy_height)
{
  bte_Source0_MemoryStartAddr(s0_addr);
  bte_Source0_ImageWidth(s0_image_width);
  bte_Source0_WindowStartXY(s0_x,s0_y);
  //bte_Source1_MemoryStartAddr(des_addr);
  //bte_Source1_ImageWidth(des_image_width);
  //bte_Source1_WindowStartXY(des_x,des_y);
  bte_DestinationMemoryStartAddr(des_addr);
  bte_DestinationImageWidth(des_image_width);
  bte_DestinationWindowStartXY(des_x,des_y);
  
  bte_WindowSize(copy_width,copy_height); 
  lcdRegDataWrite(RA8876_BTE_CTRL1,RA8876_BTE_ROP_CODE_12<<4|RA8876_BTE_MEMORY_COPY_WITH_ROP);//91h
  lcdRegDataWrite(RA8876_BTE_COLR,RA8876_S0_COLOR_DEPTH_16BPP<<5|RA8876_S1_COLOR_DEPTH_16BPP<<2|RA8876_DESTINATION_COLOR_DEPTH_16BPP);//92h
  lcdRegDataWrite(RA8876_BTE_CTRL0,RA8876_BTE_ENABLE<<4);//90h
  check2dBusy();
} 
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::bteMemoryCopyWithROP(ru32 s0_addr,ru16 s0_image_width,ru16 s0_x,ru16 s0_y,ru32 s1_addr,ru16 s1_image_width,ru16 s1_x,ru16 s1_y,
                                       ru32 des_addr,ru16 des_image_width, ru16 des_x,ru16 des_y,ru16 copy_width,ru16 copy_height,ru8 rop_code)
{
  bte_Source0_MemoryStartAddr(s0_addr);
  bte_Source0_ImageWidth(s0_image_width);
  bte_Source0_WindowStartXY(s0_x,s0_y);
  bte_Source1_MemoryStartAddr(s1_addr);
  bte_Source1_ImageWidth(s1_image_width);
  bte_Source1_WindowStartXY(s1_x,s1_y);
  bte_DestinationMemoryStartAddr(des_addr);
  bte_DestinationImageWidth(des_image_width);
  bte_DestinationWindowStartXY(des_x,des_y);
  bte_WindowSize(copy_width,copy_height);
  lcdRegDataWrite(RA8876_BTE_CTRL1,rop_code<<4|RA8876_BTE_MEMORY_COPY_WITH_ROP);//91h
  lcdRegDataWrite(RA8876_BTE_COLR,RA8876_S0_COLOR_DEPTH_16BPP<<5|RA8876_S1_COLOR_DEPTH_16BPP<<2|RA8876_DESTINATION_COLOR_DEPTH_16BPP);//92h
  lcdRegDataWrite(RA8876_BTE_CTRL0,RA8876_BTE_ENABLE<<4);//90h
  check2dBusy();
} 
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::bteMemoryCopyWithChromaKey(ru32 s0_addr,ru16 s0_image_width,ru16 s0_x,ru16 s0_y,
ru32 des_addr,ru16 des_image_width, ru16 des_x,ru16 des_y,ru16 copy_width,ru16 copy_height, ru16 chromakey_color)
{
  bte_Source0_MemoryStartAddr(s0_addr);
  bte_Source0_ImageWidth(s0_image_width);
  bte_Source0_WindowStartXY(s0_x,s0_y);
  bte_DestinationMemoryStartAddr(des_addr);
  bte_DestinationImageWidth(des_image_width);
  bte_DestinationWindowStartXY(des_x,des_y);
  bte_WindowSize(copy_width,copy_height);
  backGroundColor16bpp(chromakey_color);
  lcdRegDataWrite(RA8876_BTE_CTRL1,RA8876_BTE_MEMORY_COPY_WITH_CHROMA);//91h
  lcdRegDataWrite(RA8876_BTE_COLR,RA8876_S0_COLOR_DEPTH_16BPP<<5|RA8876_S1_COLOR_DEPTH_16BPP<<2|RA8876_DESTINATION_COLOR_DEPTH_16BPP);//92h
  lcdRegDataWrite(RA8876_BTE_CTRL0,RA8876_BTE_ENABLE<<4);//90h
  check2dBusy();
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::bteMpuWriteWithROP(ru32 s1_addr,ru16 s1_image_width,ru16 s1_x,ru16 s1_y,ru32 des_addr,ru16 des_image_width,
ru16 des_x,ru16 des_y,ru16 width,ru16 height,ru8 rop_code,const unsigned char *data)
{
   ru16 i,j;
  bte_Source1_MemoryStartAddr(s1_addr);
  bte_Source1_ImageWidth(s1_image_width);
  bte_Source1_WindowStartXY(s1_x,s1_y);
  bte_DestinationMemoryStartAddr(des_addr);
  bte_DestinationImageWidth(des_image_width);
  bte_DestinationWindowStartXY(des_x,des_y);
  bte_WindowSize(width,height);
  lcdRegDataWrite(RA8876_BTE_CTRL1,rop_code<<4|RA8876_BTE_MPU_WRITE_WITH_ROP);//91h
  lcdRegDataWrite(RA8876_BTE_COLR,RA8876_S0_COLOR_DEPTH_16BPP<<5|RA8876_S1_COLOR_DEPTH_16BPP<<2|RA8876_DESTINATION_COLOR_DEPTH_16BPP);//92h
  lcdRegDataWrite(RA8876_BTE_CTRL0,RA8876_BTE_ENABLE<<4);//90h
  ramAccessPrepare();
  
  for(i=0;i< height;i++)
  {	
   for(j=0;j< (width*2);j++)
   {
    checkWriteFifoNotFull();
    lcdDataWrite(*data);
    data++;
    }
   }
  checkWriteFifoEmpty();
}

//**************************************************************//
//**************************************************************//
void Ra8876_Lite::bteMpuWriteWithROP(ru32 s1_addr,ru16 s1_image_width,ru16 s1_x,ru16 s1_y,ru32 des_addr,ru16 des_image_width,
ru16 des_x,ru16 des_y,ru16 width,ru16 height,ru8 rop_code,const unsigned short *data)
{
   ru16 i,j;
  bte_Source1_MemoryStartAddr(s1_addr);
  bte_Source1_ImageWidth(s1_image_width);
  bte_Source1_WindowStartXY(s1_x,s1_y);
  bte_DestinationMemoryStartAddr(des_addr);
  bte_DestinationImageWidth(des_image_width);
  bte_DestinationWindowStartXY(des_x,des_y);
  bte_WindowSize(width,height);
  lcdRegDataWrite(RA8876_BTE_CTRL1,rop_code<<4|RA8876_BTE_MPU_WRITE_WITH_ROP);//91h
  lcdRegDataWrite(RA8876_BTE_COLR,RA8876_S0_COLOR_DEPTH_16BPP<<5|RA8876_S1_COLOR_DEPTH_16BPP<<2|RA8876_DESTINATION_COLOR_DEPTH_16BPP);//92h
  lcdRegDataWrite(RA8876_BTE_CTRL0,RA8876_BTE_ENABLE<<4);//90h
  ramAccessPrepare();
  
 for(j=0;j<height;j++)
 {
  for(i=0;i<width;i++)
  {
   checkWriteFifoNotFull();//if high speed mcu and without Xnwait check
   lcdDataWrite16bbp(*data);
   data++;
   //checkWriteFifoEmpty();//if high speed mcu and without Xnwait check
  }
 } 
  checkWriteFifoEmpty();
}
//**************************************************************//
//write data after setting
//**************************************************************//
void Ra8876_Lite::bteMpuWriteWithROP(ru32 s1_addr,ru16 s1_image_width,ru16 s1_x,ru16 s1_y,ru32 des_addr,ru16 des_image_width,
ru16 des_x,ru16 des_y,ru16 width,ru16 height,ru8 rop_code)
{
   ru16 i,j;
  bte_Source1_MemoryStartAddr(s1_addr);
  bte_Source1_ImageWidth(s1_image_width);
  bte_Source1_WindowStartXY(s1_x,s1_y);
  bte_DestinationMemoryStartAddr(des_addr);
  bte_DestinationImageWidth(des_image_width);
  bte_DestinationWindowStartXY(des_x,des_y);
  bte_WindowSize(width,height);
  lcdRegDataWrite(RA8876_BTE_CTRL1,rop_code<<4|RA8876_BTE_MPU_WRITE_WITH_ROP);//91h
  lcdRegDataWrite(RA8876_BTE_COLR,RA8876_S0_COLOR_DEPTH_16BPP<<5|RA8876_S1_COLOR_DEPTH_16BPP<<2|RA8876_DESTINATION_COLOR_DEPTH_16BPP);//92h
  lcdRegDataWrite(RA8876_BTE_CTRL0,RA8876_BTE_ENABLE<<4);//90h
  ramAccessPrepare();
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::bteMpuWriteWithChromaKey(ru32 des_addr,ru16 des_image_width, ru16 des_x,ru16 des_y,ru16 width,ru16 height,ru16 chromakey_color,const unsigned char *data)
{
  ru16 i,j;
  bte_DestinationMemoryStartAddr(des_addr);
  bte_DestinationImageWidth(des_image_width);
  bte_DestinationWindowStartXY(des_x,des_y);
  bte_WindowSize(width,height);
  backGroundColor16bpp(chromakey_color);
  lcdRegDataWrite(RA8876_BTE_CTRL1,RA8876_BTE_MPU_WRITE_WITH_CHROMA);//91h
  lcdRegDataWrite(RA8876_BTE_COLR,RA8876_S0_COLOR_DEPTH_16BPP<<5|RA8876_S1_COLOR_DEPTH_16BPP<<2|RA8876_DESTINATION_COLOR_DEPTH_16BPP);//92h
  lcdRegDataWrite(RA8876_BTE_CTRL0,RA8876_BTE_ENABLE<<4);//90h
  ramAccessPrepare();
  
  for(i=0;i< height;i++)
  {	
   for(j=0;j< (width*2);j++)
   {
    checkWriteFifoNotFull();
    lcdDataWrite(*data);
    data++;
    }
   }
  checkWriteFifoEmpty();
}

//**************************************************************//
//**************************************************************//
void Ra8876_Lite::bteMpuWriteWithChromaKey(ru32 des_addr,ru16 des_image_width, ru16 des_x,ru16 des_y,ru16 width,ru16 height,ru16 chromakey_color,const unsigned short *data)
{
  ru16 i,j;
  bte_DestinationMemoryStartAddr(des_addr);
  bte_DestinationImageWidth(des_image_width);
  bte_DestinationWindowStartXY(des_x,des_y);
  bte_WindowSize(width,height);
  backGroundColor16bpp(chromakey_color);
  lcdRegDataWrite(RA8876_BTE_CTRL1,RA8876_BTE_MPU_WRITE_WITH_CHROMA);//91h
  lcdRegDataWrite(RA8876_BTE_COLR,RA8876_S0_COLOR_DEPTH_16BPP<<5|RA8876_S1_COLOR_DEPTH_16BPP<<2|RA8876_DESTINATION_COLOR_DEPTH_16BPP);//92h
  lcdRegDataWrite(RA8876_BTE_CTRL0,RA8876_BTE_ENABLE<<4);//90h
  ramAccessPrepare();
  
 for(j=0;j<height;j++)
 {
  for(i=0;i<width;i++)
  {
   checkWriteFifoNotFull();//if high speed mcu and without Xnwait check
   lcdDataWrite16bbp(*data);
   data++;
   //checkWriteFifoEmpty();//if high speed mcu and without Xnwait check
  }
 } 
  checkWriteFifoEmpty();
}
//**************************************************************//
//write data after setting
//**************************************************************//
void Ra8876_Lite::bteMpuWriteWithChromaKey(ru32 des_addr,ru16 des_image_width, ru16 des_x,ru16 des_y,ru16 width,ru16 height,ru16 chromakey_color)
{
  ru16 i,j;
  bte_DestinationMemoryStartAddr(des_addr);
  bte_DestinationImageWidth(des_image_width);
  bte_DestinationWindowStartXY(des_x,des_y);
  bte_WindowSize(width,height);
  backGroundColor16bpp(chromakey_color);
  lcdRegDataWrite(RA8876_BTE_CTRL1,RA8876_BTE_MPU_WRITE_WITH_CHROMA);//91h
  lcdRegDataWrite(RA8876_BTE_COLR,RA8876_S0_COLOR_DEPTH_16BPP<<5|RA8876_S1_COLOR_DEPTH_16BPP<<2|RA8876_DESTINATION_COLOR_DEPTH_16BPP);//92h
  lcdRegDataWrite(RA8876_BTE_CTRL0,RA8876_BTE_ENABLE<<4);//90h
  ramAccessPrepare();
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::bteMpuWriteColorExpansion(ru32 des_addr,ru16 des_image_width, ru16 des_x,ru16 des_y,ru16 width,ru16 height,ru16 foreground_color,ru16 background_color,const unsigned char *data)
{
  ru16 i,j;
  bte_DestinationMemoryStartAddr(des_addr);
  bte_DestinationImageWidth(des_image_width);
  bte_DestinationWindowStartXY(des_x,des_y);
  bte_WindowSize(width,height);
  foreGroundColor16bpp(foreground_color);
  backGroundColor16bpp(background_color);
  lcdRegDataWrite(RA8876_BTE_CTRL1,RA8876_BTE_ROP_BUS_WIDTH8<<4|RA8876_BTE_MPU_WRITE_COLOR_EXPANSION);//91h
  lcdRegDataWrite(RA8876_BTE_COLR,RA8876_S0_COLOR_DEPTH_16BPP<<5|RA8876_S1_COLOR_DEPTH_16BPP<<2|RA8876_DESTINATION_COLOR_DEPTH_16BPP);//92h
  lcdRegDataWrite(RA8876_BTE_CTRL0,RA8876_BTE_ENABLE<<4);//90h
  ramAccessPrepare();
  
  for(i=0;i< height;i++)
  {	
   for(j=0;j< (width/8);j++)
   {
    checkWriteFifoNotFull();
    lcdDataWrite(*data);
    data++;
    }
   }
  checkWriteFifoEmpty();
  check2dBusy();
}
//**************************************************************//
//write data after setting
//**************************************************************//
void Ra8876_Lite::bteMpuWriteColorExpansion(ru32 des_addr,ru16 des_image_width, ru16 des_x,ru16 des_y,ru16 width,ru16 height,ru16 foreground_color,ru16 background_color)
{
  ru16 i,j;
  bte_DestinationMemoryStartAddr(des_addr);
  bte_DestinationImageWidth(des_image_width);
  bte_DestinationWindowStartXY(des_x,des_y);
  bte_WindowSize(width,height); 
  foreGroundColor16bpp(foreground_color);
  backGroundColor16bpp(background_color);
  lcdRegDataWrite(RA8876_BTE_CTRL1,RA8876_BTE_ROP_BUS_WIDTH8<<4|RA8876_BTE_MPU_WRITE_COLOR_EXPANSION);//91h
  lcdRegDataWrite(RA8876_BTE_COLR,RA8876_S0_COLOR_DEPTH_16BPP<<5|RA8876_S1_COLOR_DEPTH_16BPP<<2|RA8876_DESTINATION_COLOR_DEPTH_16BPP);//92h
  lcdRegDataWrite(RA8876_BTE_CTRL0,RA8876_BTE_ENABLE<<4);//90h
  ramAccessPrepare();
}
//**************************************************************//
/*background_color do not set the same as foreground_color*/
//**************************************************************//
void Ra8876_Lite::bteMpuWriteColorExpansionWithChromaKey(ru32 des_addr,ru16 des_image_width, ru16 des_x,ru16 des_y,ru16 width,ru16 height,ru16 foreground_color,ru16 background_color, const unsigned char *data)
{
  ru16 i,j;
  bte_DestinationMemoryStartAddr(des_addr);
  bte_DestinationImageWidth(des_image_width);
  bte_DestinationWindowStartXY(des_x,des_y);
  bte_WindowSize(width,height);
  foreGroundColor16bpp(foreground_color);
  backGroundColor16bpp(background_color);
  lcdRegDataWrite(RA8876_BTE_CTRL1,RA8876_BTE_ROP_BUS_WIDTH8<<4|RA8876_BTE_MPU_WRITE_COLOR_EXPANSION_WITH_CHROMA);//91h
  lcdRegDataWrite(RA8876_BTE_COLR,RA8876_S0_COLOR_DEPTH_16BPP<<5|RA8876_S1_COLOR_DEPTH_16BPP<<2|RA8876_DESTINATION_COLOR_DEPTH_16BPP);//92h
  lcdRegDataWrite(RA8876_BTE_CTRL0,RA8876_BTE_ENABLE<<4);//90h
  ramAccessPrepare();
  
  for(i=0;i< height;i++)
  {	
   for(j=0;j< (width/8);j++)
   {
    checkWriteFifoNotFull();
    lcdDataWrite(*data);
    data++;
    }
   }
  checkWriteFifoEmpty();
  check2dBusy();
}
//**************************************************************//
/*background_color do not set the same as foreground_color*/
//write data after setting
//**************************************************************//
void Ra8876_Lite::bteMpuWriteColorExpansionWithChromaKey(ru32 des_addr,ru16 des_image_width, ru16 des_x,ru16 des_y,ru16 width,ru16 height,ru16 foreground_color,ru16 background_color)
{
  ru16 i,j;
  bte_DestinationMemoryStartAddr(des_addr);
  bte_DestinationImageWidth(des_image_width);
  bte_DestinationWindowStartXY(des_x,des_y);
  bte_WindowSize(width,height);
  foreGroundColor16bpp(foreground_color);
  backGroundColor16bpp(background_color);
  lcdRegDataWrite(RA8876_BTE_CTRL1,RA8876_BTE_ROP_BUS_WIDTH8<<4|RA8876_BTE_MPU_WRITE_COLOR_EXPANSION_WITH_CHROMA);//91h
  lcdRegDataWrite(RA8876_BTE_COLR,RA8876_S0_COLOR_DEPTH_16BPP<<5|RA8876_S1_COLOR_DEPTH_16BPP<<2|RA8876_DESTINATION_COLOR_DEPTH_16BPP);//92h
  lcdRegDataWrite(RA8876_BTE_CTRL0,RA8876_BTE_ENABLE<<4);//90h
  ramAccessPrepare();
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite:: btePatternFill(ru8 p8x8or16x16, ru32 s0_addr,ru16 s0_image_width,ru16 s0_x,ru16 s0_y,
                                 ru32 des_addr,ru16 des_image_width, ru16 des_x,ru16 des_y,ru16 width,ru16 height)
{ 
  bte_Source0_MemoryStartAddr(s0_addr);
  bte_Source0_ImageWidth(s0_image_width);
  bte_Source0_WindowStartXY(s0_x,s0_y);
  bte_DestinationMemoryStartAddr(des_addr);
  bte_DestinationImageWidth(des_image_width);
  bte_DestinationWindowStartXY(des_x,des_y);
  bte_WindowSize(width,height); 
  lcdRegDataWrite(RA8876_BTE_CTRL1,RA8876_BTE_ROP_CODE_12<<4|RA8876_BTE_PATTERN_FILL_WITH_ROP);//91h
  lcdRegDataWrite(RA8876_BTE_COLR,RA8876_S0_COLOR_DEPTH_16BPP<<5|RA8876_S1_COLOR_DEPTH_16BPP<<2|RA8876_DESTINATION_COLOR_DEPTH_16BPP);//92h
  
  if(p8x8or16x16 == 0)
   lcdRegDataWrite(RA8876_BTE_CTRL0,RA8876_BTE_ENABLE<<4|RA8876_PATTERN_FORMAT8X8);//90h
  else
   lcdRegDataWrite(RA8876_BTE_CTRL0,RA8876_BTE_ENABLE<<4|RA8876_PATTERN_FORMAT16X16);//90h
   
  check2dBusy();
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite:: btePatternFillWithChromaKey(ru8 p8x8or16x16, ru32 s0_addr,ru16 s0_image_width,ru16 s0_x,ru16 s0_y,
                                 ru32 des_addr,ru16 des_image_width, ru16 des_x,ru16 des_y,ru16 width,ru16 height,ru16 chromakey_color)
{
  bte_Source0_MemoryStartAddr(s0_addr);
  bte_Source0_ImageWidth(s0_image_width);
  bte_Source0_WindowStartXY(s0_x,s0_y);
  bte_DestinationMemoryStartAddr(des_addr);
  bte_DestinationImageWidth(des_image_width);
  bte_DestinationWindowStartXY(des_x,des_y);
  bte_WindowSize(width,height);
  backGroundColor16bpp(chromakey_color); 
  lcdRegDataWrite(RA8876_BTE_CTRL1,RA8876_BTE_ROP_CODE_12<<4|RA8876_BTE_PATTERN_FILL_WITH_CHROMA);//91h
  lcdRegDataWrite(RA8876_BTE_COLR,RA8876_S0_COLOR_DEPTH_16BPP<<5|RA8876_S1_COLOR_DEPTH_16BPP<<2|RA8876_DESTINATION_COLOR_DEPTH_16BPP);//92h
  if(p8x8or16x16 == 0)
   lcdRegDataWrite(RA8876_BTE_CTRL0,RA8876_BTE_ENABLE<<4|RA8876_PATTERN_FORMAT8X8);//90h
  else
   lcdRegDataWrite(RA8876_BTE_CTRL0,RA8876_BTE_ENABLE<<4|RA8876_PATTERN_FORMAT16X16);//90h
  check2dBusy();
}

 /*DMA Function*/
 //**************************************************************//
 /*If used 32bit address serial flash through ra8876, must be set command to serial flash to enter 4bytes mode first.
 only needs set one times after power on */
 //**************************************************************//
 void Ra8876_Lite:: setSerialFlash4BytesMode(ru8 scs_select)
 {
  if(scs_select==0)
  {
  lcdRegDataWrite( RA8876_SPIMCR2, RA8876_SPIM_NSS_SELECT_0<<5|RA8876_SPIM_MODE0);//b9h
  lcdRegDataWrite( RA8876_SPIMCR2, RA8876_SPIM_NSS_SELECT_0<<5|RA8876_SPIM_NSS_ACTIVE<<4|RA8876_SPIM_MODE0);//b9h 
  lcdRegWrite( RA8876_SPIDR);//b8h
  delay(1);
  lcdDataWrite(0xB7);//
  delay(1);
  lcdRegDataWrite( RA8876_SPIMCR2, RA8876_SPIM_NSS_SELECT_0<<5|RA8876_SPIM_NSS_INACTIVE<<4|RA8876_SPIM_MODE0);//b9h 
  }
  if(scs_select==1)
  {
  lcdRegDataWrite( RA8876_SPIMCR2 ,RA8876_SPIM_NSS_SELECT_1<<5|RA8876_SPIM_MODE0);//b9h
  lcdRegDataWrite( RA8876_SPIMCR2, RA8876_SPIM_NSS_SELECT_1<<5|RA8876_SPIM_NSS_ACTIVE<<4|RA8876_SPIM_MODE0);//b9h
  lcdRegWrite( RA8876_SPIDR);//b8h
  delay(1);
  lcdDataWrite(0xB7);//
  delay(1);
  lcdRegDataWrite( RA8876_SPIMCR2, RA8876_SPIM_NSS_SELECT_1<<5|RA8876_SPIM_NSS_INACTIVE<<4|RA8876_SPIM_MODE0);//b9h 
  } 
 }
//**************************************************************//
/* scs = 0 : select scs0, scs = 1 : select scs1, */
//**************************************************************//
 void Ra8876_Lite:: dma_24bitAddressBlockMode(ru8 scs_select,ru8 clk_div,ru16 x0,ru16 y0,ru16 width,ru16 height,ru16 picture_width,ru32 addr)
 {
   if(scs_select==0)
    lcdRegDataWrite(RA8876_SFL_CTRL,RA8876_SERIAL_FLASH_SELECT0<<7|RA8876_SERIAL_FLASH_DMA_MODE<<6|RA8876_SERIAL_FLASH_ADDR_24BIT<<5|RA8876_FOLLOW_RA8876_MODE<<4|RA8876_SPI_FAST_READ_8DUMMY);//b7h
   if(scs_select==1)
    lcdRegDataWrite(RA8876_SFL_CTRL,RA8876_SERIAL_FLASH_SELECT1<<7|RA8876_SERIAL_FLASH_DMA_MODE<<6|RA8876_SERIAL_FLASH_ADDR_24BIT<<5|RA8876_FOLLOW_RA8876_MODE<<4|RA8876_SPI_FAST_READ_8DUMMY);//b7h
  
  lcdRegDataWrite(RA8876_SPI_DIVSOR,clk_div);//bbh  
  lcdRegDataWrite(RA8876_DMA_DX0,x0);//c0h
  lcdRegDataWrite(RA8876_DMA_DX1,x0>>8);//c1h
  lcdRegDataWrite(RA8876_DMA_DY0,y0);//c2h
  lcdRegDataWrite(RA8876_DMA_DY1,y0>>8);//c3h 
  lcdRegDataWrite(RA8876_DMAW_WTH0,width);//c6h
  lcdRegDataWrite(RA8876_DMAW_WTH1,width>>8);//c7h
  lcdRegDataWrite(RA8876_DMAW_HIGH0,height);//c8h
  lcdRegDataWrite(RA8876_DMAW_HIGH1,height>>8);//c9h 
  lcdRegDataWrite(RA8876_DMA_SWTH0,picture_width);//cah
  lcdRegDataWrite(RA8876_DMA_SWTH1,picture_width>>8);//cbh 
  lcdRegDataWrite(RA8876_DMA_SSTR0,addr);//bch
  lcdRegDataWrite(RA8876_DMA_SSTR1,addr>>8);//bdh
  lcdRegDataWrite(RA8876_DMA_SSTR2,addr>>16);//beh
  lcdRegDataWrite(RA8876_DMA_SSTR3,addr>>24);//bfh 
  
  lcdRegDataWrite(RA8876_DMA_CTRL,RA8876_DMA_START);//b6h 
  check2dBusy(); 
 }
 //**************************************************************//
/* scs = 0 : select scs0, scs = 1 : select scs1, */
//**************************************************************//
 void Ra8876_Lite:: dma_32bitAddressBlockMode(ru8 scs_select,ru8 clk_div,ru16 x0,ru16 y0,ru16 width,ru16 height,ru16 picture_width,ru32 addr)
 {
   if(scs_select==0)
    lcdRegDataWrite(RA8876_SFL_CTRL,RA8876_SERIAL_FLASH_SELECT0<<7|RA8876_SERIAL_FLASH_DMA_MODE<<6|RA8876_SERIAL_FLASH_ADDR_32BIT<<5|RA8876_FOLLOW_RA8876_MODE<<4|RA8876_SPI_FAST_READ_8DUMMY);//b7h
   if(scs_select==1)
    lcdRegDataWrite(RA8876_SFL_CTRL,RA8876_SERIAL_FLASH_SELECT1<<7|RA8876_SERIAL_FLASH_DMA_MODE<<6|RA8876_SERIAL_FLASH_ADDR_32BIT<<5|RA8876_FOLLOW_RA8876_MODE<<4|RA8876_SPI_FAST_READ_8DUMMY);//b7h
  
  lcdRegDataWrite(RA8876_SPI_DIVSOR,clk_div);//bbh 
  
  lcdRegDataWrite(RA8876_DMA_DX0,x0);//c0h
  lcdRegDataWrite(RA8876_DMA_DX1,x0>>8);//c1h
  lcdRegDataWrite(RA8876_DMA_DY0,y0);//c2h
  lcdRegDataWrite(RA8876_DMA_DY1,y0>>8);//c3h 
  lcdRegDataWrite(RA8876_DMAW_WTH0,width);//c6h
  lcdRegDataWrite(RA8876_DMAW_WTH1,width>>8);//c7h
  lcdRegDataWrite(RA8876_DMAW_HIGH0,height);//c8h
  lcdRegDataWrite(RA8876_DMAW_HIGH1,height>>8);//c9h 
  lcdRegDataWrite(RA8876_DMA_SWTH0,picture_width);//cah
  lcdRegDataWrite(RA8876_DMA_SWTH1,picture_width>>8);//cbh 
  lcdRegDataWrite(RA8876_DMA_SSTR0,addr);//bch
  lcdRegDataWrite(RA8876_DMA_SSTR1,addr>>8);//bdh
  lcdRegDataWrite(RA8876_DMA_SSTR2,addr>>16);//beh
  lcdRegDataWrite(RA8876_DMA_SSTR3,addr>>24);//bfh  
  
  lcdRegDataWrite(RA8876_DMA_CTRL,RA8876_DMA_START);//b6h 
  check2dBusy(); 
 }
 
 
 
 
 /*screen Rotate 270 application funcion*/
 //**************************************************************//
//**************************************************************//
void Ra8876_Lite::activeWindowXY_Rotate(ru16 x0,ru16 y0)	
{
  lcdRegDataWrite(RA8876_AWUL_X0,y0);//56h
  lcdRegDataWrite(RA8876_AWUL_X1,y0>>8);//57h 
  lcdRegDataWrite(RA8876_AWUL_Y0,x0);//58h
  lcdRegDataWrite(RA8876_AWUL_Y1,x0>>8);//59h 
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::activeWindowWH_Rotate(ru16 width,ru16 height)	
{
  lcdRegDataWrite(RA8876_AW_WTH0,height);//5ah
  lcdRegDataWrite(RA8876_AW_WTH1,height>>8);//5bh
  lcdRegDataWrite(RA8876_AW_HT0,width);//5ch
  lcdRegDataWrite(RA8876_AW_HT1,width>>8);//5dh  
}

void Ra8876_Lite:: setPixelCursor_Rotate(ru16 x,ru16 y)
{
  lcdRegDataWrite(RA8876_CURH0,y); //5fh
  lcdRegDataWrite(RA8876_CURH1,y>>8);//60h
  lcdRegDataWrite(RA8876_CURV0,x);//61h
  lcdRegDataWrite(RA8876_CURV1,x>>8);//62h
}

//**************************************************************//
/*support serial IF to write 16bpp pixel*/
//**************************************************************//
void Ra8876_Lite:: putPixel_16bpp_Rotate(ru16 x,ru16 y,ru16 color)
{
 setPixelCursor_Rotate(x,y);
 ramAccessPrepare();
 //checkWriteFifoNotFull();//if high speed mcu and without Xnwait check
 //lcdDataWrite(color);
 //lcdDataWrite(color>>8);
 lcdDataWrite16bbp(color);
 //checkWriteFifoEmpty();//if high speed mcu and without Xnwait check
}
//**************************************************************//
/*support ra8876 serial IF to write 16bpp(RGB565) picture data for user operation */
//**************************************************************//
void Ra8876_Lite:: putPicture_16bpp_Rotate(ru16 x,ru16 y,ru16 width, ru16 height)
{ru16 i,j;
 activeWindowXY_Rotate(x,y);
 activeWindowWH_Rotate(width,height);
 setPixelCursor_Rotate(x,y);
 ramAccessPrepare();
}
//**************************************************************//
/*support serial IF to write 16bpp(RGB565) picture data byte format from data pointer*/
//**************************************************************//
void Ra8876_Lite:: putPicture_16bpp_Rotate(ru16 x,ru16 y,ru16 width, ru16 height, const unsigned char *data)
{ru16 i,j;

 activeWindowXY_Rotate(x,y);
 activeWindowWH_Rotate(width,height);
 setPixelCursor_Rotate(x,y);
 ramAccessPrepare();
 for(j=0;j<height;j++)
 {
  for(i=0;i<width;i++)
  {
   //checkWriteFifoNotFull();//if high speed mcu and without Xnwait check
   lcdDataWrite(*data);
   data++;
   //checkWriteFifoNotFull();//if high speed mcu and without Xnwait check
   lcdDataWrite(*data);
   data++;
  }
 } 
 checkWriteFifoEmpty();//if high speed mcu and without Xnwait check
 activeWindowXY(0,0);
 activeWindowWH(SCREEN_WIDTH,SCREEN_HEIGHT);
}
//**************************************************************//
/*support serial IF to write 16bpp(RGB565) picture data word format from data pointer*/
//**************************************************************//
void Ra8876_Lite:: putPicture_16bpp_Rotate(ru16 x,ru16 y,ru16 width, ru16 height, const unsigned short *data)
{ru16 i,j;
 activeWindowXY_Rotate(x,y);
 activeWindowWH_Rotate(width,height);
 setPixelCursor_Rotate(x,y);
 ramAccessPrepare();
 for(j=0;j<height;j++)
 {
  for(i=0;i<width;i++)
  {
   //checkWriteFifoNotFull();//if high speed mcu and without Xnwait check
   lcdDataWrite16bbp(*data);
   data++;
   //checkWriteFifoEmpty();//if high speed mcu and without Xnwait check
  }
 } 
 checkWriteFifoEmpty();//if high speed mcu and without Xnwait check
 activeWindowXY(0,0);
 activeWindowWH(SCREEN_WIDTH,SCREEN_HEIGHT);
}


//**************************************************************//
//**************************************************************//
void Ra8876_Lite::drawLine_Rotate(ru16 x0, ru16 y0, ru16 x1, ru16 y1, ru16 color)
{
  foreGroundColor16bpp(color);
  lcdRegDataWrite(RA8876_DLHSR0,y0);//68h
  lcdRegDataWrite(RA8876_DLHSR1,y0>>8);//69h
  lcdRegDataWrite(RA8876_DLVSR0,x0);//6ah
  lcdRegDataWrite(RA8876_DLVSR1,x0>>8);//6bh
  lcdRegDataWrite(RA8876_DLHER0,y1);//6ch
  lcdRegDataWrite(RA8876_DLHER1,y1>>8);//6dh
  lcdRegDataWrite(RA8876_DLVER0,x1);//6eh
  lcdRegDataWrite(RA8876_DLVER1,x1>>8);//6fh        
  lcdRegDataWrite(RA8876_DCR0,RA8876_DRAW_LINE);//67h,0x80
  check2dBusy();
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::drawSquare_Rotate(ru16 x0, ru16 y0, ru16 x1, ru16 y1, ru16 color)
{
  foreGroundColor16bpp(color);
  lcdRegDataWrite(RA8876_DLHSR0,y0);//68h
  lcdRegDataWrite(RA8876_DLHSR1,y0>>8);//69h
  lcdRegDataWrite(RA8876_DLVSR0,x0);//6ah
  lcdRegDataWrite(RA8876_DLVSR1,x0>>8);//6bh
  lcdRegDataWrite(RA8876_DLHER0,y1);//6ch
  lcdRegDataWrite(RA8876_DLHER1,y1>>8);//6dh
  lcdRegDataWrite(RA8876_DLVER0,x1);//6eh
  lcdRegDataWrite(RA8876_DLVER1,x1>>8);//6fh        
  lcdRegDataWrite(RA8876_DCR1,RA8876_DRAW_SQUARE);//76h,0xa0
  check2dBusy();
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::drawSquareFill_Rotate(ru16 x0, ru16 y0, ru16 x1, ru16 y1, ru16 color)
{
  foreGroundColor16bpp(color);
  lcdRegDataWrite(RA8876_DLHSR0,y0);//68h
  lcdRegDataWrite(RA8876_DLHSR1,y0>>8);//69h
  lcdRegDataWrite(RA8876_DLVSR0,x0);//6ah
  lcdRegDataWrite(RA8876_DLVSR1,x0>>8);//6bh
  lcdRegDataWrite(RA8876_DLHER0,y1);//6ch
  lcdRegDataWrite(RA8876_DLHER1,y1>>8);//6dh
  lcdRegDataWrite(RA8876_DLVER0,x1);//6eh
  lcdRegDataWrite(RA8876_DLVER1,x1>>8);//6fh        
  lcdRegDataWrite(RA8876_DCR1,RA8876_DRAW_SQUARE_FILL);//76h,0xa0
  check2dBusy();
}

//**************************************************************//
//**************************************************************//
void Ra8876_Lite::drawCircleSquare_Rotate(ru16 x0, ru16 y0, ru16 x1, ru16 y1, ru16 xr, ru16 yr, ru16 color)
{
  foreGroundColor16bpp(color);
  lcdRegDataWrite(RA8876_DLHSR0,y0);//68h
  lcdRegDataWrite(RA8876_DLHSR1,y0>>8);//69h
  lcdRegDataWrite(RA8876_DLVSR0,x0);//6ah
  lcdRegDataWrite(RA8876_DLVSR1,x0>>8);//6bh
  lcdRegDataWrite(RA8876_DLHER0,y1);//6ch
  lcdRegDataWrite(RA8876_DLHER1,y1>>8);//6dh
  lcdRegDataWrite(RA8876_DLVER0,x1);//6eh
  lcdRegDataWrite(RA8876_DLVER1,x1>>8);//6fh     
  lcdRegDataWrite(RA8876_ELL_A0,yr);//77h    
  lcdRegDataWrite(RA8876_ELL_A1,yr>>8);//79h 
  lcdRegDataWrite(RA8876_ELL_B0,xr);//7ah    
  lcdRegDataWrite(RA8876_ELL_B1,xr>>8);//7bh
  lcdRegDataWrite(RA8876_DCR1,RA8876_DRAW_CIRCLE_SQUARE);//76h,0xb0
  check2dBusy();
}

//**************************************************************//
//**************************************************************//
void Ra8876_Lite::drawCircleSquareFill_Rotate(ru16 x0, ru16 y0, ru16 x1, ru16 y1, ru16 xr, ru16 yr, ru16 color)
{
  foreGroundColor16bpp(color);
  lcdRegDataWrite(RA8876_DLHSR0,y0);//68h
  lcdRegDataWrite(RA8876_DLHSR1,y0>>8);//69h
  lcdRegDataWrite(RA8876_DLVSR0,x0);//6ah
  lcdRegDataWrite(RA8876_DLVSR1,x0>>8);//6bh
  lcdRegDataWrite(RA8876_DLHER0,y1);//6ch
  lcdRegDataWrite(RA8876_DLHER1,y1>>8);//6dh
  lcdRegDataWrite(RA8876_DLVER0,x1);//6eh
  lcdRegDataWrite(RA8876_DLVER1,x1>>8);//6fh    
  lcdRegDataWrite(RA8876_ELL_A0,yr);//77h    
  lcdRegDataWrite(RA8876_ELL_A1,yr>>8);//78h 
  lcdRegDataWrite(RA8876_ELL_B0,xr);//79h    
  lcdRegDataWrite(RA8876_ELL_B1,xr>>8);//7ah
  lcdRegDataWrite(RA8876_DCR1,RA8876_DRAW_CIRCLE_SQUARE_FILL);//76h,0xf0
  check2dBusy();
}

//**************************************************************//
//**************************************************************//
void Ra8876_Lite::drawTriangle_Rotate(ru16 x0,ru16 y0,ru16 x1,ru16 y1,ru16 x2,ru16 y2,ru16 color)
{
  foreGroundColor16bpp(color);
  lcdRegDataWrite(RA8876_DLHSR0,y0);//68h
  lcdRegDataWrite(RA8876_DLHSR1,y0>>8);//69h
  lcdRegDataWrite(RA8876_DLVSR0,x0);//6ah
  lcdRegDataWrite(RA8876_DLVSR1,x0>>8);//6bh
  lcdRegDataWrite(RA8876_DLHER0,y1);//6ch
  lcdRegDataWrite(RA8876_DLHER1,y1>>8);//6dh
  lcdRegDataWrite(RA8876_DLVER0,x1);//6eh
  lcdRegDataWrite(RA8876_DLVER1,x1>>8);//6fh  
  lcdRegDataWrite(RA8876_DTPH0,y2);//70h
  lcdRegDataWrite(RA8876_DTPH1,y2>>8);//71h
  lcdRegDataWrite(RA8876_DTPV0,x2);//72h
  lcdRegDataWrite(RA8876_DTPV1,x2>>8);//73h  
  lcdRegDataWrite(RA8876_DCR0,RA8876_DRAW_TRIANGLE);//67h,0x82
  check2dBusy();
}

//**************************************************************//
//**************************************************************//
void Ra8876_Lite::drawTriangleFill_Rotate(ru16 x0,ru16 y0,ru16 x1,ru16 y1,ru16 x2,ru16 y2,ru16 color)
{
  foreGroundColor16bpp(color);
  lcdRegDataWrite(RA8876_DLHSR0,y0);//68h
  lcdRegDataWrite(RA8876_DLHSR1,y0>>8);//69h
  lcdRegDataWrite(RA8876_DLVSR0,x0);//6ah
  lcdRegDataWrite(RA8876_DLVSR1,x0>>8);//6bh
  lcdRegDataWrite(RA8876_DLHER0,y1);//6ch
  lcdRegDataWrite(RA8876_DLHER1,y1>>8);//6dh
  lcdRegDataWrite(RA8876_DLVER0,x1);//6eh
  lcdRegDataWrite(RA8876_DLVER1,x1>>8);//6fh  
  lcdRegDataWrite(RA8876_DTPH0,y2);//70h
  lcdRegDataWrite(RA8876_DTPH1,y2>>8);//71h
  lcdRegDataWrite(RA8876_DTPV0,x2);//72h
  lcdRegDataWrite(RA8876_DTPV1,x2>>8);//73h  
  lcdRegDataWrite(RA8876_DCR0,RA8876_DRAW_TRIANGLE_FILL);//67h,0xa2
  check2dBusy();
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::drawCircle_Rotate(ru16 x0,ru16 y0,ru16 r,ru16 color)
{
  foreGroundColor16bpp(color);
  lcdRegDataWrite(RA8876_DEHR0,y0);//7bh
  lcdRegDataWrite(RA8876_DEHR1,y0>>8);//7ch
  lcdRegDataWrite(RA8876_DEVR0,x0);//7dh
  lcdRegDataWrite(RA8876_DEVR1,x0>>8);//7eh
  lcdRegDataWrite(RA8876_ELL_A0,r);//77h    
  lcdRegDataWrite(RA8876_ELL_A1,r>>8);//78h 
  lcdRegDataWrite(RA8876_ELL_B0,r);//79h    
  lcdRegDataWrite(RA8876_ELL_B1,r>>8);//7ah
  lcdRegDataWrite(RA8876_DCR1,RA8876_DRAW_CIRCLE);//76h,0x80
  check2dBusy();
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::drawCircleFill_Rotate(ru16 x0,ru16 y0,ru16 r,ru16 color)
{
  foreGroundColor16bpp(color);
  lcdRegDataWrite(RA8876_DEHR0,y0);//7bh
  lcdRegDataWrite(RA8876_DEHR1,y0>>8);//7ch
  lcdRegDataWrite(RA8876_DEVR0,x0);//7dh
  lcdRegDataWrite(RA8876_DEVR1,x0>>8);//7eh
  lcdRegDataWrite(RA8876_ELL_A0,r);//77h    
  lcdRegDataWrite(RA8876_ELL_A1,r>>8);//78h 
  lcdRegDataWrite(RA8876_ELL_B0,r);//79h    
  lcdRegDataWrite(RA8876_ELL_B1,r>>8);//7ah
  lcdRegDataWrite(RA8876_DCR1,RA8876_DRAW_CIRCLE_FILL);//76h,0xc0
  check2dBusy();
}

//**************************************************************//
//**************************************************************//
void Ra8876_Lite::drawEllipse_Rotate(ru16 x0,ru16 y0,ru16 xr,ru16 yr,ru16 color)
{
   foreGroundColor16bpp(color);
  lcdRegDataWrite(RA8876_DEHR0,y0);//7bh
  lcdRegDataWrite(RA8876_DEHR1,y0>>8);//7ch
  lcdRegDataWrite(RA8876_DEVR0,x0);//7dh
  lcdRegDataWrite(RA8876_DEVR1,x0>>8);//7eh
  lcdRegDataWrite(RA8876_ELL_A0,yr);//77h    
  lcdRegDataWrite(RA8876_ELL_A1,yr>>8);//78h 
  lcdRegDataWrite(RA8876_ELL_B0,xr);//79h    
  lcdRegDataWrite(RA8876_ELL_B1,xr>>8);//7ah
  lcdRegDataWrite(RA8876_DCR1,RA8876_DRAW_ELLIPSE);//76h,0x80
  check2dBusy();
}
//**************************************************************//
//**************************************************************//
void Ra8876_Lite::drawEllipseFill_Rotate(ru16 x0,ru16 y0,ru16 xr,ru16 yr,ru16 color)
{
   foreGroundColor16bpp(color);
  lcdRegDataWrite(RA8876_DEHR0,y0);//7bh
  lcdRegDataWrite(RA8876_DEHR1,y0>>8);//7ch
  lcdRegDataWrite(RA8876_DEVR0,x0);//7dh
  lcdRegDataWrite(RA8876_DEVR1,x0>>8);//7eh
  lcdRegDataWrite(RA8876_ELL_A0,yr);//77h    
  lcdRegDataWrite(RA8876_ELL_A1,yr>>8);//78h 
  lcdRegDataWrite(RA8876_ELL_B0,xr);//79h    
  lcdRegDataWrite(RA8876_ELL_B1,xr>>8);//7ah
  lcdRegDataWrite(RA8876_DCR1,RA8876_DRAW_ELLIPSE_FILL);//76h,0xc0
  check2dBusy();
}


 
 void Ra8876_Lite::setTextParameter2_Rotate(ru8 align, ru8 chroma_key, ru8 width_enlarge, ru8 height_enlarge)
{
  lcdRegDataWrite(RA8876_CCR1,align<<7|chroma_key<<6|RA8876_TEXT_ROTATION<<4|width_enlarge<<2|height_enlarge);//cdh
}

 //**************************************************************//
 //**************************************************************//
void Ra8876_Lite:: setTextCursor_Rotate(ru16 x,ru16 y)
{
  lcdRegDataWrite(RA8876_F_CURX0,y); //63h
  lcdRegDataWrite(RA8876_F_CURX1,y>>8);//64h
  lcdRegDataWrite(RA8876_F_CURY0,x);//65h
  lcdRegDataWrite(RA8876_F_CURY1,x>>8);//66h
}

void Ra8876_Lite:: putString_Rotate(ru16 x0,ru16 y0, char *str)
{
  textMode(true);
  setTextCursor_Rotate(x0,y0);
  ramAccessPrepare();
  while(*str != '\0')
  {
  checkWriteFifoNotFull();  
  lcdDataWrite(*str);
  ++str; 
  } 
  check2dBusy();
  textMode(false);
}

/*put value,base on sprintf*/
//**************************************************************//
//vaule: -2147483648(-2^31) ~ 2147483647(2^31-1)
//len: 1~11 minimum output length
/*
  [flag] 
    n:靠右　
    -:靠左
    +:輸出正負號
    (space):當不輸出正負號時，就輸出空白
    0:在開頭處(左側) 補 0，而非補空白。 
 */
//**************************************************************//
void Ra8876_Lite:: putDec_Rotate(ru16 x0,ru16 y0,rs32 vaule,ru8 len,const char *flag)
{
  char char_buffer[12];
  switch(len)
  {
    case 1:
           if(flag=="n")
           {sprintf(char_buffer ,"%1d", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="-")
           {sprintf(char_buffer ,"%-1d", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="+")
           {sprintf(char_buffer ,"%+1d", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="0")
           {sprintf(char_buffer ,"%01d", vaule); putString_Rotate(x0,y0,char_buffer);}
           break;
    case 2:
           if(flag=="n")
           {sprintf(char_buffer ,"%2d", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="-")
           { sprintf(char_buffer ,"%-2d", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="+")
           { sprintf(char_buffer ,"%+2d", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="0")
           { sprintf(char_buffer ,"%02d", vaule); putString_Rotate(x0,y0,char_buffer);}
           break; 
    case 3: 
           if(flag=="n")
           {sprintf(char_buffer ,"%3d", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="-")
           {sprintf(char_buffer ,"%-3d", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="+")
           {sprintf(char_buffer ,"%+3d", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="0")
           {sprintf(char_buffer ,"%03d", vaule); putString_Rotate(x0,y0,char_buffer);}
           break;
    case 4: 
           if(flag=="n")
           {sprintf(char_buffer ,"%4d", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="-")
           { sprintf(char_buffer ,"%-4d", vaule);  putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="+")
           { sprintf(char_buffer ,"%+4d", vaule);  putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="0")
           { sprintf(char_buffer ,"%04d", vaule);  putString_Rotate(x0,y0,char_buffer);}
           break;
    case 5: 
           if(flag=="n")
           {sprintf(char_buffer ,"%5d", vaule); putString_Rotate(x0,y0,char_buffer);}    
           else if(flag=="-")
           {sprintf(char_buffer ,"%-5d", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="+")
           {sprintf(char_buffer ,"%+5d", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="0")
           {sprintf(char_buffer ,"%05d", vaule); putString_Rotate(x0,y0,char_buffer);}
           break;
    case 6: 
           if(flag=="n")
           {sprintf(char_buffer ,"%6d", vaule); putString_Rotate(x0,y0,char_buffer);}    
           else if(flag=="-")
           {sprintf(char_buffer ,"%-6d", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="+")
           {sprintf(char_buffer ,"%+6d", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="0")
           {sprintf(char_buffer ,"%06d", vaule); putString_Rotate(x0,y0,char_buffer);}
           break; 
    case 7: 
           if(flag=="n")
           {sprintf(char_buffer ,"%7d", vaule); putString_Rotate(x0,y0,char_buffer);} 
           else if(flag=="-")
           {sprintf(char_buffer ,"%-7d", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="+")
           {sprintf(char_buffer ,"%+7d", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="0")
           {sprintf(char_buffer ,"%07d", vaule); putString_Rotate(x0,y0,char_buffer);}
           break;
    case 8: 
           if(flag=="n")
           {sprintf(char_buffer ,"%8d", vaule); putString_Rotate(x0,y0,char_buffer);}     
           else if(flag=="-")
           {sprintf(char_buffer ,"%-8d", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="+")
           {sprintf(char_buffer ,"%+8d", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="0")
           {sprintf(char_buffer ,"%08d", vaule); putString_Rotate(x0,y0,char_buffer);}
           break;
    case 9:
           if(flag=="n")
           {sprintf(char_buffer ,"%9d", vaule); putString_Rotate(x0,y0,char_buffer);}  
           else if(flag=="-")
           {sprintf(char_buffer ,"%-9d", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="+")
           {sprintf(char_buffer ,"%+9d", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="0")
           {sprintf(char_buffer ,"%09d", vaule); putString_Rotate(x0,y0,char_buffer);}
           break;
    case 10:
           if(flag=="n")
           {sprintf(char_buffer ,"%10d", vaule); putString_Rotate(x0,y0,char_buffer);}  
           else if(flag=="-")
           {sprintf(char_buffer ,"%-10d", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="+")
           {sprintf(char_buffer ,"%+10d", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="0")
           {sprintf(char_buffer ,"%010d", vaule); putString_Rotate(x0,y0,char_buffer);}
           break;
    case 11: 
           if(flag=="n")
           {sprintf(char_buffer ,"%11d", vaule); putString_Rotate(x0,y0,char_buffer);} 
           else if(flag=="-")
           {sprintf(char_buffer ,"%-11d", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="+")
           {sprintf(char_buffer ,"%+11d", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="0")
           {sprintf(char_buffer ,"%011d", vaule); putString_Rotate(x0,y0,char_buffer);}
           break;       
    default:   
           break;    
  }
}

//**************************************************************//
//vaule: (3.4E-38) ~ (3.4E38)
//len: 1~11 minimum output length
//precision: right side of point numbers 1~4 
/*
  [flag] 
    n:靠右　
    -:靠左
    +:輸出正負號
    (space):當不輸出正負號時，就輸出空白
    #: 強制輸出小數點
    0:在開頭處(左側) 補 0，而非補空白。
    
 */
 //arduino Floats have only 6-7 decimal digits of precision. That means the total number of digits, not the number to the right of the decimal point. 
 //Unlike other platforms, where you can get more precision by using a double (e.g. up to 15 digits), on the Arduino, double is the same size as float.
//**************************************************************//
void Ra8876_Lite:: putFloat_Rotate(ru16 x0,ru16 y0,double vaule,ru8 len,ru8 precision, const char *flag)
{
  char char_buffer[20];
  switch(len)
  {
    case 1:
           if(flag=="n")
           {
             if(precision==1)
             sprintf(char_buffer ,"%1.1f", vaule); putString_Rotate(x0,y0,char_buffer);
             if(precision==2)
             sprintf(char_buffer ,"%1.2f", vaule); putString_Rotate(x0,y0,char_buffer);
             if(precision==3)
             sprintf(char_buffer ,"%1.3f", vaule); putString_Rotate(x0,y0,char_buffer);
             if(precision==4)
             sprintf(char_buffer ,"%1.4f", vaule); putString_Rotate(x0,y0,char_buffer);
           }
           else if(flag=="-")
           {
            if(precision==1)
            sprintf(char_buffer ,"%-1.1f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%-1.2f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%-1.3f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%-1.4f", vaule); putString_Rotate(x0,y0,char_buffer);
           }
           else if(flag=="+")
           {
            if(precision==1)
            sprintf(char_buffer ,"%+1.1f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%+1.2f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%+1.3f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%+1.4f", vaule); putString_Rotate(x0,y0,char_buffer);
           }
           else if(flag=="0")
           {
            if(precision==1)
            sprintf(char_buffer ,"%01.1f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%01.2f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%01.3f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%01.4f", vaule); putString_Rotate(x0,y0,char_buffer);
           }
           break;
    case 2:
           if(flag=="n")
           {
           if(precision==1)  
           sprintf(char_buffer ,"%2.1f", vaule); putString_Rotate(x0,y0,char_buffer);
           if(precision==2)  
           sprintf(char_buffer ,"%2.2f", vaule); putString_Rotate(x0,y0,char_buffer);
           if(precision==3)  
           sprintf(char_buffer ,"%2.3f", vaule); putString_Rotate(x0,y0,char_buffer);
           if(precision==4)  
           sprintf(char_buffer ,"%2.4f", vaule); putString_Rotate(x0,y0,char_buffer);
           }
           else if(flag=="-")
           { 
           if(precision==1) 
           sprintf(char_buffer ,"%-2.1f", vaule); putString_Rotate(x0,y0,char_buffer);
           if(precision==2) 
           sprintf(char_buffer ,"%-2.2f", vaule); putString_Rotate(x0,y0,char_buffer);
           if(precision==3) 
           sprintf(char_buffer ,"%-2.3f", vaule); putString_Rotate(x0,y0,char_buffer);
           if(precision==4) 
           sprintf(char_buffer ,"%-2.4f", vaule); putString_Rotate(x0,y0,char_buffer);
           }
           else if(flag=="+")
           { 
           if(precision==1)
           sprintf(char_buffer ,"%+2.1f", vaule); putString_Rotate(x0,y0,char_buffer);
           if(precision==2)
           sprintf(char_buffer ,"%+2.2f", vaule); putString_Rotate(x0,y0,char_buffer);
           if(precision==3)
           sprintf(char_buffer ,"%+2.3f", vaule); putString_Rotate(x0,y0,char_buffer);
           if(precision==4)
           sprintf(char_buffer ,"%+2.4f", vaule); putString_Rotate(x0,y0,char_buffer);
           }
           else if(flag=="0")
           { 
           if(precision==1)
           sprintf(char_buffer ,"%02.1f", vaule); putString_Rotate(x0,y0,char_buffer);
           if(precision==2)
           sprintf(char_buffer ,"%02.2f", vaule); putString_Rotate(x0,y0,char_buffer);
           if(precision==3)
           sprintf(char_buffer ,"%02.3f", vaule); putString_Rotate(x0,y0,char_buffer);
           if(precision==4)
           sprintf(char_buffer ,"%02.4f", vaule); putString_Rotate(x0,y0,char_buffer);
           }
           break; 
    case 3: 
           if(flag=="n")
           {
            if(precision==1)
            sprintf(char_buffer ,"%3.1f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%3.2f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%3.3f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%3.4f", vaule); putString_Rotate(x0,y0,char_buffer);
           }
           else if(flag=="-")
           {
           if(precision==1)
           sprintf(char_buffer ,"%-3.1f", vaule); putString_Rotate(x0,y0,char_buffer);
           if(precision==2)
           sprintf(char_buffer ,"%-3.2f", vaule); putString_Rotate(x0,y0,char_buffer);
           if(precision==3)
           sprintf(char_buffer ,"%-3.3f", vaule); putString_Rotate(x0,y0,char_buffer);
           if(precision==4)
           sprintf(char_buffer ,"%-3.4f", vaule); putString_Rotate(x0,y0,char_buffer);
           }
           else if(flag=="+")
           {
           if(precision==1)
           sprintf(char_buffer ,"%+3.1f", vaule); putString_Rotate(x0,y0,char_buffer);
           if(precision==2)
           sprintf(char_buffer ,"%+3.2f", vaule); putString_Rotate(x0,y0,char_buffer);
           if(precision==3)
           sprintf(char_buffer ,"%+3.3f", vaule); putString_Rotate(x0,y0,char_buffer);
           if(precision==4)
           sprintf(char_buffer ,"%+3.4f", vaule); putString_Rotate(x0,y0,char_buffer);
           }
           else if(flag=="0")
           {
           if(precision==1)
           sprintf(char_buffer ,"%03.1f", vaule); putString_Rotate(x0,y0,char_buffer);
           if(precision==2)
           sprintf(char_buffer ,"%03.2f", vaule); putString_Rotate(x0,y0,char_buffer);
           if(precision==3)
           sprintf(char_buffer ,"%03.3f", vaule); putString_Rotate(x0,y0,char_buffer);
           if(precision==4)
           sprintf(char_buffer ,"%03.4f", vaule); putString_Rotate(x0,y0,char_buffer);
           }
           break;
    case 4: 
           if(flag=="n")
           {
            if(precision==1)
            sprintf(char_buffer ,"%4.1f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%4.2f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%4.3f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%4.4f", vaule); putString_Rotate(x0,y0,char_buffer);
           }
           else if(flag=="-")
           { 
            if(precision==1)
            sprintf(char_buffer ,"%-4.1f", vaule);  putString_Rotate(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%-4.2f", vaule);  putString_Rotate(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%-4.3f", vaule);  putString_Rotate(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%-4.4f", vaule);  putString_Rotate(x0,y0,char_buffer);
           }
           else if(flag=="+")
           {   
            if(precision==1)
            sprintf(char_buffer ,"%+4.1f", vaule);  putString_Rotate(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%+4.2f", vaule);  putString_Rotate(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%+4.3f", vaule);  putString_Rotate(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%+4.4f", vaule);  putString_Rotate(x0,y0,char_buffer);
           }
           else if(flag=="0")
           { 
            if(precision==1)
            sprintf(char_buffer ,"%04.1f", vaule);  putString_Rotate(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%04.2f", vaule);  putString_Rotate(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%04.3f", vaule);  putString_Rotate(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%04.4f", vaule);  putString_Rotate(x0,y0,char_buffer);
           }
           break;
    case 5: 
           if(flag=="n")
           {
            if(precision==1)
            sprintf(char_buffer ,"%5.1f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%5.2f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%5.3f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%5.4f", vaule); putString_Rotate(x0,y0,char_buffer);
           }    
           else if(flag=="-")
           {
            if(precision==1)
            sprintf(char_buffer ,"%-5.1f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%-5.2f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%-5.3f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%-5.4f", vaule); putString_Rotate(x0,y0,char_buffer);
           }
           else if(flag=="+")
           {
            if(precision==1)
            sprintf(char_buffer ,"%+5.1f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%+5.2f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%+5.3f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%+5.4f", vaule); putString_Rotate(x0,y0,char_buffer);
           }
           else if(flag=="0")
           {
            if(precision==1)
            sprintf(char_buffer ,"%05.1f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%05.2f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%05.3f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%05.4f", vaule); putString_Rotate(x0,y0,char_buffer);
           }
           break;
    case 6: 
           if(flag=="n")
           {
            if(precision==1)
            sprintf(char_buffer ,"%6.1f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%6.2f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%6.3f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%6.4f", vaule); putString_Rotate(x0,y0,char_buffer);
           }    
           else if(flag=="-")
           {
            if(precision==1)
            sprintf(char_buffer ,"%-6.1f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%-6.2f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%-6.3f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%-6.4f", vaule); putString_Rotate(x0,y0,char_buffer);
           }
           else if(flag=="+")
           {
            if(precision==1)
            sprintf(char_buffer ,"%+6.1f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%+6.2f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%+6.3f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%+6.4f", vaule); putString_Rotate(x0,y0,char_buffer);
           }
           else if(flag=="0")
           {
            if(precision==1)
            sprintf(char_buffer ,"%06.1f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%06.2f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%06.3f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%06.4f", vaule); putString_Rotate(x0,y0,char_buffer);
           }
           break; 
    case 7: 
           if(flag=="n")
           {
            if(precision==1)
            sprintf(char_buffer ,"%7.1f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%7.2f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%7.3f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%7.4f", vaule); putString_Rotate(x0,y0,char_buffer);
           } 
           else if(flag=="-")
           {
            if(precision==1)
            sprintf(char_buffer ,"%-7.1f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%-7.2f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%-7.3f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%-7.4f", vaule); putString_Rotate(x0,y0,char_buffer);
           }
           else if(flag=="+")
           {
            if(precision==1)
            sprintf(char_buffer ,"%+7.1f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%+7.2f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%+7.3f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%+7.4f", vaule); putString_Rotate(x0,y0,char_buffer);
           }
           else if(flag=="0")
           {
            if(precision==1)
            sprintf(char_buffer ,"%07.1f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%07.2f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%07.3f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%07.4f", vaule); putString_Rotate(x0,y0,char_buffer);
           }
           break;
    case 8: 
           if(flag=="n")
           {
            if(precision==1)
            sprintf(char_buffer ,"%8.1f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%8.2f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%8.3f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%8.4f", vaule); putString_Rotate(x0,y0,char_buffer);
           }     
           else if(flag=="-")
           {
            if(precision==1)
            sprintf(char_buffer ,"%-8.1f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%-8.2f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%-8.3f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%-8.4f", vaule); putString_Rotate(x0,y0,char_buffer);
           }
           else if(flag=="+")
           {
            if(precision==1)
            sprintf(char_buffer ,"%+8.1f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%+8.2f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%+8.3f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%+8.4f", vaule); putString_Rotate(x0,y0,char_buffer);
           }
           else if(flag=="0")
           {
            if(precision==1)
            sprintf(char_buffer ,"%08.1f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%08.2f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%08.3f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%08.4f", vaule); putString_Rotate(x0,y0,char_buffer);
           }
           break;
    case 9:
           if(flag=="n")
           {
            if(precision==1)
            sprintf(char_buffer ,"%9.1f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%9.2f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%9.3f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%9.4f", vaule); putString_Rotate(x0,y0,char_buffer);
           }  
           else if(flag=="-")
           {
            if(precision==1)
            sprintf(char_buffer ,"%-9.1f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%-9.2f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%-9.3f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%-9.4f", vaule); putString_Rotate(x0,y0,char_buffer);
           }
           else if(flag=="+")
           {
            if(precision==1)
            sprintf(char_buffer ,"%+9.1f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%+9.2f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%+9.3f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%+9.4f", vaule); putString_Rotate(x0,y0,char_buffer);
           }
           else if(flag=="0")
           {
            if(precision==1)
            sprintf(char_buffer ,"%09.1f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%09.2f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%09.3f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%09.4f", vaule); putString_Rotate(x0,y0,char_buffer);
           }
           break;
    case 10:
           if(flag=="n")
           {
            if(precision==1)
            sprintf(char_buffer ,"%10.1f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%10.2f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%10.3f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%10.4f", vaule); putString_Rotate(x0,y0,char_buffer);
           }  
           else if(flag=="-")
           {
            if(precision==1)
            sprintf(char_buffer ,"%-10.1f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%-10.2f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%-10.3f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%-10.4f", vaule); putString_Rotate(x0,y0,char_buffer);
           }
           else if(flag=="+")
           {
            if(precision==1)
            sprintf(char_buffer ,"%+10.1f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%+10.2f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%+10.3f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%+10.4f", vaule); putString_Rotate(x0,y0,char_buffer);
           }
           else if(flag=="0")
           {
            if(precision==1)
            sprintf(char_buffer ,"%010.1f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%010.2f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%010.3f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%010.4f", vaule); putString_Rotate(x0,y0,char_buffer);
           }
           break;
    case 11: 
           if(flag=="n")
           {
            if(precision==1)
            sprintf(char_buffer ,"%11.1f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%11.2f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%11.3f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%11.4f", vaule); putString_Rotate(x0,y0,char_buffer);
           } 
           else if(flag=="-")
           {
            if(precision==1)
            sprintf(char_buffer ,"%-11.1f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%-11.2f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%-11.3f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%-11.4f", vaule); putString_Rotate(x0,y0,char_buffer);
           }
           else if(flag=="+")
           {
            if(precision==1)
            sprintf(char_buffer ,"%+11.1f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%+11.2f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%+11.3f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%+11.4f", vaule); putString_Rotate(x0,y0,char_buffer);
           }
           else if(flag=="0")
           {
            if(precision==1)
            sprintf(char_buffer ,"%011.1f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==2)
            sprintf(char_buffer ,"%011.2f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==3)
            sprintf(char_buffer ,"%011.3f", vaule); putString_Rotate(x0,y0,char_buffer);
            if(precision==4)
            sprintf(char_buffer ,"%011.4f", vaule); putString_Rotate(x0,y0,char_buffer);
           }
           break;       
    default:   
           break;    
  }
}
//**************************************************************//
//vaule: 0x00000000 ~ 0xffffffff
//len: 1~11 minimum output length
/*
  [flag] 
    n:靠右,補空白　
    #:強制輸出 0x 作為開頭.
    0:在開頭處(左側) 補 0，而非補空白。
    x:強制輸出 0x 作為開頭，補 0。 
 */
//**************************************************************//
void Ra8876_Lite:: putHex_Rotate(ru16 x0,ru16 y0,ru32 vaule,ru8 len,const char *flag)
{
  char char_buffer[12];
  switch(len)
  {
    case 1:
           if(flag=="n")
           {sprintf(char_buffer ,"%1x", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="0")
           {sprintf(char_buffer ,"%01x", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="#")
           {sprintf(char_buffer ,"%#1x", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="x")
           {sprintf(char_buffer ,"%#01x", vaule); putString_Rotate(x0,y0,char_buffer);}
           break;
    case 2:
           if(flag=="n")
           {sprintf(char_buffer ,"%2x", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="0")
           { sprintf(char_buffer ,"%02x", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="#")
           { sprintf(char_buffer ,"%#2x", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="x")
           { sprintf(char_buffer ,"%#02x", vaule); putString_Rotate(x0,y0,char_buffer);}
           break; 
    case 3: 
           if(flag=="n")
           {sprintf(char_buffer ,"%3x", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="0")
           {sprintf(char_buffer ,"%03x", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="#")
           {sprintf(char_buffer ,"%#3x", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="x")
           {sprintf(char_buffer ,"%#03x", vaule); putString_Rotate(x0,y0,char_buffer);}
           break;
    case 4: 
           if(flag=="n")
           {sprintf(char_buffer ,"%4x", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="0")
           { sprintf(char_buffer ,"%04x", vaule);  putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="#")
           { sprintf(char_buffer ,"%#4x", vaule);  putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="x")
           { sprintf(char_buffer ,"%#04x", vaule);  putString_Rotate(x0,y0,char_buffer);}
           break;
    case 5: 
           if(flag=="n")
           {sprintf(char_buffer ,"%5x", vaule); putString_Rotate(x0,y0,char_buffer);}    
           else if(flag=="0")
           {sprintf(char_buffer ,"%05x", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="#")
           {sprintf(char_buffer ,"%#5x", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="x")
           {sprintf(char_buffer ,"%#05x", vaule); putString_Rotate(x0,y0,char_buffer);}
           break;
    case 6: 
           if(flag=="n")
           {sprintf(char_buffer ,"%6x", vaule); putString_Rotate(x0,y0,char_buffer);}    
           else if(flag=="0")
           {sprintf(char_buffer ,"%06x", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="#")
           {sprintf(char_buffer ,"%#6x", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="x")
           {sprintf(char_buffer ,"%#06x", vaule); putString_Rotate(x0,y0,char_buffer);}
           break; 
    case 7: 
           if(flag=="n")
           {sprintf(char_buffer ,"%7x", vaule); putString_Rotate(x0,y0,char_buffer);} 
           else if(flag=="0")
           {sprintf(char_buffer ,"%07x", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="#")
           {sprintf(char_buffer ,"%#7x", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="x")
           {sprintf(char_buffer ,"%#07x", vaule); putString_Rotate(x0,y0,char_buffer);}
           break;
    case 8: 
           if(flag=="n")
           {sprintf(char_buffer ,"%8x", vaule); putString_Rotate(x0,y0,char_buffer);}     
           else if(flag=="0")
           {sprintf(char_buffer ,"%08x", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="#")
           {sprintf(char_buffer ,"%#8x", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="x")
           {sprintf(char_buffer ,"%#08x", vaule); putString_Rotate(x0,y0,char_buffer);}
           break;
   case 9: 
           if(flag=="n")
           {sprintf(char_buffer ,"%9x", vaule); putString_Rotate(x0,y0,char_buffer);} 
           else if(flag=="0")
           {sprintf(char_buffer ,"%09x", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="#")
           {sprintf(char_buffer ,"%#9x", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="x")
           {sprintf(char_buffer ,"%#09x", vaule); putString_Rotate(x0,y0,char_buffer);}
           break;
   case 10: 
           if(flag=="n")
           {sprintf(char_buffer ,"%10x", vaule); putString_Rotate(x0,y0,char_buffer);}     
           else if(flag=="0")
           {sprintf(char_buffer ,"%010x", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="#")
           {sprintf(char_buffer ,"%#10x", vaule); putString_Rotate(x0,y0,char_buffer);}
           else if(flag=="x")
           {sprintf(char_buffer ,"%#010x", vaule); putString_Rotate(x0,y0,char_buffer);}
           break;
      
    default:   
           break;    
  }
}

