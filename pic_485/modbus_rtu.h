#ifndef MODBUS_RTU_H_
#define MODBUS_RTU_H_

	#define RX_BUF_SIZE 50
	#define TX_BUF_SIZE 50
	typedef enum
	{
		RX_STATE_INIT,
		RX_STATE_IDLE,
		RX_STATE_RECEIVE,
		RX_STATE_DONE
	}RX_STATE;
	typedef struct
	{
		unsigned char frame_ok;//�������������Ƿ���Ч
		unsigned char data_valid;//�������������Ƿ���Ч
		unsigned char rec_count;//�������������ֽ���
		unsigned char data_buf[RX_BUF_SIZE];//������
	} RX_BUF;
	typedef struct
	{
		//unsigned char frame_need_send;//�������������Ƿ���Ч
		unsigned char tx_count;//�������д����͵��ֽ�����
		unsigned char data_buf[TX_BUF_SIZE];//������
	} TX_BUF;
        
    void init_uart(void);
    void rec_isr(void);
    void tx_isr(void);
    void init_timer35(void);
    void timer35_isr(void);
    void restart_timer35(void);
    void disable_timer35(void);
    void modbus_receive(void);
    void read_holding_regs(void);
    void write_single_holding_reg(void);
#endif