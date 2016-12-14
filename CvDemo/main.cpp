#include <opencv.hpp>

using namespace cv;

//高旭 从左搜索到第一个大于230点，继续搜索找到第一个小于230点，从此范围向两边延伸至150点，之后重心法
void PeanutDivide(uchar *p, int w, int h)
{
	int base = 0;		//行地址
	for (int i = 0; i < h; i++)			//行搜索
	{
		int middle = 0;
		int left = 0, right = 0, j = 0;
		for (j = 0; j < w; j++)			//从左搜索到第一个大于230点
		{
			if (p[base + j] >= 200)
			{
				left = j;
				break;
			}
		}
		for (j; j < w; j++)		//继续搜索找到第一个小于150点
		{
			if (p[base + j] < 150)
			{
				right = j - 1;
				break;
			}
		}
		for (j = left - 1; j>0; j--)		//向左搜索找左端点
		{
			if (p[base + j] < 150)
			{
				left = j + 1;
				break;
			}
		}
		if (right != 0 || left != 0)
		{
			int sum = 0, sum1 = 0;
			for (j = left; j <= right; j++)		//重心法
			{
				sum += j*p[base + j];
				sum1 += p[base + j];
			}
			middle = ((double)sum + 0.5) / sum1;
		}
		for (j = 0; j < w; j++)			//抹黑
		{
			p[base + j] = 0;
		}
		p[base + middle] = middle == 0 ? 0 : 255;
		base += w;
	}
}

//高旭 OTSU
void OtsuDivide(uchar *p, int w, int h, int *offs)
{
	//用otsu方法获取全局阈值，然后重心法提取光刀中心
	int arr[256] = { 0 };//灰度直方图
	int base = 0;		//行坐标
	double sum = 0, ave = 0;
	for (int i = 0; i < h; i++)			//灰度直方图统计,全局均值计算
	{
		for (int j = 0; j < w; j++)
		{
			arr[p[base + j]]++;
			sum += p[base + j];
		}
		base += w;
	}
	ave = sum / (w*h);		//全局均值
	int threshold = 0;
	double max = 0;
	double ave1 = 0, ave2 = 0, sum1 = 0, sum2 = 0, count1 = 0, count2 = 0;

	for (int i = 150; i < 256; i++)		//判断何时取最大类间差
	{
		ave1 = 0, ave2 = 0, sum1 = 0, sum2 = 0, count1 = 0, count2 = 0;
		for (int j = 0; j < i; j++)
		{
			sum1 += arr[j] * j;
			count1 += arr[j];
		}
		for (int j = i; j < 256; j++)
		{
			sum2 += arr[j] * j;
			count2 += arr[j];
		}
		ave1 = sum1 / count1;
		ave2 = sum2 / count2;
		if (max <= ((ave - ave1)*(ave2 - ave)))
		{
			max = ((ave - ave1)*(ave2 - ave));
			threshold = i;
		}
	}
	printf("%d,", threshold);
// 	if (threshold == 0) waitKey(0);
	base = 0;
	// 		for (int i = 0; i < h; i++)			//二值化
	// 		{
	// 			for (int j = 0; j < w; j++)
	// 			{
	// 				if (p[base+j]>=threshold)
	// 				{
	// 					p[base + j] = 255;
	// 				}
	// 				else
	// 				{
	// 					p[base + j] = 0;
	// 				}
	// 			}
	// 			base += w;
	// 		}

	for (int i = 0; i < h; i++)			//每行重心法
	{
		int sum3 = 0, count3 = 0;
		for (int j = 0; j < w; j++)
		{
			if (p[base + j] >= threshold)
			{
				sum3 += j*p[base + j];
				count3 += p[base + j];
			}
			p[base + j] = 0;
		}
		if (sum3 != 0)
		{
			int oppo = (sum3 + 0.5) / count3;
			p[base + oppo] = 255;
		}
		base += w;

	}


}

