#include <iostream>
#include <windows.h>
#include <bitset>
#include "USART.h"
#define byte 8
using namespace std;
bool Comm::bSend_Package()
{
	Package ToSend;
}
BYTE Comm::Send_Byte(BYTE DAT_8bit)
{
	BYTE buf;//	ReadFile(hCom, &buf, 1, NULL, NULL);
			 //cout << "发送" << bitset<byte>(DAT_8bit)<<"...";
	if (!WriteFile(hCom, &DAT_8bit, 1, NULL, NULL))
	{
		system("COLOR 0c");
		//cout << "失败" << endl;
		return 0;
	}
	else
		//cout << "成功" << endl;
		return DAT_8bit;
}

bool Comm::bInitPort(LPCSTR sSerialPort)
{
	//尝试打开串口
	hCom = CreateFileA(sSerialPort,		//串口号
		GENERIC_READ | GENERIC_WRITE,	//允许读写
		0,								//通讯设备必须以独占方式打开
		0,								//无安全属性
		OPEN_EXISTING,					//通讯设备已存在
		0,								//同步I/O
		0);								//通讯设备不能用模板打开
	if (hCom == INVALID_HANDLE_VALUE)
	{
		CloseHandle(hCom);
		cout << "INVALID SERIAL PORT:" << sSerialPort << endl;
		return false;
	}
	else
		cout << "SERIAL PORT " << sSerialPort << " IS OPEN" << endl;
	//初始化串口
	DCB dcb;
	dcb.BaudRate = 115200; //波特率为9600
	dcb.ByteSize = 8; //数据位数为8位
	dcb.Parity = NOPARITY; //无校验
	dcb.StopBits = ONESTOPBIT; //1个停止位a
	if (!SetCommState(hCom, &dcb)) //设置DCB
	{
		cout << "SET DCB ERROR" << endl;
		return false;
	}
	else
	{

		dcb.StopBits = ONESTOPBIT; //1个停止位a
		cout << "BAULD RATE = " << dcb.BaudRate << endl;
	}
	if (!SetupComm(hCom, 1024, 1024)) //设置缓冲区
	{
		cout << "SET QUEUE ERROR" << endl;
		return false;
	}
	else
	{
		cout << "OUT QUEUE = " << 1024 << endl;
		cout << "IN QUEUE = " << 1024 << endl;
	}

	COMMTIMEOUTS TimeOuts;
	//设定读超时
	TimeOuts.ReadIntervalTimeout = 1000;
	TimeOuts.ReadTotalTimeoutMultiplier = 500;
	TimeOuts.ReadTotalTimeoutConstant = 5000;
	//设定写超时
	TimeOuts.WriteTotalTimeoutMultiplier = 500;
	TimeOuts.WriteTotalTimeoutConstant = 2000;
	if (!SetCommTimeouts(hCom, &TimeOuts)) //设置超时
	{
		cout << "SET TIMEOUT ERROR" << endl;
		return false;
	}
	else
	{
		cout << "ReadIntervalTimeout = " << TimeOuts.ReadIntervalTimeout << "ms" << endl;
		cout << "ReadTotalTimeoutMultiplier = " << TimeOuts.ReadTotalTimeoutMultiplier << "X" << endl;
		cout << "ReadTotalTimeoutConstant = " << TimeOuts.ReadTotalTimeoutConstant << "ms" << endl;
		cout << "WriteTotalTimeoutMultiplier = " << TimeOuts.WriteTotalTimeoutMultiplier << "X" << endl;
		cout << "WriteTotalTimeoutConstant = " << TimeOuts.WriteTotalTimeoutConstant << "ms" << endl;
	}
	if (!PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR))
	{
		cout << "PURGE ERROR FAILED" << endl;
		return false;
	}
	else
	{
		cout << "PURGE ERROR SUCCESS" << endl;
		cout << "INIT SUCCESS" << endl;
		cout << "-----------------------------------------------------" << endl;
		return true;
	}
}
