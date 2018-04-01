#pragma once
#include<pxchandconfiguration.h>
#include<pxchandmodule.h>
#include<pxchanddata.h>
#include<pxcprojection.h>
#include<pxcpowerstate.h>
#include<utilities\pxcsmoother.h>

#include<opencv2/opencv.hpp>
#include<Windows.h>

using namespace std;

class Hand3D
{
public:
	bool Init() {


		sr300_manager = PXCSenseManager::CreateInstance();
		sr300_manager->EnableStream(PXCCapture::STREAM_TYPE_COLOR, 960, 540, 60);
		sr300_manager->EnableStream(PXCCapture::STREAM_TYPE_DEPTH, 640, 480, 60);
		sr300_manager->EnableStream(PXCCapture::STREAM_TYPE_IR, 640, 480, 60);
		sr300_manager->EnableHand();

		handModule = sr300_manager->QueryHand();

		PXCHandConfiguration* handConfig = handModule->CreateActiveConfiguration();
		handConfig->SetTrackingMode(PXCHandData::TrackingModeType::TRACKING_MODE_FULL_HAND);
		handConfig->EnableStabilizer(true);
		handConfig->EnableTrackedJoints(true);
		handConfig->EnableNormalizedJoints(true);
		handConfig->EnableSegmentationImage(true);
		handConfig->ApplyChanges();

		handData = handModule->CreateOutput();

		sr300_manager->QuerySession()->CreateImpl<PXCSmoother>(&smooth);
		for (size_t i = 0; i < 10; i++) {
			smoother[i] = smooth->Create1DSpring();
			smoother[i]->SmoothValue(0.1);
		}
		for (size_t i = 0; i < 8; i++) {
			smoother_rect[i] = smooth->Create1DSpring(0.3);
			smoother[i]->SmoothValue(0.1);
		}
		auto device = Seek();


		projection = device->CreateProjection();
		sr300_manager->Init();

		//auto device = sr300_manager->QuerySession()->CreateCaptureManager()->QueryDevice();
		//auto device = sr300_manager->QueryCaptureManager()->QueryDevice();

		yy = 480 - 5 * hist_w;
		xx = 640 - 5 * hist_w;
		indicator = cv::Mat::zeros(cv::Size(5 * hist_w, 5 * hist_w), CV_8UC3);
		canvas = cv::Mat::zeros(cv::Size(640, 480), CV_8UC3);

		Update();
		//device->SetDeviceAllowProfileChange(true);
		device->SetIVCAMLaserPower(16);
		cout << device->SetDepthConfidenceThreshold(5);

		cout << device->SetIVCAMMotionRangeTradeOff(5);

		cout << device->SetIVCAMFilterOption(0);
		cout << device->SetIVCAMAccuracy(PXCCapture::Device::IVCAM_ACCURACY_FINEST);
		sr300_manager->QuerySession()->CreatePowerManager()->SetState(PXCPowerState::STATE_PERFORMANCE);
		return true;
	}
	int Update() {
		// 清零
		for (size_t i = 0; i < 3; i++) {
			ihand[i] = NULL;
			handId[i] = 0;
		}
		sr300_manager->ReleaseFrame();
		// 更新
		if (sr300_manager->AcquireFrame(true) < PXC_STATUS_NO_ERROR) return -1;
		sample = sr300_manager->QuerySample();
		depth = sample->depth;
		color = sample->color;
		handData->Update();
		nHands = handData->QueryNumberOfHands();
		for (size_t i = 0; i < nHands; i++) {
			auto ret = handData->QueryHandId(PXCHandData::ACCESS_ORDER_NEAR_TO_FAR, i, handId[PXCHandData::BodySideType::BODY_SIDE_UNKNOWN]);
			ret = handData->QueryHandDataById(handId[PXCHandData::BodySideType::BODY_SIDE_UNKNOWN], ihand[PXCHandData::BodySideType::BODY_SIDE_UNKNOWN]);
			auto side = ihand[PXCHandData::BodySideType::BODY_SIDE_UNKNOWN]->QueryBodySide();
			ihand[side] = ihand[PXCHandData::BodySideType::BODY_SIDE_UNKNOWN];
			handId[side] = handId[PXCHandData::BodySideType::BODY_SIDE_UNKNOWN];
		}
		colorSight = QueryColorImage();
		return nHands;
	}