//自适应阈值，图像分割，阈值以下的置黑
void adaptive(Mat img){
	int h = img.rows;
	int w = img.cols;
	int step = img.step1();
	uchar *data = img.data;

	uchar th_h = 100; //光刀高阈值
	uchar th_d = 50; //阈值浮动量 delta

	int base = 0; //某行基址
	for (int i = 0; i < h; i++){

		int iMax = 0; //每行最大值的索引值，初始值为0
		uchar max = data[base + 0]; //第一个像素的值

		//寻找每行最大值
		for (int j = 1; j < w; j++){
			if (data[base + j]>max){
				iMax = j;
				max = data[base + j];
			}
		}

		if (max < th_h){
			//未达到阈值，视为该行没有光刀
			for (int j = 0; j < w; j++) data[base + j] = 0;
		}
		else{
			int th_l = max - th_d; //光刀低阈值
			
			//找左右边缘
			int rightEdge = iMax + 1;
			while (data[base + rightEdge] > th_l) rightEdge++;

			int leftEdge = iMax - 1;
			while (data[base + leftEdge] > th_l) leftEdge--;

			//置黑其余像素
			for (int j = 0; j <= leftEdge; j++) data[base + j] = 0;
			for (int j = rightEdge; j < w; j++) data[base + j] = 0;
		}

		base += step; //跳到下一行
	}
}

//重心（形心）法，得到偏移量数组
void centroid(Mat img, int *offs){
	int h = img.rows;
	int w = img.cols;
	int step = img.step1();
	uchar *data = img.data;

	int base = 0; //某行基址
	for (int i = 0; i < h; i++){
		//分子，分母
		int sum1 = 0, sum2 = 0;
		//求和
		for (int j = 0; j < w; j++){
			uchar val = data[base + j];
			sum1 += val*j;
			sum2 += val;
		}

		// **一定要记得判断分母为零的情况！！**
		int center = sum2 == 0 ? 0 : (int)((float)sum1 / sum2 + 0.5f); //重心
		offs[i] = center;

		base += step; //跳到下一行
	}

}

//最大值法
void laserMax(Mat img, int *offs) {
	int h = img.rows;
	int w = img.cols;
	int step = img.step1();
	uchar *data = img.data;

	int base = 0; //某行基址
	for (int i = 0; i < h; i++){

		int iMax = 0; //每行最大值的索引值，初始值为0
		uchar max = data[base + 0]; //第一个像素的值

		//寻找每行最大值
		for (int j = 1; j < w; j++){
			if (data[base + j]>max){
				iMax = j;
				max = data[base + j];
			}
		}
		//亮度大于阈值才认为是光刀
		offs[i] = max > 100 ? iMax : 0;

		base += step;
	}
}

//中点法
void midPoint(Mat img, uchar T, int *offs){
	int h = img.rows;
	int w = img.cols;
	int step = img.step1();
	uchar *data = img.data;

	//uchar T = 180; //阈值

	int base = 0; //某行基址
	for (int i = 0; i < h; i++){
		//上升沿
		int left = 0;
		while (data[base + left] < T && left < w) left++;

		//没找到大于阈值的点，这行没有光刀
		if (left == w) {
			offs[i] = 0;
		}
		else{
			//下降沿
			int right = w - 1;
			while (data[base + right] < T && right > 0) right--;
			offs[i] = (left + right) >> 1; //中点，除以二
		}
		base += step;
	}
}

//按行取OTSU，阈值以下置黑
void otsuByRow(Mat img){
	int h = img.rows;
	int w = img.cols;
	int step = img.step1();
	uchar *data = img.data;

	int base = 0; //某行基址
	for (int i = 0; i < h; i++){
		int hist[256] = { 0 }; //直方图histogram
		//求一行的直方图
		for (int j = 0; j < w; j++){
			uchar gray = data[base + j];
			hist[gray]++;
		}

		uchar bestT = 0; //最大方差对应阈值
		float maxG = 0; //最大方差
		int valSum0 = 0, valSum1 = 0; //灰度值总和
		for (int val = 0; val < 256; val++)
			valSum1 += hist[val] * val;
		int N0 = 0, N1 = w, N = w; //阈值 以左、以右、总体 的像素个数。初始值：T=0时

		for (uchar T = 0; T < 255; T++){
			int delta = hist[T];
			N0 += delta;
			N1 -= delta;
			valSum0 += delta * T;
			valSum1 -= delta * T;
			//新平均灰度
			float u0 = valSum0 / (float)N0;
			float u1 = valSum1 / (float)N1;
			//新的两边所占比例
			float w0 = N0 / (float)N;
			float w1 = N1 / (float)N;
			//新方差
			float G = w0 * w1 * (u0 - u1) * (u0 - u1);
			//阈值如果太小则忽略
			if (T < 50) continue;
			if (G >= maxG){
				bestT = T;
				maxG = G;
			}
		}
		//if (bestT == 50){
		//	printf("%d", bestT);
		//	bestT = 255;
		//}
		//找最大灰度，如果太小，说明没有光刀
		int max = 255;
		while (hist[max] == 0) max--;
		if (max < 80) bestT = 255;

		//将该行中小于阈值的部分去除
		for (int j = 0; j < w; j++){
			if (data[base + j] < bestT)
				data[base + j] = 0;
		}

		//**************************
		base += step;
	}

}

