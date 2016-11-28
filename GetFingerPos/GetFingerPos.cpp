/**********************************************************************
    
  GetFingerPos
  Release Date: 2010/12/31
  Copyright (C) 2010	Zhijie Lee
                        email: onezeros.lee@gmail.com 
                        web: http://blog.csdn.net/onezeros
  modified by zhyh2010 in 2016/11/28
						web: http://blog.csdn.net/zhyh1435589631

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
  
**********************************************************************/

#include <iostream>
#include <string>
#include <queue>
#include <opencv.hpp>
#include <exception>
#include <vector>

using namespace cv;
using namespace std;

class Target{
public:
	int area;//size
	int top;//position
	int bottom;
	int left;
	int right;
	Target() :area(0), top(0), bottom(0), left(0), right(0){}
	int width(){ return right - left; }
	int height(){ return bottom - top; }
};

vector<string> getVideoFileName(int argc, char ** argv){
	vector<string> videoFile(2);
	if (argc == 3){				//GetFingerPos 0.avi 1.avi
		videoFile[0] = argv[1];
		videoFile[1] = argv[2];
	}
	else{
		videoFile[0] = "../data/videos/video0.avi";
		videoFile[1] = "../data/videos/video1.avi";
	}
	return videoFile;
}

void openCapture(const string & filename, VideoCapture & cap, int & frameCount){
	cap.open(filename);
	if (!cap.isOpened()){
		throw exception("无法打开视频");
	}

	frameCount = cap.get(CV_CAP_PROP_FRAME_COUNT);
	frameCount--;
}

bool getCameraFrame(VideoCapture & cap, Mat & src){
	if (!cap.read(src)){
		throw exception("无法读取帧数据");
		return false;
	}
	return true;
}

void dealWithImg(Mat & src){
	Mat src_YCrCb;
	cvtColor(src, src_YCrCb, CV_RGB2YCrCb);                     // 转化到 YCRCB 空间处理
	vector<Mat> channels;
	split(src_YCrCb, channels);

	Mat target = channels[2];
	threshold(target, target, 0, 255, CV_THRESH_OTSU);              // 二值化处理

	Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));   // 开运算去除噪点
	morphologyEx(target, target, MORPH_OPEN, element);

	vector<vector<Point>> contours;                                 // 利用最大范围查找手臂
	findContours(target, contours, CV_RETR_EXTERNAL, CHAIN_APPROX_NONE);
	double mymax = 0;
	vector<Point> max_contours;
	for (int i = 0; i < contours.size(); i++){
		double area = contourArea(contours[i]);
		if (area > mymax){
			mymax = area;
			max_contours = contours[i];
		}
	}
	vector<vector<Point>> final_cont;
	final_cont.push_back(max_contours);
	Mat target1 = Mat::zeros(src.rows, src.cols, CV_8U);
	drawContours(target1, final_cont, -1, Scalar(255), CV_FILLED);
	src = target1;
}

/*!
 * \fn 通过BFS 遍历二值化图像， 找到他们的连通域， 选出面积大于指定阈值的区域块， 并以最大的作为这个图像的区域块
 *			本质上是查找 图像中的最大图形的最小外包轮廓
 *   \brief 
 *   \param 
 *   \return 
 */
void FindTarget(Mat img, const int area_threshold, Target & targets){
	Target tar;
	for (int h = 0; h < img.rows; h++){
		for (int w = 0; w < img.cols; w++){
			if (img.at<uchar>(h, w) == 255){
				Target target;
				target.top = h;
				target.bottom = h;
				target.left = w;
				target.right = w;
				queue<Point> points;
				points.push(Point(w, h));
				img.at<uchar>(h, w) = 0;

				//find target with breadth iteration  BFS
				while (!points.empty()){
					target.area++;
					Point p = points.front();
					points.pop();

					if (p.x > 0 && img.at<uchar>(p.y, p.x - 1) == 255){//left
						img.at<uchar>(p.y, p.x - 1) = 0;
						points.push(Point(p.x - 1, p.y));
						if (target.left > p.x - 1){
							target.left = p.x - 1;
						}
					}
					if (p.y + 1 < img.rows && img.at<uchar>(p.y + 1, p.x) == 255){//bottom
						img.at<uchar>(p.y + 1, p.x) = 0;
						points.push(Point(p.x, p.y + 1));
						if (target.bottom < p.y + 1){
							target.bottom = p.y + 1;
						}
					}
					if (p.x + 1 < img.cols && img.at<uchar>(p.y, p.x + 1) == 255){//right
						img.at<uchar>(p.y, p.x + 1) = 0;
						points.push(Point(p.x + 1, p.y));
						if (target.right < p.x + 1){
							target.right = p.x + 1;
						}
					}
					if (p.y > 0 && img.cols && img.at<uchar>(p.y - 1, p.x) == 255){//top
						img.at<uchar>(p.y - 1, p.x) = 0;
						points.push(Point(p.x, p.y - 1));
						if (target.top > p.y - 1){
							target.top = p.y - 1;
						}
					}
				}
				if (target.area > area_threshold){
					if (target.area > tar.area){
						tar = target;
					}
				}
			}
		}
	}
	targets = tar;
}


void playVideo(int frameCount, vector<VideoCapture> & caps, vector<vector<Point2d>> & pointsMat){
	pointsMat = vector<vector<Point2d>>(2);
	int frameCounter = 0;
	bool isRun = true;
	vector<Mat> imgs(2);
	while (frameCounter < frameCount){
		if (isRun && getCameraFrame(caps[0], imgs[0]) && getCameraFrame(caps[1], imgs[1])){
			imshow("camera1", imgs[0]);
			imshow("camera2", imgs[1]);
			for (int i = 0; i < 2; i++){
				dealWithImg(imgs[i]);
				Target target;
				// find target, 会清空原始数据
				FindTarget(imgs[i].clone(), 4000, target);

				Point2d point(-1.0, -1.0);
				if (target.width() >= 0){
					int left = -1;
					int right = -1;
					// 查找在 target.bottom 处 左右短线的中值
					for (int w = target.left; w <= target.right; w++){
						if (imgs[i].at<uchar>(target.bottom, w) == 255){
							if (left < 0)
								left = w;
							else
								right = w;
						}
					}
					point.y = target.bottom;
					if (right > 0){
						point.x = (right + left) >> 1;
					}					
				}	
				pointsMat[i].push_back(point);
			}
			frameCounter++;
		}
		imshow("binary1", imgs[0]);
		imshow("binary2", imgs[1]);

		int key = cvWaitKey(3);
		if (key == ' '){
			isRun = !isRun;
		}
		else if (key == 27){
			break;
		}
	}
}

int main(int argc, char** argv){
	vector<string> videoFile = getVideoFileName(argc, argv);
	vector<string> winName{ "camera1", "camera2" };

	int frameCount = 0;      //counts of frames in the two videos must be the same
	vector<VideoCapture> caps(2);
	for (int i = 0; i < 2; i++){
		openCapture(videoFile[i], caps[i], frameCount);
	}
	vector<vector<Point2d>> pointsMat;
	playVideo(frameCount, caps, pointsMat);

	for (int i = 0; i < 2; i++){
		FileStorage fs(videoFile[i] + ".yml", CV_STORAGE_WRITE);
		fs << "fingertip" << pointsMat[i];
		fs.release();
	}
	return 0;
}
