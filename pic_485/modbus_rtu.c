/*
 *
 *   单片机信号：PIC18F25K22
 *   开发环境：Mplab X V1.7
 *   编译器：HI-TECH-PICC18(V9.66)
 *
 * 基于中断接收数据，使用中断发送数据
 * rec_isr(在中断保证接受到一帧数据，标志位置位）-->
 * modbus_receive查询标志位，处理该帧数据-->
 * 有数据需要返回时，使能发送中断，单片机会自动发送所有的数据。
 *
*/

#include<pic18f25k22.h>
#include "mbcrc.h"
#include<htc.h>
#include"modbus_rtu.h"
#include"modbus_rtu_config.h"

#define REC_INTERRUPT_FLG ((RC2IE)&&(RC2IF))
#define TXD_INTERRUPT_FLG (TX2IE&&TX2IF)

#define REC_BUFFER (RC2REG)
#define TXD_BUFFER (TX2REG)
#define TXD_INTERRUPT_EN (TX2IE)

TX_BUF tx_buf;

RX_BUF rx_buf;

RX_STATE rx_state = RX_STATE_INIT;

unsigned int modbus_reg[MODBUS_REG_NUM];

#if DEBUG
void main(void)
{
    unsigned int md_keep;

    ANSELC = 0;
    TRISC = 0x00;//输出
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
#endif

/*
 * 接收中断函数中调用
 * 该函数可以直接移植到其他系统
 */
void rec_isr(void)
{
    unsigned char rec_data = 0;
    if(REC_INTERRUPT_FLG)
    {
       rec_data = REC_BUFFER;
         switch(rx_state)
        {
            case RX_STATE_INIT:
                    restart_timer35();//重启3.5字符定时器
                    rx_buf.rec_count = 0;
                    rx_buf.frame_ok = 0;
            break;
            case RX_STATE_IDLE:
                    rx_buf.data_buf[rx_buf.rec_count++] = rec_data;
                    rx_state = RX_STATE_RECEIVE;
                    restart_timer35();//重启3.5字符定时器
            break;
            case RX_STATE_RECEIVE:
                    if(rx_buf.rec_count<250)//接收字节数不能超过250
                    {
                    rx_buf.data_buf[rx_buf.rec_count++] = rec_data;
                    restart_timer35();//重启3.5字符定时器
                    }
                    else
                    {
                        rx_state = RX_STATE_INIT;
                    }
            break;
            default:
            break;
        }
    }
}
/*
 * 属于底层函数，与单片机相关
 */
void timer35_isr(void)
{
        if(TMR1IF&&TMR1IE)
        {
           // TMR1 = 65535-40320;
            switch(rx_state)
            {
                    case RX_STATE_INIT:
                            rx_state = RX_STATE_IDLE;
                    break;
                    case RX_STATE_RECEIVE://接收到一帧数据
                            rx_state = RX_STATE_IDLE;//继续准备接收下一帧数据
                            rx_buf.frame_ok = 1;//缓冲区数据有效 供应用程序查询使用
                    break;
                    default:
                    break;
            }
            disable_timer35();//禁止3.5字符定时器

             TMR1IF=0;//清除中断标志
        }


}
/*
 * 属于底层函数，与单片机相关
 */
void tx_isr(void)
{
	static unsigned char current_need_send = 0;
	
	if(TXD_INTERRUPT_FLG)
	{
		if(tx_buf.tx_count>0)
		{
			TXD_BUFFER  = tx_buf.data_buf[current_need_send++];
			if(current_need_send==tx_buf.tx_count)
			{
				TXD_INTERRUPT_EN = 0;//禁止发送中断
				current_need_send = 0;
				tx_buf.tx_count = 0;
				rx_buf.rec_count = 0;
				//切换到接收数据状态
			}
		}
	}
}

/*
 * 根据实际波特率设置3.5字符时间 定时器1 被占用
 * 属于底层函数，与单片机相关
 */
void init_timer35(void)
{
  TMR1ON = 0;
  TMR1CS0 = 1;
  TMR1CS1 = 0;//FOSO
  T1CKPS0 = 0;
  T1CKPS1 = 0;
  
  TMR1 = 65535-40320;
  TMR1IF=0;			/* Clear overflow flag*/
  TMR1IE=1;			/* Enable TIMER0 interrupts */
  restart_timer35();
  //TMR1ON = 1;
}

/*
 * 属于底层函数，与单片机相关
 */
void init_uart(void)
{
		SPEN2 = 1;/*使能串口（将RX和TX引脚配置为串口引脚）*/
		SYNC2 = 0;/*异步模式*/
		SPBRG2 = 11059200/64*(1*3+1)/9600-1;/*波特率寄存器置值，设置波特率*/
		BRGH2  = 1;/*速度模式：高速*/
		CREN2  = 1;/*接收使能*/
		TXEN2  = 1;/*发送使能*/
		RC2IE = 1;//使能接收中断	
}

/*
 * 属于底层函数，与单片机相关
 */
void restart_timer35(void)
{
      TMR1ON = 0;
      TMR1 = 65535-40320;
      TMR1IF=0;			/* Clear overflow flag*/
      TMR1IE=1;			/* Enable TIMER0 interrupts */
      TMR1ON = 1;
}

/*
 * 属于底层函数，与单片机相关
 */
void disable_timer35(void)
{
    TMR1ON = 0;
}

void modbus_receive(void)
{
	if(rx_buf.frame_ok==1)//接收到一帧数据
	{
		rx_buf.frame_ok = 0;	
		
		if(modbus_crc16(rx_buf.data_buf,rx_buf.rec_count)==0)//CRC校验成功
		{
			if((ADDRESS_SLAVE ==rx_buf.data_buf[0])||(ADDRESS_BROADCAST==rx_buf.data_buf[0]))//判断接收到的是否是本机地址
			{
				rx_buf.data_valid = 1;
			}
			else
			{
				rx_buf.rec_count = 0;
			}
		}
		else//校验失败
		{
			rx_buf.rec_count = 0;
		}
	}
	
	if(rx_buf.data_valid)//接收到有效数据 开始解码数据
	{
                rx_buf.data_valid = 0;
		switch(rx_buf.data_buf[1])
		{
			case 0x03://读一个或多个寄存器
				read_holding_regs();
			break;
			case 0x06://写单保持寄存器
				write_single_holding_reg();
			break;
			case 0x16://写多个连续的保持寄存器
				//构造返回数据	
			break;
			default://暂不支持其他功能码
				rx_buf.data_valid = 0;
				rx_buf.rec_count = 0;
				rx_buf.frame_ok = 0;			
			break;
		}
	}
}

void read_holding_regs(void)
{
	unsigned int num = 0;//读取寄存器的数量
	unsigned int start_addr;//读取寄存器的数量
	unsigned int data_crc=0;//读取寄存器的数量
	unsigned char i = 0 ;
	
	start_addr = ((rx_buf.data_buf[2])<<8)|(rx_buf.data_buf[3]);
	
	num = ((rx_buf.data_buf[4])<<8)|(rx_buf.data_buf[5]);
        
	tx_buf.data_buf[0] = ADDRESS_SLAVE;//保存从机地址
	tx_buf.data_buf[1] = 0x03;
	tx_buf.data_buf[2] = 2*num;

	for(i=0;i<num;i++)
	{
		tx_buf.data_buf[3+2*i] = modbus_reg[i+start_addr]>>8;
		tx_buf.data_buf[3+2*i+1] = modbus_reg[i+start_addr];
	}
	data_crc = modbus_crc16(tx_buf.data_buf,(2*num+3));

	tx_buf.data_buf[3+2*num] = data_crc;
	tx_buf.data_buf[4+2*num] = data_crc>>8;
	tx_buf.tx_count = 5+2*num;
	//发送数据
	TXD_INTERRUPT_EN = 1;//发送数据	
}

void write_single_holding_reg(void)
{
	unsigned int start_addr;//读取寄存器的数量
	unsigned char i;

	start_addr = ((rx_buf.data_buf[2])<<8)|(rx_buf.data_buf[3]);

	modbus_reg[start_addr] = (rx_buf.data_buf[5])|((rx_buf.data_buf[4])<<8);
        
	for(i=0;i<rx_buf.rec_count;i++)
	{
		tx_buf.data_buf[i] = rx_buf.data_buf[i];
	}
	tx_buf.tx_count = rx_buf.rec_count;
	//转到发送数据状态
	//发送数据
	TXD_INTERRUPT_EN = 1;//发送数据
}

void interrupt my_isr(void)
{
    rec_isr();
    tx_isr();
    timer35_isr();
}