//OTSU。返回：取到的阈值。Gs：所有阈值对应的方差的数组，如果不需要则输入nullptr
uchar otsu(Mat img, float Gs[] = nullptr){
	int h = img.rows;
	int w = img.cols;
	int step = img.step1();
	uchar *data = img.data;

	int hist[256] = { 0 }; //直方图histogram
	//求直方图
	int base = 0;
	for (int i = 0; i < h; i++){
		for (int j = 0; j < w; j++){
			uchar gray = data[base + j];
			hist[gray]++;
		}
		base += step;
	}

	uchar bestT = 0; //最大方差对应阈值
	float maxG = 0; //最大方差
	int valSum0 = 0, valSum1 = 0; //灰度值总和
	for (int val = 0; val < 256; val++)
		valSum1 += hist[val] * val;
	int N0 = 0, N1 = h*w, N = h*w; //阈值 以左、以右、总体 的像素个数。初始值：T=0时

	for (uchar T = 0; T < 255; T++){
		int delta = hist[T];
		N0 += delta;
		N1 -= delta;
		valSum0 += delta * T;
		valSum1 -= delta * T;
		//新平均灰度
		float u0 = valSum0 / (float)N0;
		float u1 = valSum1 / (float)N1;
		//新的两边所占比例
		float w0 = N0 / (float)N;
		float w1 = N1 / (float)N;
		//新方差
		float G = w0 * w1 * (u0 - u1) * (u0 - u1);
		if (Gs != nullptr) Gs[T] = G; //中间数据
		//阈值如果太小则忽略
		if (T < 50) continue;
		if (G > maxG){
			bestT = T;
			maxG = G;
		}
	}

	return bestT;
}

//使用阈值进行图像分割，低于阈值的置黑
void applyThreshold(Mat img, uchar T){
	int h = img.rows;
	int w = img.cols;
	int step = img.step1();
	uchar *data = img.data;

	int base = 0;
	for (int i = 0; i < h; i++){
		for (int j = 0; j < w; j++){
			if (data[base + j] < T) data[base + j] = 0;
		}
		base += step;
	}
}

//画数据图
Mat drawGraph(const float vals[], int count, int height, float max){
	Mat img(height + 1, count, CV_8UC1, Scalar(0));
	int h = img.rows;
	int w = img.cols;
	int step = img.step1();
	uchar *data = img.data;

	for (int x = 0; x < count; x++){
		int y = (int)(vals[x] / max * height + 0.5f); //从下往上数的纵坐标
		if (y > height) y = height; //越界检查
		int base = height * step + x;
		for (y; y > 0; y--){
			data[base] = 255;
			base -= step;
		}
	}

	return img;
}

//把光刀结果用红线画在原图上
void mix(Mat &src, int *offs){
	int h = src.rows;
	int w = src.cols;
	int step = src.step1();
	uchar *data = src.data;

	int base = 0; //某行基址
	for (int i = 0; i < h; i++){
		if (offs[i]>0){
			//该行第i个像素的首地址
			uchar *bgr = data + base + offs[i] * 3;
			//设为红色
			bgr[0] = 0;
			bgr[1] = 0;
			bgr[2] = 255;
		}
		base += step;
	}
}

