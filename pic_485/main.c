#include<htc.h>

#include"modbus_rtu.h"

#include"modbus_rtu_config.h"

extern unsigned int modbus_reg[];

void main(void)
{
    unsigned int md_keep;

    ANSELC = 0;
    TRISC = 0x00;//Êä³ö
    PORTC = 0xc0;
    
    init_timer35();
    
    PEIE = 1;
    GIE = 1;
    init_uart();
    while(1)
	{
            modbus_receive();
            
            if(modbus_reg[0]!=md_keep)
            {
                md_keep = modbus_reg[0];
                PORTC = md_keep;
            }
	}
}

void interrupt my_isr(void)
{
    rec_isr();
    tx_isr();
    timer35_isr();
}