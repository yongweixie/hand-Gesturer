#include<Windows.h>
#include"usart.h"
using namespace std;
Comm usart;
int setHand(unsigned char id, UINT16 param, unsigned char speed_level)
{
	/*if (id == 20) {
		if (param > 70)param = 70;
	}*/
	//usart.bInitPort("COM3");
	union param16 { unsigned char data[2];	UINT16 value; } u16;
	u16.value = param;
	union fingers
	{
		unsigned char data[4];
	};
	unsigned char pkg[8];
	pkg[7] = 0;
	pkg[7] += pkg[0] = 0xFF;
	pkg[7] += pkg[1] = 0xFF;
	pkg[7] += pkg[2] = id;
	pkg[7] += pkg[3] = 0x00;
	pkg[7] += pkg[4] = u16.data[0];
	pkg[7] += pkg[5] = u16.data[1];
	pkg[7] += pkg[6] = speed_level;
	WriteFile(usart.hCom, pkg, 8, NULL, NULL);
	Sleep(2);
}
void RandHandset(int n)
{
	cout << n << endl;
	switch (n)
	{
	case 0:
		setHand(0 + 16, 89, 0);
		setHand(1 + 16, 0, 0);
		setHand(2 + 16, 0, 0);
		setHand(3 + 16, 89, 0);
		setHand(4 + 16, 89, 0);
		break;
	case 1:
		setHand(0 + 16, 00, 0);
		setHand(1 + 16, 00, 0);
		setHand(2 + 16, 00, 0);
		setHand(3 + 16, 00, 0);
		setHand(4 + 16, 00, 0);
		break;

	case 2:
		setHand(0 + 16, 89, 0);
		setHand(1 + 16, 89, 0);
		setHand(2 + 16, 89, 0);
		setHand(3 + 16, 89, 0);
		setHand(4 + 16, 89, 0);
		break;
	default:
		break;
	}
}