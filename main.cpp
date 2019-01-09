//********************************************************************************************
//                                                                                           *
// AB&T Tecnologie Informatiche - Ivrea Italy                                                *
// http://www.bausano.net                                                                    *
// https://www.ethercat.org/en/products/791FFAA126AD43859920EA64384AD4FD.htm                 *
//                                                                                           *  
//********************************************************************************************    
//                                                                                           *
// This software is distributed as an example, in the hope that it could be useful,          *
// WITHOUT ANY WARRANTY, even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE       *
//                                                                                           *
//******************************************************************************************** 


// revision 2 - moved  "DigitalOut Led(LED1)"



//----- EasyCAT shield application basic example for mbed boards 170912 ----------------------
//----- Derived from the example project TestEasyCAT.ino for the AB&T EasyCAT Arduino shield

//----- Tested with the STM32 NUCLEO-F767ZI board --------------------------------------------



#include "mbed.h"   
#include "EasyCAT.h"                // EasyCAT library to interface the LAN9252
    

void Application (void); 

 
EasyCAT EASYCAT;                    // EasyCAT istantiation

                                    // The constructor allow us to choose the pin used for the EasyCAT SPI chip select 
                                    // Without any parameter pin 9 will be used 
                                                                      
                                    // for EasyCAT board REV_A we can choose between:
                                    // 8, 9, 10 
                                    //  
                                    // for EasyCAT board REV_B we can choose between:
                                    // 8, 9, 10, A5, 6, 7                                    

                                    // example:                                  
//EasyCAT EASYCAT(8);               // pin 8 will be used as SPI chip select


                                    // The chip select chosen by the firmware must match the setting on the board

                                    // On board REV_A the chip select is set soldering
                                    // a 0 ohm resistor in the appropriate position

                                    // On board REV_B the chip select is set
                                    // througt a bank of jumpers                                    



//---- pins declaration ------------------------------------------------------------------------------



AnalogIn Ana0(A0);               // analog input 0
AnalogIn Ana1(A1);               // analog input 1


DigitalOut Out_0(A2);            // four bits output
DigitalOut Out_1(A3);            // 
DigitalOut Out_2(A4);            // 
DigitalOut Out_3(A5);            // 

DigitalIn In_0(D3);              // four bits input
DigitalIn In_1(D5);              // 
DigitalIn In_2(D6);              // 
DigitalIn In_3(D7);              //     





//---- global variables ---------------------------------------------------------------------------


UWORD ContaUp;                      // used for sawthoot test generation
UWORD ContaDown;                    //

unsigned long Millis = 0;
unsigned long PreviousSaw = 0;
unsigned long PreviousCycle = 0;


//---- declarations for Arduino "millis()" emulation ----------------------- 

static Ticker uS_Tick;
static volatile uint32_t MillisVal = 0;

void InitMillis(void);    
void mS_Tick(void);
  
inline static uint32_t millis (void) 
{
    return MillisVal; 
};
 
      

//---------------------------------------------------------------------------------------------
 
int main(void)
{
          
  printf ("\nEasyCAT - Generic EtherCAT slave\n");      // print the banner

  InitMillis();                                         // init Arduino "millis()" emulation                

  ContaDown.Word = 0x0000;
  ContaUp.Word = 0x0000; 
                                                        //---- initialize the EasyCAT board -----
                                                                  
  if (EASYCAT.Init() == true)                           // initialization
  {                                                     // succesfully completed
    printf ("initialized\n");                           //
  }                                                            
  
  else                                                  // initialization failed   
  {                                                     // the EasyCAT board was not recognized
    printf ("initialization failed\n");                 //     
                                                        // The most common reason is that the SPI 
                                                        // chip select choosen on the board doesn't 
                                                        // match the one choosen by the firmware
                                                             
    DigitalOut Led(LED1);                               //                                                                   
                                                                  
    while (1)                                           // stay in loop for ever
    {                                                   // with the led blinking
        Led = 1;                                        //    
        wait_ms(125);                                   //   
        Led = 0;                                        //
        wait_ms(125);                                   //
    }                                                   // 
  }   
   
 
  while (1)                                             //---- main loop ---------------------------
  {      
                                                        // In the main loop we must call ciclically the 
                                                        // EasyCAT task and our application
                                                        //
                                                        // This allows the bidirectional exachange of the data
                                                        // between the EtherCAT master and our application
                                                        //
                                                        // The EasyCAT cycle and the Master cycle are asynchronous
                                                        //     
  
   // wait_ms(20);                                      // Here we to set the EasyCAT cycle time  
                                                        // according to the needs of our application
                                                        //
                                                        // For user interface applications a cycle time of 100mS,
                                                        // or even more, is appropriate, but, for data processing 
                                                        // applications, a faster cycle time may be required
                                                        //
                                                        // In this case we can also completely eliminate this
                                                        // delay in order to obtain the fastest possible response
                                                        

                                                        // Instead we can also use millis() to set the cycle time  
                                                        //  
                                                        // example:
    Millis = millis();                                  //
    if (Millis - PreviousCycle >= 10)                   // each 10 mS   
    {                                                   // 
      PreviousCycle = Millis;                           //
    
      EASYCAT.MainTask();                               // execute the EasyCAT task
                                                                      
      Application();                                    // execute the user application
    }    
  }  
}


