#include <opencv.hpp>
using namespace cv;

//²âÊÔMat::step1(), Mat::ptr(), Mat::isContinuous()
//²âÊÔVideoCapture::isOpened()

void _2main(){
	//Mat img = imread("C:\\Users\\weigs\\Desktop\\laser.bmp");
	Mat img;
	//VideoCapture cap("C:\\Users\\weigs\\Desktop\\video.avi");
	VideoCapture cap(0);
	//cvtColor(img, img, COLOR_BGR2GRAY);
	//printf("%d %d %d\n", img.rows, img.cols, img.step1());
	//uchar *p = img.data, *p2 = img.ptr(1);
	//printf("step:%d", p2 - p);
	//printf("img.isContinuous() ? %c", img.isContinuous() ? 'y' : 'n');

	while(cap.isOpened()){
		cap >> img;
		imshow("try", img);
		waitKey(15);
	}
	//printf("%d %d %d", img.rows, img.cols, img.depth());
	waitKey(3000);


}