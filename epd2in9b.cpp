/**
 *  @filename   :   epd2in9b.cpp
 *  @brief      :   Implements for Dual-color e-paper library
 *  @author     :   Yehui from Waveshare
 *
 *  Copyright (C) Waveshare     August 10 2017
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documnetation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to  whom the Software is
 * furished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdlib.h>
#include "epd2in9b.h"

Epd::~Epd() {
};

Epd::Epd() {
    reset_pin = RST_PIN;
    dc_pin = DC_PIN;
    cs_pin = CS_PIN;
    busy_pin = BUSY_PIN;
    width = EPD_WIDTH;
    height = EPD_HEIGHT;
};

const unsigned char init_data[] = {
0x22,  0x11, 0x10, 0x00, 0x10, 0x00, 0x00, 0x11, 0x88, 0x80, 0x80, 0x80, 0x00, 0x00, 0x6A, 0x9B,
0x9B, 0x9B, 0x9B, 0x00, 0x00, 0x6A, 0x9B, 0x9B, 0x9B, 0x9B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x04, 0x18, 0x04, 0x16, 0x01, 0x0A, 0x0A, 0x0A, 0x0A, 0x02, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x04, 0x08, 0x3C, 0x07, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

int Epd::Init(void) {
    /* this calls the peripheral hardware interface, see epdif */
    if (IfInit() != 0) {
        return -1;
    }
    /* EPD hardware init start */
    Reset();
    SendCommand(0x12);
    WaitUntilIdle();
    SendCommand(0x74);
    SendData(0x54);
    SendCommand(0x75);
    SendData(0x3b);
    SendCommand(0x01);   // Set MUX as 296
    SendData(0x27);
    SendData(0x01);
    SendData(0x00);
    SendCommand(0x3A);   // Set 100Hz
    SendData(0x35);         // Set 130Hz
    SendCommand(0x3B);   // Set 100Hz
    SendData(0x04);         // Set 130Hz
    SendCommand(0x11);   // data enter mode
    SendData(0x03);
    /*
    SendCommand(0x44);   // set RAM x address start/end, in page 36
    SendData(0x00);    // RAM x address start at 00h;
    SendData(0x0f);    // RAM x address end at 0fh(15+1)*8->128 
    SendCommand(0x45);   // set RAM y address start/end, in page 37
    SendData(0x00);    // RAM y address start at 127h;
    SendData(0x00);      
    SendData(0x27);    // RAM y address end at 00h;
    SendData(0x01);    */
    
    SendCommand(0x04);   // set VSH,VSL value
    SendData(0x41);    //      2D9  15v
    SendData(0xa8);   //      2D9   5v 
    SendData(0x32);    //      2D9  -15v
    SendCommand(0x2C);           // vcom
    SendData(0x68);           //-2.6V
    SendCommand(0x3C);   // board
    SendData(0x33);    //GS1-->GS1
    SendCommand(0x32);   // board
    for(int i=0;i<70;i++){    // write LUT register with 29bytes instead of 30bytes 2D13
      SendData(init_data[i]);
    }
    /* EPD hardware init end */
    return 0;

}

/**
 *  @brief: basic function for sending commands
 */
void Epd::SendCommand(unsigned char command) {
    DigitalWrite(dc_pin, LOW);
    SpiTransfer(command);
}

/**
 *  @brief: basic function for sending data
 */
void Epd::SendData(unsigned char data) {
    DigitalWrite(dc_pin, HIGH);
    SpiTransfer(data);
}

void Epd::SetPartialWindowAux(const unsigned char* buffer, int x, int y, int w, int l, int color){
  this->set_xy_window(x>>3, ((x+w)>>3)-1, y, (y+l)-1);
  DelayMs(2);
  this->set_xy_counter(x>>3, y);
  DelayMs(2);
  switch(color){
    case COLOR_RED:
      SendCommand(0x26); // Write RAM (red)
      break;
    case COLOR_BW:
      SendCommand(0x24); // Write RAM (B/W)
    default:
      break;
  }

  if (buffer != NULL) {
        for(int i = 0; i < w  / 8 * l; i++) {
            SendData(buffer[i]);  
        }  
  } else {
        for(int i = 0; i < w  / 8 * l; i++) {
          switch(color){
            case COLOR_RED:
              SendData(0x00);
              break;
            case COLOR_BW:
            default:
              SendData(0xFF);
              break;
          }
        }  
    }
  DelayMs(2);
  SendCommand(0x22); 
}

