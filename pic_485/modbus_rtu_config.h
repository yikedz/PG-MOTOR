#ifndef MODBUS_RTU_CONFIG_H_
#define MODBUS_RTU_CONFIG_H_
	//ʹ�ô���2 PIC18F25K22����˫���� RC6--TXD1 RC7--RXD1
	#define USE_UART 1

	#define MODBUS_REG_NUM 100
	#define ADDRESS_SLAVE 1
	#define ADDRESS_BROADCAST 255

        //�Ĵ���ƫ�Ƶ�ַ
	#define HOLDING_START_ADDR 10

#if USE_UART==1
        #define RXD_INTERRUPT_REQ_FLAG ((RC1IE)&&(RC1IF))
	#define CLEAR_RXD_INTERRUPT_REQ_FLAG 
		
        #define TXD_INTERRUPT_REQ_FLAG (TX1IE&&TX1IF)
	#define CLEAR_TXD_INTERRUPT_REQ_FLAG
		
        #define RXD_BUFFER (RC1REG)
        #define TXD_BUFFER (TX1REG)
        #define TXD_INTERRUPT_EN (TX1IE)

        /*ʹ�ܴ��ڣ���RX��TX��������Ϊ�������ţ�*/
        /*�첽ģʽ*/
        /*�����ʼĴ�����ֵ�����ò�����*/
        /*�ٶ�ģʽ������*/
        /*����ʹ��*/
        /*����ʹ��*/
    //SPBRG1 = 11059200/64*(1*3+1)/9600-1;
        #define init_uart() \
                        SPEN1 = 1;\
                        SYNC1 = 0;\
                        SPBRG1 = 110592/(96*16)-1;\
                        BRGH1  = 0;\
                        CREN1  = 1;\
                        TXEN1  = 1;\
                        RC1IE = 1
#elif USE_UART==2
        #define RXD_INTERRUPT_REQ_FLAG ((RC2IE)&&(RC2IF))
	#define CLEAR_RXD_INTERRUPT_REQ_FLAG

        #define TXD_INTERRUPT_REQ_FLAG (TX2IE&&TX2IF)
	#define CLEAR_TXD_INTERRUPT_REQ_FLAG

        #define RXD_BUFFER (RC2REG)
        #define TXD_BUFFER (TX2REG)
        #define TXD_INTERRUPT_EN (TX2IE)


        /*ʹ�ܴ��ڣ���RX��TX��������Ϊ�������ţ�*/
        /*�첽ģʽ*/
        /*�����ʼĴ�����ֵ�����ò�����*/
        /*�ٶ�ģʽ������*/
        /*����ʹ��*/
        /*����ʹ��*/
        #define init_uart() \
                        SPEN2 = 1;\
                        SYNC2 = 0;\
                        SPBRG2 = 11059200/64*(1*3+1)/9600-1;\
                        BRGH2  = 1;\
                        CREN2  = 1;\
                        TXEN2  = 1;\
                        RC2IE = 1
    
#endif
		
	#define TIMER1_INTERRUPT_REQ_FLAG (TMR1IF&&TMR1IE)
	#define CLEAR_TIMER1_INTERRUPT_REQ_FLAG	TMR1IF=0;

        #define init_timer35() \
                          TMR1ON = 0;\
                          TMR1CS0 = 0;\
                          TMR1CS1 = 0;\
                          T1CKPS0 = 0;\
                          T1CKPS1 = 0;\
                          TMR1 = 65535-40320;\
                          TMR1IF=0;\
                          TMR1IE=1;\
                          TMR1ON = 1
        #define  restart_timer35()              \
				  TMR1ON = 0;			\
				  TMR1 = 65535-40320;               \
				  TMR1IF=0;				\
				  TMR1IE=1;				\
				  TMR1ON = 1

        #define disable_timer35() TMR1ON = 0
#endif