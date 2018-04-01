#include"Hand3D.hpp"
#include<Windows.h>
#include<utilities\pxcsmoother.h>
using namespace cv;
#define devView(i) imshow(#i,i);
#include"HandSet.hpp"
#include"RingBuffer.hpp"
ringBuffer<Point3f> handTrack(5);
int random[18] = { 0, 2, 1, 2, 0, 1, 2, 0, 1, 0, 2, 0, 1, 2, 0, 1, 2, };
void alwayswin(Hand3D &hand, double *foldedness);
void randomwin(Hand3D &hand, double *foldedness);
void num_dectrtion(Hand3D &hand, double *foldedness, Mat show);
int main()
{
	setiosflags(ios::fixed);
	setprecision(1);
	PXCSmoother *smooth = NULL;
	PXCSmoother::Smoother1D* smoother;
	PXCSenseManager::CreateInstance()->QuerySession()->CreateImpl<PXCSmoother>(&smooth);
	smoother = smooth->Create1DQuadratic(1);
	//usart.bInitPort("COM3");
	Hand3D hand;
	hand.KillDCM();
	hand.Init();
	LARGE_INTEGER t0, t, freq;
	for (; (waitKey(1)) != 27; hand.Update())
	{


		Mat show = hand.drawIndicator();
		auto r = hand.QueryHandRight();
		double *foldedness = hand.QueryFingerFoldedness(r);
		//randomwin(hand, foldedness);
		//alwayswin(hand, foldedness);
		num_dectrtion(hand, foldedness, show);
		/*for (size_t i = 0; i < 5; i++)
		{
			setHand(i + 16, (1-foldedness[i])*89, 1);
		}*/
		QueryPerformanceCounter(&t);
		QueryPerformanceFrequency(&freq);
		double fps = freq.QuadPart/ ((double)t.QuadPart - t0.QuadPart) ;
		putText(show, "fps:" + (std::to_string(smoother->SmoothValue(fps))).substr(0,4), Point(50, 50), cv::HersheyFonts::FONT_HERSHEY_COMPLEX, 1, Scalar(255, 255, 255), 2);
		resize(show, show, Size(1024, 768));
		devView(show);
		QueryPerformanceCounter(&t0);
	}
}
void alwayswin(Hand3D &hand, double *foldedness) {
	int nfold = 0;
	for (size_t i = 0; i < 5; i++)
	{
		nfold += foldedness[i] < 0.4 ? 1 : 0;

	}
		if (nfold <=2)
		{
			setHand(16, 89, 1);
			setHand(17, 0, 1);
			setHand(18, 0, 1);
			setHand(19, 89, 1); 
			setHand(20, 89, 1);
		}
		else
		{
			if (nfold>=4)
			{
				setHand(16, 0, 1);
				setHand(17, 0, 1);
				setHand(18, 0, 1);
				setHand(19, 0, 1);
				setHand(20, 0, 1);
			}
			else
			{
				setHand(16, 89, 1);
				setHand(17, 89, 1);
				setHand(18, 89, 1);
				setHand(19, 89, 1);
				setHand(20, 89, 1);
			}
		//}
	//	cout << "³öÈ­" << endl;

	}
}
void randomwin(Hand3D &hand, double *foldedness) {
	auto worldPos = hand.QueryMassCenterWorld(hand.QueryHandRight());
	handTrack.push_back(worldPos);
	if (handTrack[handTrack.size() - 1].z - handTrack[0].z < -0.02
		&&handTrack[handTrack.size() - 1].y - handTrack[0].y < -0.02)
	{
		//cout << "³öÈ­" << endl;
		RandHandset(random[rand() % 18]);
		handTrack.clear();

	}
}
void num_dectrtion(Hand3D &hand, double *foldedness, Mat show)
{
	int nfold = 0;
	for (size_t i = 0; i < 5; i++)
	{
		nfold += foldedness[i] > 0.8 ? 1 : 0;

	}
	if (nfold == 0)
	{
		putText(show, "mean: no information", Point(50, 460), cv::HersheyFonts::FONT_HERSHEY_COMPLEX, 1, Scalar(255, 255, 255), 3.5);

	}
	else if (nfold==1&& foldedness[1]>0.8)
	{
		putText(show, "mean:" + (std::to_string(1)).substr(0, 4), Point(50, 460), cv::HersheyFonts::FONT_HERSHEY_COMPLEX, 1, Scalar(255, 255, 255), 3.5);

	}
	else if (nfold ==2&& foldedness[2]>0.8)
	{
		putText(show, "mean:" + (std::to_string(2)).substr(0, 4), Point(50, 460), cv::HersheyFonts::FONT_HERSHEY_COMPLEX, 1, Scalar(255, 255, 255), 3.5);

	}
	else if (foldedness[1]>0.8&&foldedness[2]>0.8&&foldedness[3]>0.8&&nfold==3)
	{
		putText(show, "mean:" + (std::to_string(3)).substr(0, 4), Point(50, 460), cv::HersheyFonts::FONT_HERSHEY_COMPLEX, 1, Scalar(255, 255, 255), 3.5);

	}
	else if (nfold ==4&& foldedness[3]>0.8)
	{
		putText(show, "mean:" + (std::to_string(4)).substr(0, 4), Point(50, 460), cv::HersheyFonts::FONT_HERSHEY_COMPLEX, 1, Scalar(255, 255, 255), 3.5);

	}
	else if (nfold == 5)
	{
		putText(show, "mean:" + (std::to_string(5)).substr(0, 4), Point(50, 460), cv::HersheyFonts::FONT_HERSHEY_COMPLEX, 1, Scalar(255, 255, 255), 3.5);

	}
	else if (foldedness[2]>0.8&&foldedness[3]>0.8&&foldedness[4]>0.8&&nfold == 3)
	{
		putText(show, "mean:OK", Point(50, 460), cv::HersheyFonts::FONT_HERSHEY_COMPLEX, 1, Scalar(255, 255, 255), 3.5);

	}
	else
	{
		putText(show, "mean: no information", Point(50, 460), cv::HersheyFonts::FONT_HERSHEY_COMPLEX, 1, Scalar(255, 255, 255), 3.5);

	}
	
}