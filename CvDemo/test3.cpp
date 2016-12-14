#include <opencv2/opencv.hpp>
#include <iostream>
using namespace cv;
using namespace std;

//测试Mat.data的保留转移规律
//cvtColor, clone, copyTo

void main(){
	Mat m1 = imread("wgs.bmp");
	Mat m2 = imread("wgs.bmp");
	Mat m3 = m1;
	printf("%x\n", m1.data);
	printf("%x\n", m2.data);
	printf("%x\n", m3.data);

	Mat m4 = m1.clone();
	printf("%x\n", m4.data);
	//cvtColor(m1, m4, COLOR_BGR2RGB);
	//printf("%x\n", m4.data);
	cvtColor(m2, m4, COLOR_BGR2RGB);
	printf("%x\n", m4.data);
	Mat m5 = imread("1.jpg");
	cvtColor(m5, m4, COLOR_BGR2RGB);
	printf("%x\n", m4.data);
	cvtColor(m4, m4, COLOR_RGB2BGR);
	printf("%x\n", m4.data);
	cvtColor(m4, m4, COLOR_RGB2GRAY);
	printf("%x\n", m4.data);

	printf("m1:%x; m5:%x\n", m1.data, m5.data);
// 	imshow("m1 before", m1);
// 	imshow("m5 before", m5);
	m1.copyTo(m5);
	printf("m1:%x; m5:%x\n", m1.data, m5.data);
// 	imshow("m1 after", m1);
// 	imshow("m5 after", m5);
	waitKey();
}