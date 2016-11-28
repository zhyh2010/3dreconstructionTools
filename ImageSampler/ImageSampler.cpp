/**********************************************************************
    
  ImageSampler
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
#include <fstream>
#include <opencv.hpp>
#include <windows.h>

using namespace cv;
using namespace std;

VideoCapture openCapture(int deviceId){
	VideoCapture cap(deviceId);
	if (!cap.isOpened()){
		cerr << "Can not open camera" << deviceId << endl;
		exit(0);
	}
	return cap;
}

Mat getFrameData(VideoCapture & cap){
	Mat src;
	if (!cap.read(src)){
		cerr << "can not query frame from camera" << endl;
		exit(-1);
	}
	return src;
}

vector<VideoCapture> openCameras(const vector<int> & deviceIds){
	vector<VideoCapture> videocaps;
	for (auto item : deviceIds){
		videocaps.emplace_back(item);
	}
	return videocaps;
}

vector<Mat> getPairedFrameData(vector<VideoCapture> & caps){
	vector<Mat> mats;
	for (auto item : caps){
		mats.emplace_back(getFrameData(item));
	}
	return mats;
}

void saveFile(string filename, Mat src){
	if (imwrite(filename, src)){
		cout << "succeed to save image:" << filename << endl;
	}
	else{
		cout << "failed to save image:" << filename << endl;
	}
}

//#define USEONE

int main(int argc, char** argv){
	vector<int> deviceIds{ 0, 1};
	auto caps = openCameras(deviceIds);	

	namedWindow("camera1", CV_WINDOW_NORMAL);

#ifndef USEONE
	namedWindow("camera2", CV_WINDOW_NORMAL);
#endif

	int count = 0;
	cout << "press Enter to save a pair of images" << endl;
	Sleep(1000);

	while (true){
		auto Imgs = getPairedFrameData(caps);
		imshow("camera1", Imgs[0]);
#ifndef USEONE
		imshow("camera2", Imgs[1]);
#endif
		int key = waitKey(3);
		if (key == 27){
			break;
		}
		else if (key == 13){
			string camera_name1 = "../data/images/image0_" + to_string(count) + ".jpg";
			saveFile(camera_name1, Imgs[0]);
#ifndef USEONE
			string camera_name2 = "../data/images/image1_" + to_string(count) + ".jpg";			
			saveFile(camera_name2, Imgs[1]);
#endif
			count++;
		}
	}
	return 0;
}


