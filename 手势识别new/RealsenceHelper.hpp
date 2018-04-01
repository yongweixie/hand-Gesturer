#pragma once
#include<opencv2\opencv.hpp>
#include<iostream>
#include<Windows.h>
using namespace std;
using namespace cv;
/*****************************************************
@ KillDCM 只针对SR300，因为其他的很垃圾用不到
*使用思路：
如果没有找到相机，排除供电因素，应该是服务卡死了
*使用规范：
尝试访问相机，如果找不到则重启一次服务
如果还找不到就说明硬件问题或者驱动问题
*****************************************************/
DWORD KillDCM();
Mat getMat(PXCImage* PXCImageInterface);
Mat getMat(PXCImage* PXCImageInterface, Size size, PXCImage::PixelFormat pixformat, int format);
int getMatType(PXCImage::PixelFormat fmt);
int getPXCImageFormat(int MatType);

DWORD KillDCM()
{
	cout << "正在重启DCM[";
	SHELLEXECUTEINFO sei = { sizeof(SHELLEXECUTEINFO) };
	sei.fMask = SEE_MASK_NOCLOSEPROCESS;
	sei.lpVerb = TEXT("runas");
	sei.lpFile = TEXT("cmd.exe");
	sei.nShow = SW_HIDE;
	// 停止
	sei.lpParameters = TEXT("cmd /c NET STOP RealSenseDCMSR300 & NET START RealSenseDCMSR300");

	if (!ShellExecuteEx(&sei)) {
		if (GetLastError() == ERROR_CANCELLED)
			cout << "访问权限被用户拒绝";
	}
	DWORD dwExitCode;
	GetExitCodeProcess(sei.hProcess, &dwExitCode);
	while (dwExitCode == STILL_ACTIVE) {
		Sleep(800);
		GetExitCodeProcess(sei.hProcess, &dwExitCode);
		cout << "=";
	}
	cout << "]";
	CloseHandle(sei.hProcess);
	dwExitCode ? cout << "访问错误(" << dwExitCode << ")" << endl : cout << "完成" << endl;
	return dwExitCode;
}

Mat getMat(PXCImage* PXCImageInterface)
{
	auto info = PXCImageInterface->QueryInfo();
	PXCImage::ImageData img_dat;
	PXCImageInterface->AcquireAccess(PXCImage::Access::ACCESS_READ, info.format, &img_dat);
	Mat img = Mat(info.height, info.width, getMatType(info.format), (void*)img_dat.planes[0], img_dat.pitches[0] / sizeof(uchar)).clone();
	PXCImageInterface->ReleaseAccess(&img_dat);
	return img;
}
Mat getMat(PXCImage* PXCImageInterface, Size size, PXCImage::PixelFormat pixformat, int format)
{
	PXCImage::ImageData img_dat;
	PXCImageInterface->AcquireAccess(PXCImage::Access::ACCESS_READ, pixformat, &img_dat);
	Mat img = Mat(size.height, size.width, format, (void*)img_dat.planes[0], img_dat.pitches[0] / sizeof(uchar)).clone();
	PXCImageInterface->ReleaseAccess(&img_dat);
	return img;
}
int getMatType(PXCImage::PixelFormat fmt)
{
	switch (fmt)
	{
	case PXCImage::PIXEL_FORMAT_YUY2:
		return CV_8UC3;
	case PXCImage::PIXEL_FORMAT_NV12:
		return CV_8UC3;
	case PXCImage::PIXEL_FORMAT_RGB32:
		return CV_8UC4;
	case PXCImage::PIXEL_FORMAT_RGB24:
		return CV_8UC3;
	case PXCImage::PIXEL_FORMAT_Y8:
		return CV_8UC1;
	case PXCImage::PIXEL_FORMAT_DEPTH:
		return CV_16UC1;
	case PXCImage::PIXEL_FORMAT_DEPTH_RAW:
		return CV_16UC1;
	case PXCImage::PIXEL_FORMAT_DEPTH_F32:
		return CV_32FC1;
	case PXCImage::PIXEL_FORMAT_DEPTH_CONFIDENCE:
		return CV_8UC1;
	case PXCImage::PIXEL_FORMAT_Y16:
		return CV_16UC1;
	case PXCImage::PIXEL_FORMAT_Y8_IR_RELATIVE:
		return CV_8UC1;
	default:
		return CV_8U;
	}
}
int getPXCImageFormat(int MatType)
{
	switch (MatType)
	{
	default:
		break;
	}
}