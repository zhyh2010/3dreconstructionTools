/**********************************************************************
    
  VideoRecorder
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
#include <opencv.hpp>
#include <windows.h>

using namespace std;
using namespace cv;

VideoCapture OpenCapture(int deviceId){
	VideoCapture cap = VideoCapture(deviceId);
	if (!cap.isOpened()){
		cerr << "Cannot open camera" << deviceId << endl;
	}
	return cap;
}

vector<VideoWriter> RecordCamera(vector<string> & videoName, vector<VideoCapture> & caps){
	caps = vector<VideoCapture>(2);
	Mat img[2];
	vector<VideoWriter> writer(2);
	for (int i = 0; i < 2; i++){
		caps[i] = OpenCapture(i);
		caps[i].read(img[i]);
		double fps = 15.0;
		Size size(int(caps[i].get(CV_CAP_PROP_FRAME_WIDTH)), 
			int(caps[i].get(CV_CAP_PROP_FRAME_HEIGHT)));
		writer[i].open(videoName[i], CV_FOURCC('M', 'J', 'P', 'G'), fps, size);
		if (!writer[i].isOpened()){
			cerr << "can not create video writer" << endl;
			exit(-1);
		}
	}
	return writer;
}

Mat getFrameData(VideoCapture & cap){
	Mat src;
	if (!cap.read(src)){
		cerr << "can not query frame from camera" << endl;
		exit(-1);
	}
	return src;
}

vector<Mat> getPairedFrameData(vector<VideoCapture> & caps){
	vector<Mat> mats;
	for (auto item : caps){
		mats.emplace_back(getFrameData(item));
	}
	return mats;
}

int main(int argc, char** argv){
	vector<string> videoName{ "../data/videos/video0.avi", "../data/videos/video1.avi" };

	if (argc == 3){//user defined filename
		videoName[0] = argv[1];
		videoName[1] = argv[2];
	}

	vector<VideoCapture> caps;
	vector<VideoWriter> writer = RecordCamera(videoName, caps);

	namedWindow("camera1");
	namedWindow("camera2");

	cout << "press Enter to start ,Esc to quit" << endl;
	bool startWrite = false;

	vector<Mat> res;
	while (true){
		res = getPairedFrameData(caps);
		imshow("camera1", res[0]);
		imshow("camera2", res[1]);

		if (startWrite){
			writer[0].write(res[0]);
			writer[1].write(res[1]);
		}

		int key = waitKey(3);
		if (key == 27){
			cout << "recording stopped" << endl;
			break;
		}
		else if (key == 13){
			cout << "recording started" << endl;
			startWrite = true;
		}
	}
	destroyAllWindows();
	return 0;
}