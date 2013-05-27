#ifndef MODBUS_RTU_CONFIG_H_
#define MODBUS_RTU_CONFIG_H_
	//使用串口2 PIC18F25K22具有双串口 RC6--TXD1 RC7--RXD1
	#define USE_UART1 0
	//使用串口2 PIC18F25K22具有双串口 RC6--TXD1 RC7--RXD1
	#define USE_UART2 1



	#define MODBUS_REG_NUM 100
	#define ADDRESS_SLAVE 1
	#define ADDRESS_BROADCAST 255

        //寄存器偏移地址
	#define HOLDING_START_ADDR 10

	#define DEBUG 1
#endif