#include "lpc17xx_i2c.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_systick.h"
#include "lpc_types.h"
#include "string.h"
#include "serial.h"
//remove uart
int count = 0;

int send_message(char *message, int length){
        return(UART_Send((LPC_UART_TypeDef *) LPC_UART0, (uint8_t *)message, length, BLOCKING));  

}

void printsetup(void){
        UART_CFG_Type UARTConfigStruct;			// UART Configuration structure variable
	UART_FIFO_CFG_Type UARTFIFOConfigStruct;	// UART FIFO configuration Struct variable
	PINSEL_CFG_Type PinCfg;				// Pin configuration for UART
        
        PinCfg.OpenDrain = 0;
        PinCfg.Pinmode = 0;
        PinCfg.Funcnum = 1;   
        PinCfg.Portnum = 0;
        PinCfg.Pinnum = 2;
        PINSEL_ConfigPin(&PinCfg);
        PinCfg.Pinnum = 3;
        PINSEL_ConfigPin(&PinCfg);

        UART_ConfigStructInit(&UARTConfigStruct);      

        UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);

        UART_Init((LPC_UART_TypeDef *) LPC_UART0, &UARTConfigStruct);

        UART_TxCmd((LPC_UART_TypeDef *) LPC_UART0, ENABLE);
}


void I2Csetup(void){

        PINSEL_CFG_Type PinCfg;
        
        PinCfg.OpenDrain = 0;
        PinCfg.Pinmode = 0;
        PinCfg.Funcnum = 3;   
        PinCfg.Portnum = 0;
        PinCfg.Pinnum = 1;
        PINSEL_ConfigPin(&PinCfg);
        PinCfg.Pinnum = 0;
        PINSEL_ConfigPin(&PinCfg);

        uint32_t I2Csystemcoreclock = 100000;
        I2C_Init((LPC_I2C_TypeDef *) LPC_I2C1, I2Csystemcoreclock);
        I2C_Cmd((LPC_I2C_TypeDef *) LPC_I2C1, 1);
}

void lcdsetup(void){

        I2C_M_SETUP_Type SEND;

        uint8_t LCDinit[11] = {0x00
,0x34,0x0c,0x06,0x35,0x04,0x10,0x42,0x9f,0x34,0x02};
        
        SEND.sl_addr7bit = 59;
        SEND.tx_data = LCDinit;
        SEND.tx_length = sizeof(uint8_t)*11;
        SEND.rx_data = NULL;
        SEND.rx_length = 0;
        SEND.retransmissions_max = 3;
        
        uint8_t transfer = I2C_MasterTransferData((LPC_I2C_TypeDef *) LPC_I2C1 , &SEND, I2C_TRANSFER_POLLING );
                
        if (transfer == SUCCESS)
        {
        char message[] = "screen set up \n\r";
        send_message(message, strlen(message));
        }
                
}

void lcdclear(void){
        
        I2C_M_SETUP_Type SEND;

        uint8_t clear[2] = {0x00 , 0x01};
        
        SEND.sl_addr7bit = 59;
        SEND.tx_data = clear;
        SEND.tx_length = sizeof(uint8_t)*2;
        SEND.rx_data = NULL;
        SEND.rx_length = 0;
        SEND.retransmissions_max = 3;

        I2C_MasterTransferData((LPC_I2C_TypeDef *) LPC_I2C1 , &SEND, I2C_TRANSFER_POLLING );
}

void columnread(unsigned char * buf) {

                I2C_M_SETUP_Type READ;                        
                
                READ.sl_addr7bit = 33;
                READ.tx_data = NULL;
                READ.tx_length = 0;
                READ.rx_data = &buf[0];
                READ.rx_length = sizeof(unsigned char);
                READ.retransmissions_max = 3;
                              
                I2C_MasterTransferData((LPC_I2C_TypeDef *) LPC_I2C1 , &READ, I2C_TRANSFER_POLLING );
                }