/**
 *  @brief: Wait until the busy_pin goes HIGH
 */
void Epd::WaitUntilIdle(void) {
    int busy_value = HIGH;

  while(1){
    busy_value = digitalRead(BUSY_PIN);
    if(busy_value == LOW){
      break;
    }
  }     
}

/**
 *  @brief: module reset. 
 *          often used to awaken the module in deep sleep, 
 *          see Epd::Sleep();
 */
void Epd::Reset(void) {
    DigitalWrite(reset_pin, LOW);
    DelayMs(200);
    DigitalWrite(reset_pin, HIGH);
    DelayMs(200);   
}

/**
 *  @brief: transmit partial data to the SRAM
 */
void Epd::SetPartialWindow(const unsigned char* buffer_black, const unsigned char* buffer_red, int x, int y, int w, int l) {
  SetPartialWindowAux(buffer_black, 0, 0, this->width, this->height, COLOR_BW);
  SetPartialWindowAux(buffer_red, 0, 0, this->width, this->height, COLOR_RED);
}

void Epd::set_xy_window(unsigned char xs, unsigned char xe, unsigned int ys, unsigned int ye){
  SendCommand(0x44);    // set RAM x address start/end, in page 36
  SendData(xs);    // RAM x address start at 00h;
  SendData(xe);    // RAM x address end at 0fh(12+1)*8->104 
  SendCommand(0x45);   // set RAM y address start/end, in page 37
  SendData(ys);    // RAM y address start at 0;
  SendData(ys>>8);   
  SendData(ye);    // RAM y address end at 
  SendData(ye>>8);   // RAM y address end at
}

void Epd::set_xy_counter(unsigned char x, unsigned char y){
  SendCommand(0x4E);    // set RAM x address count 
  SendData(x);
  SendCommand(0x4F);   // set RAM y address count  
  SendData(y);
  SendData(y>>8);
}



/**
 *  @brief: transmit partial data to the black part of SRAM
 */
void Epd::SetPartialWindowBlack(const unsigned char* buffer_black, int x, int y, int w, int l) {
  SetPartialWindowAux(buffer_black, x, y, w, l, COLOR_BW);
}

/**
 *  @brief: transmit partial data to the red part of SRAM
 */
void Epd::SetPartialWindowRed(const unsigned char* buffer_red, int x, int y, int w, int l) {
  SetPartialWindowAux(buffer_red, x, y, w, l, COLOR_RED); 
}

/**
 * @brief: refresh and displays the frame
 */
void Epd::DisplayFrame(const unsigned char* frame_buffer_black, const unsigned char* frame_buffer_red) {
  this->SetPartialWindow(frame_buffer_black, frame_buffer_red, 0, 0, this->width, this->height);

  SendCommand(0x22);
  this->DisplayFrame();
}

/**
 * @brief: clear the frame data from the SRAM, this won't refresh the display
 */
void Epd::ClearFrame(void) {
  this->set_xy_window(0, (this->width>>3)-1, 0, this->height-1);
  this->set_xy_counter(0, 0);
  SendCommand(0x24); // Write RAM (B/W)
  for (int i = 0; i < this->width * this->height / 8; i++) {
    SendData(0xFF);  
  }

  SendCommand(0x26); // Write RAM (red)  
  for (int i = 0; i < this->width * this->height / 8; i++) {
    SendData(0x00);  
  }
  SendCommand(0x22);
}

/**
 * @brief: This displays the frame data from SRAM
 */
void Epd::DisplayFrame(void) {
  SendData(0xC7);    //Load LUT from MCU(0x32), Display update
  SendCommand(0x20);
  WaitUntilIdle();
}

/**
 * @brief: After this command is transmitted, the chip would enter the deep-sleep mode to save power. 
 *         The deep sleep mode would return to standby by hardware reset. The only one parameter is a 
 *         check code, the command would be executed if check code = 0xA5. 
 *         You can use Epd::Reset() to awaken and use Epd::Init() to initialize.
 */
void Epd::Sleep() {
  /*SendCommand(DEEP_SLEEP);
  SendData(0xa5);*/
  return;
}


/* END OF FILE */


