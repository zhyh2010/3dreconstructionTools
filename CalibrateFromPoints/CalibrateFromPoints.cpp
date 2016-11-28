/**********************************************************************
    
  CalibrateFromPoints
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

#include <opencv.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>

using namespace std;
using namespace cv;

void showTips(int argc){
	if (argc != 7){
		cout << "usage: CalibFromPoints f.ifl l w h nw nh"
			<< "\n\tf.ifl\tlist of files which store filename of corner coordinates"
			<< "\n\t\tlike this"
			<< "\n\t\t\timg0.jpg"
			<< "\n\t\t\timg1.jpg"
			<< "\n\t\t\timg2.jpg"
			<< "\n\tl\tlength of chessboard lattice"
			<< "\n\tw\twidth of the images"
			<< "\n\th\theight of the images"
			<< "\n\tnw\tnumber of chessboard corners in width"
			<< "\n\tnh\tnumber of chessboard corners in height"
			<< endl;
		exit(-1);
	}
}

vector<string> readFiles(string filename){
	vector<string> filenameVec;
	fstream fin(filename, ios::in);
	if (!fin.is_open()){
		cout << "can not open file:" << filename << endl;
		exit(-1);
	}
	while (!fin.eof()){
		string nameBuf;
		fin >> nameBuf;
		if (nameBuf == "")
			continue;
		filenameVec.push_back(nameBuf);
	}
	fin.close();
	return filenameVec;
}

struct CornerInfo{
	int cornerHeightNum;
	int cornerWidthNum;
	int cornerCount;
};

CornerInfo getCornersNum(int nw, int nh){
	CornerInfo info;
	info.cornerHeightNum = nh;
	info.cornerWidthNum = nw;
	info.cornerCount = info.cornerHeightNum * info.cornerWidthNum;
	return info;
}

void setCountMat(int imgCount, Mat & countMat, int cornerCount){
	countMat = Mat(imgCount, 1, CV_32SC1);
	for (int i = 0; i < imgCount; i++){
		countMat.at<int>(i, 0) = cornerCount;
	}
}

void setPoint2dFromFile(vector<string> filenameVec, vector<vector<Point2f>> & points2d){
	for (unsigned int i = 0; i < filenameVec.size(); i++){
		FileStorage fs(filenameVec[i], CV_STORAGE_READ);
		Mat corner;
		fs["corners"] >> corner;
		vector<Point2f> tmp;
		for (int i = 0; i < corner.rows; i++){
			tmp.emplace_back(corner.at<float>(i, 0), corner.at<float>(i, 1));
		}	
		points2d.push_back(tmp);
		fs.release();
	}
}

void setPoint3d(int imgCount, vector<vector<Point3f>> & points3d, CornerInfo info, double latticeLen){
	for (int m = 0; m < imgCount; m++){
		vector<Point3f> tmp;
		for (int i = 0; i < info.cornerHeightNum; i++){
			for (int j = 0; j < info.cornerWidthNum; j++){
				tmp.emplace_back(j * latticeLen, i * latticeLen, 0);
			}
		}
		points3d.push_back(tmp);
	}
}

void Calibration(vector<vector<Point3f>> & points3d, vector<vector<Point2f>> & points2d, Size imgSize, Mat & intriniscMat, Mat & distortionMat, vector<Mat> & rotateMat, vector<Mat> & transMat){
	calibrateCamera(points3d, points2d, imgSize, intriniscMat, distortionMat, rotateMat, transMat, 0);
}

void saveMats(string fsname, int imgCount, Mat intriniscMat, Mat distortionMat, vector<Mat> & rotateMat, vector<Mat> & transMat){
	FileStorage fs(fsname, CV_STORAGE_WRITE);
	fs << "intrinsic" << intriniscMat;
	fs << "distortion" << distortionMat;
	for (int i = 0; i < imgCount; i++){
		fs << string("rotate") + to_string(i) << rotateMat[i];
		fs << string("translate") + to_string(i) << transMat[i];
	}
	fs.release();
}

int main(int argc,char** argv){
	showTips(argc);
	//read files
	auto filenameVec = readFiles(argv[1]);
	auto info = getCornersNum(atoi(argv[5]), atoi(argv[6]));

	//set counteMat
	Mat countMat;
	setCountMat(filenameVec.size(), countMat, info.cornerCount);
	
	//set points2d from files
	vector<vector<Point2f>> points2d;
	setPoint2dFromFile(filenameVec, points2d);

	//set points3d
	vector<vector<Point3f>> points3d;
	setPoint3d(filenameVec.size(), points3d, info, atof(argv[2]));
	
	//calibration
	Size imgSize(atoi(argv[3]), atoi(argv[4]));
	Mat intriniscMat, distortionMat;
	vector<Mat> rotateMat, transMat;
	Calibration(points3d, points2d, imgSize, intriniscMat, distortionMat, rotateMat, transMat);

	//save camera parameters
	string fsname = string(argv[1]) + "-parameters.yml";
	saveMats(fsname, filenameVec.size(), intriniscMat, distortionMat, rotateMat, transMat);

	cout<<"Done.parameters saved in"<<fsname<<endl;

	return 0;
}