void columnwrite(unsigned char * buf) {
                
                I2C_M_SETUP_Type SEND;

                SEND.sl_addr7bit = 33;
                SEND.tx_data = buf;
                SEND.tx_length = sizeof(unsigned char);
                SEND.rx_data = NULL;
                SEND.rx_length = 0;
                SEND.retransmissions_max = 3;

                I2C_MasterTransferData((LPC_I2C_TypeDef *) LPC_I2C1 , &SEND, I2C_TRANSFER_POLLING );
                }


void printchar(uint8_t * buf) {
        count += 1;
        I2C_M_SETUP_Type SEND;
        
        SEND.sl_addr7bit = 59;
        SEND.tx_data = buf;
        SEND.tx_length = sizeof(uint8_t) * 2;
        SEND.rx_data = NULL;
        SEND.rx_length = 0;
        SEND.retransmissions_max = 3;        

        I2C_MasterTransferData((LPC_I2C_TypeDef *) LPC_I2C1 , &SEND, I2C_TRANSFER_POLLING );

}

void debounce(unsigned char testbuf[] , uint8_t column){
        unsigned char test[0x6];
        uint8_t keypadvalues[4][4] = {{0xB1, 0xB4, 0xB7, 0xaa},
                                    {0xB2, 0xB5, 0xB8, 0xB0},
                                    {0xB3, 0xB6, 0xB9, 0xa3},
                                    {0xc1, 0xc2, 0xc3, 0xc4}};
        
                                                // keypad
        int colposition = (0x0F &(~column));
        if (colposition == 4){colposition = 3;}
        else if (colposition == 8){colposition = 4;}
        colposition = 4 - colposition ;
        
        //debounce
                        if(testbuf[0] == ((column<<4) | 0x07)){
                              uint8_t keypadentry[2] = {0x40, keypadvalues[colposition][0]};
                              printchar(keypadentry);    
                        }
                        else if(testbuf[0] == ((column<<4) | 0x0B)){
                              uint8_t keypadentry[2] = {0x40, keypadvalues[colposition][1]};
                              printchar(keypadentry); 
                               
                        }
                        else if(testbuf[0] == ((column<<4) | 0x0D)){
                              uint8_t keypadentry[2] = {0x40, keypadvalues[colposition][2]};
                              printchar(keypadentry);       
                        }
                        else if(testbuf[0] == ((column<<4) | 0x0E)){
                              uint8_t keypadentry[2] = {0x40, keypadvalues[colposition][3]};
                              printchar(keypadentry); 
                        }
                do{
                columnread(test);
                }
                while(test[0] == testbuf[0]); 
                
                if(count == 16){
                        uint8_t keypadthing2[2] = {0x40, keypadvalues[12]};
                        for(i = 0; i < 24; i++){
                        printchar(keypadthing2);
                }
                count = 0;
                
              

}


void keypad(void) {
        printsetup();
        I2Csetup();
        
        lcdsetup();
        lcdclear();
  
        unsigned char buf[0x6];
        unsigned char testbuf[0x6];
        
        
        while(1){
                buf[0] = 0x7F ;
                columnwrite(buf);
                columnread(testbuf);
                if (buf[0] != testbuf[0]){debounce(testbuf,0x07);}     
                            
                buf[0] = 0xBF ;
                columnwrite(buf);
                columnread(testbuf);
                if (buf[0] != testbuf[0]){debounce(testbuf,0x0B);}
               
                buf[0] = 0xDF ;
                columnwrite(buf);
                columnread(testbuf);
                if (buf[0] != testbuf[0]){debounce(testbuf,0x0D);}
                
                buf[0] = 0xEF ;
                columnwrite(buf);
                columnread(testbuf);
                if (buf[0] != testbuf[0]){debounce(testbuf,0x0E);}
                
                
                }    
}

void main(void){
        keypad();
}