void EdgeDivide(uchar *p, int w, int h, int *offs)
//边缘检测，找出230以上的点，向两边检测二阶导数变化的点
{
	//QElapsedTimer sec;
	//sec.start();

	int base = 0;
	for (int i = 0; i < h; i++)
	{
		// 		int *diff = new int[w];		//梯度数组
		// 		for (int j = 0; j < w - 1; j++)
		// 		{
		// 			diff[j] = p[base + j + 1] - p[base + j];
		// 		}
		// 		diff[w - 1] = 0;
		int left = 0;
		int right = w - 1;
		int j = 0;
		for (j = 0; j<w; j++)			//首先取230以上的点为光刀中心点
		{
			if (p[base + j]>150)
			{
				left = j - 1;
				break;
			}
		}
		for (j; j < w; j++)
		{
			if (p[base + j] < 150)
			{
				right = j;
				break;
			}
		}
		for (j = left; j > 1; j--)				//向两边寻找梯度较大的点
		{
			if (p[base + j + 1] + p[base + j - 1] - 2 * p[base + j] >= 0)
			{
				left = j;
				break;
			}
		}
		for (j = right; j < w - 1; j++)
		{
			if (p[base + j + 1] + p[base + j - 1] - 2 * p[base + j] <= 0)
			{
				right = j;
				break;
			}
		}
		if (left != 0 || right != w - 1)			//有光刀则重心法
		{
			int count = 0;
			int sum = 0;
			int middle = 0;
			for (j = left; j <= right; j++)
			{
				sum += j*p[base + j];
				count += p[base + j];
			}
			middle = (sum + 0.5) / count;
			//for (j = 0; j < w; j++)
			//{
			//	p[base + j] = 0;
			//}
			//p[base + middle] = 255;

			//计入偏移量数组
			offs[i] = middle;
		}
		else
		{
			offs[i] = 0;

			//未找到则抹黑
			//for (j = 0; j < w; j++)
			//{
			//	p[base + j] = 0;
			//}
		}
		// 		for (j = 0; j < w;j++)				//显示提取区域
		// 		{
		// 			p[base + j] =0;
		// 		}
		// 		if (left!=0||right!=w-1)
		// 		{
		// 			for (j = left; j <= right;j++)
		// 			{
		// 				p[base + j] = 255;
		// 			}
		// 		}
		base += w;
	}
	//qDebug("%d", sec.elapsed());
}


void _0main(){
	Mat src; //原图，**颜色为BGR**
	Mat img; //中间运算结果，**颜色为灰度**
	Mat dst; //缩放后显示结果
	Mat mat;
	Mat otsuImg; //otsu中间值
	float otsuVals[256] = { 0 };
	int *offs; //偏移量结果数组
// 	VideoCapture cap("video.avi");
// 	cap >> src;
	src = imread("wgs.bmp");
	offs = new int[src.rows];

	while (1){
// 		bool ok = cap.read(src);
// 		if (!ok) break; //视频读完后退出，否则会出错
		cvtColor(src, img, COLOR_BGR2GRAY); //处理灰度图，便于计算

		//预处理 滤波
		//blur(img, img, Size(5, 5));
		//GaussianBlur(img, img, Size(5, 5), 0, 0);
		//medianBlur(img, img, 5);
		//bilateralFilter(mat, img, 4, 8, 2);
		//resize(mat, mat, Size(), 2, 2, INTER_NEAREST);
		//imshow("mat", mat);

		//图像分割
		//adaptive(img);
		//otsuByRow(img);

		//uchar T = otsu(img, otsuVals);
		//applyThreshold(img, T);
		//resize(img, otsuImg, Size(), 2, 2, INTER_NEAREST);
		//imshow("test", img);

		//画OTSU阈值方差图
		//int maxT = 0;
		//for (int i = 0; i < 256; i++)
		//if (otsuVals[i] > otsuVals[maxT])
		//	maxT = i;
		//Mat mat = drawGraph(otsuVals, 256, 256, otsuVals[maxT]);
		//resize(mat, otsuImg, Size(), 2, 2, INTER_NEAREST);
		//imshow("test otsu", otsuImg);

		//应用光刀提取算法
		//laserMax(img, offs);
		//midPoint(img, 200, offs);
		//centroid(img, offs);
		//PeanutDivide(src.data, src.cols, src.rows);
		//OtsuDivide(img.data, img.cols, img.rows, offs);
		EdgeDivide(img.data, img.cols, img.rows, offs);

		//绘制光刀，显示
		mix(src, offs);
		resize(src, dst, Size(), 2, 2, INTER_NEAREST);
		imshow("mixed img", dst);

		//默认5帧，按任意键加速，按esc==27退出
		if (waitKey(0) == 27) break;
	}
	

	//waitKey(0);
	if (offs != nullptr)
		delete offs;
}

