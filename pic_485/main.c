
/*
 *  单片机内部PLLX4
 */
#include<htc.h>

#include"modbus_rtu.h"

#include"modbus_rtu_config.h"

// PIC18F25K22 Configuration Bit Settings

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
//unsigned int PWM_PERCENT[100];


#define PG_OUT (PORTCbits.RC5)
#define BEEP (PORTAbits.RA5)

extern unsigned int modbus_reg[];

unsigned int count_1ms = 0;
unsigned char have_speed = 0;
unsigned char count_2s_flg = 0;

void init_timer0(void);//初始化timer0 测速使用 速度太低 超时使速度为零
void init_timer3(void);//测速定时器
void init_timer5(void);


void timer0_isr(void);
void timer3_isr(void);
void timer5_isr(void);

//to get speed
void start_timer3(void);
void stop_timer3(void);


void init_pg_in(void);//初始化RB1,采集过零脉冲信号
void init_pv1(void);//下降沿中断采集速度

void pg_in_isr(void);//过零脉冲中断处理
void pv1_isr(void);//速度采集中断处理


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
		
		count_1ms++;
		
                if(count_1ms==6000)
                {
                      modbus_reg[0] = 0;
                      count_1ms--;
                }
	}
}
void main(void)
{

    ANSELC = 0;
    TRISC = 0x80;//输出
    PORTC = 0xc0;

   //BEEP
   ANSELA = 0;
   TRISA5 =0;

    init_timer35();
    init_uart();

    init_timer3();
    init_pv1();
    
    init_timer0();
	
    init_pg_in();
    init_timer5();

    PEIE = 1;
    GIE = 1;
    
    while(1)
	{

			if(modbus_reg[3]==1)
				{
					BEEP = 1;
				}
			else
				{
					BEEP = 0;
				}
            modbus_receive();
            
            if(count_2s_flg)
            {
                count_2s_flg = 0;
                
                if(have_speed)
                {
                    have_speed = 0;
                }
                else//2s 没有速度
                {
                    modbus_reg[0] = 0;
                }
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
    timer0_isr();
    pg_in_isr();
    timer5_isr();
}
void get_speed(void)
{
	static unsigned int pulse_count = 0;//脉冲数量
    static unsigned char sample_step = 1;
    
	//采样6个脉冲
        have_speed = 1;
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
                                stop_timer3();                               
                                if(count_1ms>0)
                                {
                                     modbus_reg[0] = (unsigned int)(100000/count_1ms);
                                }
                                 count_1ms = 0;
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

void init_timer0(void)//初始化timer0 测速使用 速度太低 超时使速度为零
{
    TMR0ON = 0;//turn off timer0
    T08BIT = 0;//16bit reg mode
    T0CS = 0;//clock input Fosc/4 if the value=1 as counter
    //4 分频 (2 4 8 16 32 64 128 256)
    T0PS0 = 1;
    T0PS1 = 0;
    T0PS2 = 0;

    PSA = 0;//use 分频器

        
    TMR0 = 65535-27648;//10MS
    TMR0IF = 0;
    TMR0IE = 1;
    TMR0ON = 1;

}
void timer0_isr(void)
{
    static unsigned char count0_10ms = 0;
    
       if(TMR0IF&&TMR0IE)
       {
           TMR0 = 65535-27648;//10MS
           TMR0IF = 0;
           count0_10ms++;

           if(count0_10ms==200)
           {
                //PORTCbits.RC4 = ~PORTCbits.RC4;
                count_2s_flg = 1;
                count0_10ms = 0;
           }
       }
}

void init_pg_in(void)//下降沿中断采集速度
{
    //ANSELB = 1;
    TRISB1=0;
    RB1=0;      //为下降沿创造高电平的初始条件
    TRISB1=1;   //输入模式
      INTEDG1  = 1;//上升边沿
      INT1IE = 1;
      INT1IF = 0;
}

void pg_in_isr(void)
{
   if(INT1IE&&INT1IF)
   {
       INT1IF = 0;
       PG_OUT = 0;//关闭输出
	   //TMR5 = 65535-PWM_PERCENT[modbus_reg[2]];
       TMR5 = 65535-55296;
       TMR5ON = 1;//启动定时器
   }
}
void init_timer5(void)//控制pwm调节定时器 最大值10ms (0-10ms) 100us 100等分 Fosc/8
{
    //11.0592M x 4
     TMR5ON = 0;

     TMR5CS0 = 0;
     TMR5CS1 = 0;

     T5CKPS0 = 0;
     T5CKPS1 = 0;

     TMR5 = 65535-55296;
	 
     TMR5IF=0;
     TMR5IE=1;
	//TMR5ON = 1;
}
void timer5_isr(void)
{
       if(TMR5IF&&TMR5IE)
	   {
		TMR5IF = 0;
		 // TMR5 = 65535-55296;
		PG_OUT = 1;//开始转动
		TMR5ON = 0;//停止定时器停止
		//PORTCbits.RC4 = ~PORTCbits.RC4;
	   }
}