#ifndef USART_H
#define USART_H
#include<iostream>
#include<fstream> 
#include<vector>
#include<conio.h>
#include<time.h>
#include<Windows.h>
class Comm
{
public:
	HANDLE			hCom;
	LPCSTR			PortX;
	struct			Package
	{
		unsigned char Head_Byte = 0x5B;
		unsigned char Control_Byte = 0x00;
		unsigned char PAYLOAD_DATA[64];
		unsigned char Checksum;
		unsigned char Tail_Byte = 0x5D;
	};
	bool			bInitPort(LPCSTR PortX);
	BYTE			Send_Byte(unsigned char DAT_8bit);
	bool			bSend_Package();
	BYTE			u8Recive_Byte(unsigned char DAT_8bit);
	bool			stop;
private:

};

#endif UART_H