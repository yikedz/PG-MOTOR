
#include<htc.h>

#include"modbus_rtu.h"

#include"modbus_rtu_config.h"

// PIC18F25K22 Configuration Bit Settings

#include <htc.h>


// PIC18F25K22 Configuration Bit Settings

#include <htc.h>

//#pragma config CONFIG1H = 0x23
__CONFIG(1, FOSC_HSMP & PLLCFG_OFF & PRICLKEN_ON & FCMEN_OFF & IESO_OFF);
//#pragma config CONFIG2L = 0x1F
__CONFIG(2, PWRTEN_OFF & BOREN_SBORDIS & BORV_190);
//#pragma config CONFIG2H = 0x3F
__CONFIG(3, WDTEN_ON & WDTPS_32768);
//#pragma config CONFIG3H = 0xBF
__CONFIG(4, CCP2MX_PORTC1 & PBADEN_ON & CCP3MX_PORTB5 & HFOFST_ON & T3CMX_PORTC0 & P2BMX_PORTB5 & MCLRE_EXTMCLR);
//#pragma config CONFIG4L = 0x85
__CONFIG(5, STVREN_ON & LVP_ON & XINST_OFF);
//#pragma config CONFIG5L = 0xF
__CONFIG(6, CP0_OFF & CP1_OFF & CP2_OFF & CP3_OFF);
//#pragma config CONFIG5H = 0xC0
__CONFIG(7, CPB_OFF & CPD_OFF);
//#pragma config CONFIG6L = 0xF
__CONFIG(8, WRT0_OFF & WRT1_OFF & WRT2_OFF & WRT3_OFF);
//#pragma config CONFIG6H = 0xE0
__CONFIG(9, WRTC_OFF & WRTB_OFF & WRTD_OFF);
//#pragma config CONFIG7L = 0xF
__CONFIG(10, EBTR0_OFF & EBTR1_OFF & EBTR2_OFF & EBTR3_OFF);
//#pragma config CONFIG7H = 0x40
__CONFIG(11, EBTRB_OFF);
extern unsigned int modbus_reg[];

void main(void)
{
    unsigned int md_keep;

    ANSELC = 0;
    TRISC = 0x80;//输出
   PORTC = 0xc0;


    init_timer35();
    
    PEIE = 1;
    GIE = 1;
    init_uart();
    while(1)
	{
            modbus_receive();

            modbus_reg[1] = 100;//发送给组态王的数据

            //modbus_reg[0] = 200；
            
            if(modbus_reg[0]!=md_keep)//modbus_reg[0] 组态王发送过来的数据
            {
                md_keep = modbus_reg[0];
                //PORTC = md_keep;
            }
	}
}

void interrupt my_isr(void)
{
    rec_isr();
    tx_isr();
    timer35_isr();
}