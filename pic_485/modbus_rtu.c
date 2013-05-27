/*
 *
 *   ��Ƭ���źţ�PIC18F25K22
 *   ����������Mplab X V1.7
 *   ��������HI-TECH-PICC18(V9.66)
 *
 * �����жϽ������ݣ�ʹ���жϷ�������
 * rec_isr(���жϱ�֤���ܵ�һ֡���ݣ���־λ��λ��-->
 * modbus_receive��ѯ��־λ�������֡����-->
 * ��������Ҫ����ʱ��ʹ�ܷ����жϣ���Ƭ�����Զ��������е����ݡ�
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
    TRISC = 0x00;//���
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
 * �����жϺ����е���
 * �ú�������ֱ����ֲ������ϵͳ
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
                    restart_timer35();//����3.5�ַ���ʱ��
                    rx_buf.rec_count = 0;
                    rx_buf.frame_ok = 0;
            break;
            case RX_STATE_IDLE:
                    rx_buf.data_buf[rx_buf.rec_count++] = rec_data;
                    rx_state = RX_STATE_RECEIVE;
                    restart_timer35();//����3.5�ַ���ʱ��
            break;
            case RX_STATE_RECEIVE:
                    if(rx_buf.rec_count<250)//�����ֽ������ܳ���250
                    {
                    rx_buf.data_buf[rx_buf.rec_count++] = rec_data;
                    restart_timer35();//����3.5�ַ���ʱ��
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
 * ���ڵײ㺯�����뵥Ƭ�����
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
                    case RX_STATE_RECEIVE://���յ�һ֡����
                            rx_state = RX_STATE_IDLE;//����׼��������һ֡����
                            rx_buf.frame_ok = 1;//������������Ч ��Ӧ�ó����ѯʹ��
                    break;
                    default:
                    break;
            }
            disable_timer35();//��ֹ3.5�ַ���ʱ��

             TMR1IF=0;//����жϱ�־
        }


}
/*
 * ���ڵײ㺯�����뵥Ƭ�����
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
				TXD_INTERRUPT_EN = 0;//��ֹ�����ж�
				current_need_send = 0;
				tx_buf.tx_count = 0;
				rx_buf.rec_count = 0;
				//�л�����������״̬
			}
		}
	}
}

/*
 * ����ʵ�ʲ���������3.5�ַ�ʱ�� ��ʱ��1 ��ռ��
 * ���ڵײ㺯�����뵥Ƭ�����
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
 * ���ڵײ㺯�����뵥Ƭ�����
 */
void init_uart(void)
{
		SPEN2 = 1;/*ʹ�ܴ��ڣ���RX��TX��������Ϊ�������ţ�*/
		SYNC2 = 0;/*�첽ģʽ*/
		SPBRG2 = 11059200/64*(1*3+1)/9600-1;/*�����ʼĴ�����ֵ�����ò�����*/
		BRGH2  = 1;/*�ٶ�ģʽ������*/
		CREN2  = 1;/*����ʹ��*/
		TXEN2  = 1;/*����ʹ��*/
		RC2IE = 1;//ʹ�ܽ����ж�	
}

/*
 * ���ڵײ㺯�����뵥Ƭ�����
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
 * ���ڵײ㺯�����뵥Ƭ�����
 */
void disable_timer35(void)
{
    TMR1ON = 0;
}

void modbus_receive(void)
{
	if(rx_buf.frame_ok==1)//���յ�һ֡����
	{
		rx_buf.frame_ok = 0;	
		
		if(modbus_crc16(rx_buf.data_buf,rx_buf.rec_count)==0)//CRCУ��ɹ�
		{
			if((ADDRESS_SLAVE ==rx_buf.data_buf[0])||(ADDRESS_BROADCAST==rx_buf.data_buf[0]))//�жϽ��յ����Ƿ��Ǳ�����ַ
			{
				rx_buf.data_valid = 1;
			}
			else
			{
				rx_buf.rec_count = 0;
			}
		}
		else//У��ʧ��
		{
			rx_buf.rec_count = 0;
		}
	}
	
	if(rx_buf.data_valid)//���յ���Ч���� ��ʼ��������
	{
                rx_buf.data_valid = 0;
		switch(rx_buf.data_buf[1])
		{
			case 0x03://��һ�������Ĵ���
				read_holding_regs();
			break;
			case 0x06://д�����ּĴ���
				write_single_holding_reg();
			break;
			case 0x16://д��������ı��ּĴ���
				//���췵������	
			break;
			default://�ݲ�֧������������
				rx_buf.data_valid = 0;
				rx_buf.rec_count = 0;
				rx_buf.frame_ok = 0;			
			break;
		}
	}
}

void read_holding_regs(void)
{
	unsigned int num = 0;//��ȡ�Ĵ���������
	unsigned int start_addr;//��ȡ�Ĵ���������
	unsigned int data_crc=0;//��ȡ�Ĵ���������
	unsigned char i = 0 ;
	
	start_addr = ((rx_buf.data_buf[2])<<8)|(rx_buf.data_buf[3]);
	
	num = ((rx_buf.data_buf[4])<<8)|(rx_buf.data_buf[5]);
        
	tx_buf.data_buf[0] = ADDRESS_SLAVE;//����ӻ���ַ
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
	//��������
	TXD_INTERRUPT_EN = 1;//��������	
}

void write_single_holding_reg(void)
{
	unsigned int start_addr;//��ȡ�Ĵ���������
	unsigned char i;

	start_addr = ((rx_buf.data_buf[2])<<8)|(rx_buf.data_buf[3]);

	modbus_reg[start_addr] = (rx_buf.data_buf[5])|((rx_buf.data_buf[4])<<8);
        
	for(i=0;i<rx_buf.rec_count;i++)
	{
		tx_buf.data_buf[i] = rx_buf.data_buf[i];
	}
	tx_buf.tx_count = rx_buf.rec_count;
	//ת����������״̬
	//��������
	TXD_INTERRUPT_EN = 1;//��������
}

void interrupt my_isr(void)
{
    rec_isr();
    tx_isr();
    timer35_isr();
}