//---- user application ------------------------------------------------------------------------------

void Application (void)                                        

{ 
  float Analog; 
                                                      // --- analog inputs management ---
                                                      //
  Analog = Ana0.read();                               // read analog input 0
  Analog = Analog * 255;                              // normalize it on 8 bits   
  EASYCAT.BufferIn.Byte[0] = (uint8_t)Analog;         // and put the result into
                                                      // input Byte 0 
                                                      
  Analog = Ana1.read();                               // read analog input 1
  Analog = Analog * 255;                              // normalize it on 8 bits  
  EASYCAT.BufferIn.Byte[1] = (uint8_t)Analog;         // and put the result into
                                                      // input Byte 1    


                                                      // --- four output bits management ----
                                                      //       
  if (EASYCAT.BufferOut.Byte[0] & 0b00000001)         // the four output bits are mapped to the
    Out_0 = 1;                                        // lower nibble of output Byte 0
  else                                                //
    Out_0 = 0;                                        // 
                                                      //
  if (EASYCAT.BufferOut.Byte[0] & (1<<0))             // 
    Out_0 = 1;                                        //
  else                                                //
    Out_0 = 0;                                        // 
                                                      //
  if (EASYCAT.BufferOut.Byte[0] & (1<<1))             // 
    Out_1 = 1;                                        //
  else                                                //
    Out_1 = 0;                                        // 
                                                      //
  if (EASYCAT.BufferOut.Byte[0] & (1<<2))             // 
    Out_2 = 1;                                        //
  else                                                //
    Out_2 = 0;                                        // 
                                                      //
  if (EASYCAT.BufferOut.Byte[0] & (1<<3))             // 
    Out_3 = 1;                                        //
  else                                                //
    Out_3 = 0;                                        //         

                                                      //--- four input bits management ---
                                                      //  
  EASYCAT.BufferIn.Byte[6] = 0x00;                    // the four input pins are mapped to the     
  if (!In_0)                                          // lower nibble of input Byte 6
    EASYCAT.BufferIn.Byte[6] |= 0b00000001;           //    
  if (!In_1)                                          //        
    EASYCAT.BufferIn.Byte[6] |= 0b00000010;           //    
  if (!In_2)                                          //
    EASYCAT.BufferIn.Byte[6] |= 0b00000100;           //
  if (!In_3)                                          //
    EASYCAT.BufferIn.Byte[6] |= 0b00001000;           //
    
    
                                                      // --- test sawtooth generation --- 
                                                      //
  Millis = millis();                                  // each 100 mS
  
  if (Millis - PreviousSaw >= 100)                    // 
  {                                                   // 
    PreviousSaw = Millis;                             //
                                                      //
    ContaUp.Word++;                                   // we increment the variable ContaUp  
    ContaDown.Word--;                                 // and decrement ContaDown   
  }                                                   //

                                                      // we use these variables to create sawtooth,
                                                      // with different slopes and periods, for
                                                      // test pourpose, in input Bytes 2,3,4,5,30,31
                                                      
  EASYCAT.BufferIn.Byte[2] = ContaUp.Byte[0];         // slow rising slope
  EASYCAT.BufferIn.Byte[3] = ContaUp.Byte[1];         // extremly slow rising slope
  
  EASYCAT.BufferIn.Byte[4] = ContaDown.Byte[0];       // slow falling slope
  EASYCAT.BufferIn.Byte[5] = ContaDown.Byte[1];       // extremly slow falling slope
 
 
  EASYCAT.BufferIn.Byte[30] = ContaUp.Byte[0] << 2;   // medium speed rising slope
  EASYCAT.BufferIn.Byte[31] = ContaDown.Byte[0] << 2; // medium speed falling slope    
}   


//--- functions for Arduino "millis()" emulation -------------------------------------


void InitMillis(void) 
{
  uS_Tick.attach (&mS_Tick, 0.001);        
}

void mS_Tick(void)
{
  MillisVal++;
}