	cv::Mat QueryColorImage() {
		auto pxcimg = projection->CreateColorImageMappedToDepth(depth, color);
		auto info = pxcimg->QueryInfo();
		PXCImage::ImageData img_dat;
		pxcimg->AcquireAccess(PXCImage::Access::ACCESS_READ, PXCImage::PIXEL_FORMAT_RGB24, &img_dat);
		cv::Mat img = cv::Mat(info.height, info.width, CV_8UC3, (void*)img_dat.planes[0], img_dat.pitches[0] / sizeof(uchar)).clone();
		pxcimg->ReleaseAccess(&img_dat);
		pxcimg->Release();
		flip(img, img, 1);
		return img + QueryIrImage();
	}
	cv::Mat QueryIrImage() {
		auto pxcimg = sample->ir;
		auto info = pxcimg->QueryInfo();
		PXCImage::ImageData img_dat;
		pxcimg->AcquireAccess(PXCImage::Access::ACCESS_READ, PXCImage::PIXEL_FORMAT_Y8_IR_RELATIVE, &img_dat);
		cv::Mat img = cv::Mat(info.height, info.width, CV_8UC1, (void*)img_dat.planes[0], img_dat.pitches[0] / sizeof(uchar));
		//	cv::normalize(img,img,0,255)
		cv::cvtColor(img, img, cv::COLOR_GRAY2BGR);
		pxcimg->ReleaseAccess(&img_dat);
		flip(img*1.5, img, 1);
		return img;
	}
	cv::Mat QuerySegmentedMask() {///xxx
		PXCImage::ImageData img_dat;
		color->AcquireAccess(PXCImage::Access::ACCESS_READ, PXCImage::PIXEL_FORMAT_Y8, &img_dat);
		cv::Mat img = cv::Mat(360, 640, CV_8UC1, (void*)img_dat.planes[0], img_dat.pitches[0] / sizeof(uchar)).clone();
		color->ReleaseAccess(&img_dat);
	}

