#ifndef MODBUS_RTU_CONFIG_H_
#define MODBUS_RTU_CONFIG_H_
	//ʹ�ô���2 PIC18F25K22����˫���� RC6--TXD1 RC7--RXD1
	#define USE_UART1 0
	//ʹ�ô���2 PIC18F25K22����˫���� RC6--TXD1 RC7--RXD1
	#define USE_UART2 1



	#define MODBUS_REG_NUM 100
	#define ADDRESS_SLAVE 1
	#define ADDRESS_BROADCAST 255

        //�Ĵ���ƫ�Ƶ�ַ
	#define HOLDING_START_ADDR 10

	#define DEBUG 1
#endif