
/*
 *  单片机内部PLLX4
 */
#include<htc.h>

#include"modbus_rtu.h"

#include"modbus_rtu_config.h"

// PIC18F25K22 Configuration Bit Settings

// PIC18F25K22 Configuration Bit Settings

#include <htc.h>

//#define PORTCbits.RC4
//#pragma config CONFIG1H = 0x23
__CONFIG(1, FOSC_HSMP & PLLCFG_ON & PRICLKEN_ON & FCMEN_OFF & IESO_OFF);
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
void start_timer3(void);
void stop_timer3(void);
void init_pv1(void);//下降沿中断采集速度
void pv1_isr(void);
unsigned int count_10ms = 0;
void init_timer3(void)//测速定时器
{
//11.0592M x 4
     TMR3ON = 0;

     TMR3CS0 = 0;
     TMR3CS1 = 0;

     T3CKPS0 = 0;
     T3CKPS1 = 0;

     TMR3 = 65535-11059;
     TMR3IF=0;
     TMR3IE=1;
     TMR3ON = 1;
}

void timer3_isr(void)
{

	if(TMR3IE&&TMR3IF)
	{
                TMR3IF = 0;
		TMR3 = 65535-11059;
		count_10ms++;
                if(count_10ms==6000)
                {
                      modbus_reg[0] = 0;
                      count_10ms--;
                }
	}
}
void main(void)
{
    unsigned int md_keep;
    ANSELC = 0;
    TRISC = 0x80;//输出
   PORTC = 0xc0;

   ANSELA = 0;
   TRISA5 =0;
   RA5 = 0;


    init_timer35();
    
    PEIE = 1;
    GIE = 1;
    init_uart();

    init_timer3();
    init_pv1();
    PORTCbits.RC5  =  1;
    while(1)
	{
            modbus_receive();

          //  modbus_reg[0] = 100;//发送给组态王的数据

            //modbus_reg[0] = 200；
            if(modbus_reg[3]==1)
            {
                //RA5 = 1;
            }
            else
            {
                //RA5 = 0;
            }

	}
}

void interrupt my_isr(void)
{
    rec_isr();
    tx_isr();
    timer35_isr();
    timer3_isr();
    pv1_isr();
}
void get_speed(void)
{
	static unsigned int pulse_count = 0;//脉冲数量
        static unsigned char sample_step = 1;
	//采样6个脉冲
	switch(sample_step)
	{
		case 1:
			start_timer3();
			pulse_count++;
			sample_step = 2;
		break;
		case 2:
			pulse_count++;
			if(pulse_count==6)
			{
				stop_timer3();
				pulse_count = 0;
				//temp_speed  = count_10ms;
                                stop_timer3();                               
				//current_speed = count_10ms/250;
                                if(count_10ms>0)
                                {
                                     modbus_reg[0] = (unsigned int)(100000/count_10ms);
                                    //modbus_reg[0] = count_10ms;
                                }

                                 count_10ms = 0;
				//reset_timer_value();
				sample_step = 1;
			}
		break;

		default:
			sample_step = 1;
		break;
	}
}

void start_timer3(void)
{
        TMR3 = 65535-11059;
        TMR3ON = 1;
}
void stop_timer3(void)
{
    TMR3ON = 0;
    TMR3 = 65535-11059;
}
void init_pv1(void)//下降沿中断采集速度
{
    ANSELB = 0;
        TRISB0=0;
        RB0=1;      //为下降沿创造高电平的初始条件
        TRISB0=1;   //输入模式
      INTEDG0  = 0;
      INT0IE = 1;
      INT0IF = 0;
}
void pv1_isr(void)
{
    if(INT0IF&&INT0IE)
    {
        INT0IF = 0;
           get_speed();
    }
}