	cv::Mat drawIndicator() {
		canvas.setTo(0);
		draw(ihand[PXCHandData::BodySideType::BODY_SIDE_LEFT]);
		draw(ihand[PXCHandData::BodySideType::BODY_SIDE_RIGHT]);

		//for (size_t i = 0; i < nHands; i++)
		//{
		//	pxcUID handId;
		//	auto ret = handData->QueryHandId(PXCHandData::ACCESS_ORDER_NEAR_TO_FAR, i, handId);
		//	//cout << handId << endl;
		//	handData->QueryHandDataById(handId, ihand[PXCHandData::BodySideType::BODY_SIDE_UNKNOWN]);
		//	// 画框
		//	auto rect = ihand[PXCHandData::BodySideType::BODY_SIDE_UNKNOWN]->QueryBoundingBoxImage();
		//	rect.x = 640 - rect.x - rect.w; 
		//	auto yy = 480 - 5 * hist_w;
		//	cout << rect.y << endl;
		//	if (rect.y > yy) rect.y = yy;
		//	// 指示器
		//	cv::Mat ind = canvas(cv::Rect(rect.x, rect.y, 5 * hist_w, 5 * hist_w));
		//	double *foldedness = QueryFingerFoldedness(handId);
		//	for (size_t i = 0; i < 5; i++) {
		//		auto h = (int)(foldedness[i] * indicator.rows);
		//		rectangle(ind, cv::Rect(i*hist_w, indicator.rows - h, hist_w, h), cv::Scalar(255, 255, 255), -1);
		//	}
		//	if (ihand[PXCHandData::BodySideType::BODY_SIDE_UNKNOWN]->QueryBodySide() == PXCHandData::BodySideType::BODY_SIDE_LEFT)flip(ind, ind, 1);
		//	rectangle(canvas, cv::Rect(rect.x, rect.y, rect.w, rect.h), cv::Scalar(0, 255, ihand[PXCHandData::BodySideType::BODY_SIDE_UNKNOWN]->IsCalibrated() ? 0 : 255), 2);
		//	// string显示框
		//}
		return canvas + colorSight;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	PXCHandData::IHand *QueryHandRight() {
		return ihand[PXCHandData::BodySideType::BODY_SIDE_RIGHT];
	}
	PXCHandData::IHand *QueryHandLeft() {
		return ihand[PXCHandData::BodySideType::BODY_SIDE_LEFT];
	}
	PXCHandData::IHand *QueryHandUnknown() {
		return ihand[PXCHandData::BodySideType::BODY_SIDE_UNKNOWN];
	}
	//////////////////////////////////////////////////////////////////////////////////////
	//cv::Rect projectRect(PXCRectI32 rect,float z)
	//{
	//	PXCPointF32 p2[2];
	//	PXCPoint3DF32 p3[2];
	//	p3[0].x = rect.x;
	//	p3[0].y = rect.y;
	//	p3[1].x = p3[0].x+rect.w;
	//	p3[1].y = p3[0].y + rect.h;
	//	p3[0].z = p3[1].z = z;
	//	auto  cal = projection->MapDepthToColor(2,p3,p2);
	//	return cv::Rect(cv::Point(p2[0].x, p2[0].y), cv::Point(p2[1].x, p2[1].y));
	//}
	// 边界框
	cv::Rect QueryHandBoundingBox(PXCHandData::IHand *ihand) {
		if (ihand == NULL) return cv::Rect(0, 0, 0, 0);
		auto rect = ihand->QueryBoundingBoxImage();
		rect.x = 640 - rect.x - rect.w;

		return cv::Rect(rect.x, rect.y, rect.w, rect.h);
	}
	void draw(PXCHandData::IHand *ihand) {

		if (ihand == NULL) return;
		// 框

		auto offset = (ihand->QueryBodySide() == PXCHandData::BodySideType::BODY_SIDE_LEFT) ? 4 : 0;;
		auto rect = ihand->QueryBoundingBoxImage();
		rect.x = 640 - rect.x - rect.w;
		if (rect.y > yy) rect.y = yy;
		if (rect.x > xx) rect.x = xx;
		//if (rect.x > xx) rect.x = xx;
		auto x = smoother_rect[offset + 0]->SmoothValue(rect.x);
		auto y = smoother_rect[offset + 1]->SmoothValue(rect.y);
		auto w = smoother_rect[offset + 2]->SmoothValue(rect.w);
		auto h = smoother_rect[offset + 3]->SmoothValue(rect.h);

		auto box = cv::Rect(x, y, w, h);

		auto iscalib = ihand->IsCalibrated();
		if (iscalib) {
			// 指示器
			cv::Rect indBox = cv::Rect(x, y + h - 5 * hist_w, 5 * hist_w, 5 * hist_w);
			cv::Mat ind;
			colorSight(indBox) /= 2;
			ind = canvas(indBox);

			double *foldedness = QueryFingerFoldedness(ihand);
			for (size_t i = 0; i < 5; i++) {
				auto h = (int)(foldedness[i] * indicator.rows);
				rectangle(ind, cv::Rect(i*hist_w, indicator.rows - h, hist_w, h), cv::Scalar(255 * foldedness[i], 255, 255 - 200 * foldedness[i]), -1);
			}
			if (ihand->QueryBodySide() == PXCHandData::BodySideType::BODY_SIDE_LEFT)flip(ind, ind, 1);
		}
		rectangle(canvas, box, cv::Scalar(iscalib ? 255 : 0, 127, iscalib ? 0 : 255), iscalib ? 2 : 15);
		//cout << box << endl;
	}
	// 弯曲度指示器
	cv::Mat QueryIndicator(PXCHandData::IHand *ihand) {
		indicator.setTo(0);
		if (ihand == NULL) return indicator;
		double *foldedness = QueryFingerFoldedness(ihand);
		for (size_t i = 0; i < 5; i++) {
			auto h = (int)(foldedness[i] * indicator.rows);
			rectangle(indicator, cv::Rect(i*hist_w, indicator.rows - h, hist_w, h), cv::Scalar(255, 255, 255), -1);
		}
		if (ihand == NULL) return indicator;
	}

	double* QueryFingerFoldedness(PXCHandData::IHand *ihand) {
		if (ihand == NULL) {
			for (size_t i = 0; i < 5; i++)
				finger_foldedness[i] = -1;
			return finger_foldedness;
		}
		int offset = ihand->QueryBodySide() == PXCHandData::BodySideType::BODY_SIDE_RIGHT ? 5 : 0;
		for (size_t i = 0; i < 5; i++) {
			ihand->QueryFingerData((PXCHandData::FingerType)i, finger[i]);
			finger_foldedness[i] = smoother[i + offset]->SmoothValue(finger[i].foldedness) / 100.0f;
			//cout << finger_foldedness[i] << endl;
		}
		return finger_foldedness;
	}
	cv::Point3f QueryMassCenterWorld(PXCHandData::IHand *ihand) {
		if (ihand == NULL)return cv::Point3f();
		auto worldPos = ihand->QueryMassCenterWorld();
		return cv::Point3f(worldPos.x, worldPos.y, worldPos.z);
	}

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
	PXCCapture::Device* Seek()
	{
		PXCCapture *Capture;
		uint_fast8_t idx = 0, i;
		for (;; idx += 1)
		{
			PXCSession::ImplDesc Impl = {};
			Impl.group = PXCSession::IMPL_GROUP_SENSOR;
			Impl.subgroup = PXCSession::IMPL_SUBGROUP_VIDEO_CAPTURE;
			auto Session = PXCSession::CreateInstance();
			auto retStatus = Session->QueryImpl(&Impl, idx, &Impl);
			if (retStatus < PXC_STATUS_NO_ERROR)    continue;
			cout << "--------------------------------------------------------------------------------------" << endl;
			wcout << "DCM服务:" << Impl.friendlyName;
			cout << " *** IUID是:" << Impl.iuid << endl;
			retStatus = Session->CreateImpl(&Impl, &Capture);
			if (retStatus < PXC_STATUS_NO_ERROR)    continue;
			PXCCapture::DeviceInfo dinfo = { 0 };
			retStatus = Capture->QueryDeviceInfo(idx, &dinfo);
			if (retStatus < PXC_STATUS_NO_ERROR) { cout << "详细信息:获取失败" << endl; break; }
			wcout << "详细信息:" << endl;
			wcout << "  名称:" << dinfo.name << " 位于DCM设备索引 " << dinfo.didx << endl;
			wcout << "  可使用视频流:";
			if (dinfo.streams&PXCCapture::STREAM_TYPE_COLOR) cout << "COLOR ";
			if (dinfo.streams&PXCCapture::STREAM_TYPE_DEPTH) cout << "DEPTH ";
			if (dinfo.streams&PXCCapture::STREAM_TYPE_IR) cout << "IR ";
			if (dinfo.streams&PXCCapture::STREAM_TYPE_LEFT) cout << "LEFT";
			if (dinfo.streams&PXCCapture::STREAM_TYPE_RIGHT) cout << "RIGHT "; wcout << endl;
			wcout << "  固件版本:V" << dinfo.firmware[0] << "." << dinfo.firmware[1] << "." << dinfo.firmware[2] << "." << dinfo.firmware[3] << endl; // 顺便检查一下是否要升级固件 
			wcout << "  序列号:" << dinfo.serial << endl; // 最终确定是哪个摄像机就靠它认了
			wcout << "  设备标识符:" << dinfo.did << endl;// 不嫌蛋疼的也可以拿它认
			cout << "======================================================================================" << endl;
			i = idx;
			break;
			//wcout << "√!找到了匹配的" << Capture->DeviceModelToString(dinfo.model) << " ";
		}
		auto device = Capture->CreateDevice(i);
		if (device == NULL) { cout << "\n没有匹配设备," << (int)i; }
		return device;
	}
private:

	PXCSenseManager* sr300_manager = NULL;
	PXCImage *color, *depth, *segmask = NULL;
	PXCCapture::Sample *sample;

	PXCHandModule *handModule = NULL;
	PXCHandData* handData = NULL;

	PXCSmoother *smooth = NULL;
	PXCSmoother::Smoother1D* smoother[10];
	PXCSmoother::Smoother1D* smoother_rect[8];

	PXCHandData::IHand *ihand[3];
	pxcUID handId[3];
	float handz[3];
	pxcUID current_handId = 0;
	pxcStatus ihand_update_ret = PXC_STATUS_DATA_UNAVAILABLE;
	pxcStatus ihand_update(pxcUID handId) {
		return ihand_update_ret = handData->QueryHandDataById(handId, ihand[0]);
	}

	int hist_w = 20;
	int yy = 480 - 5 * hist_w;
	int xx = 640 - 5 * hist_w;
	cv::Mat indicator;
	PXCHandData::FingerData finger[5];
	double finger_foldedness[5];

	int nHands = 0;
	cv::Mat canvas, colorSight;

	PXCProjection *projection = NULL;
};