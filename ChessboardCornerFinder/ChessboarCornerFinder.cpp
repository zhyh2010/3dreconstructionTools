/**********************************************************************
    
  ChessboarCornerFinder
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
#include <string>
#include <iostream>
using namespace std;
using namespace cv;

void ShowTips(int argc){
	if (argc != 4 && argc != 5){
		cout << "usage:\tchessboardCornerFinder fi nw nh [fs]"
			<< "\n\tfi\tfilename of the image"
			<< "\n\tnw\tnumber of chessboard corners in width"
			<< "\n\tnh\tnumber of chessboard corners in height"
			<< "\n\tfs\tname of the file to save the result " << endl;
		return exit(0);
	}
}

Mat loadGrayImg(string name){
	Mat img = imread(name, CV_LOAD_IMAGE_GRAYSCALE);
	if (img.empty()){
		cerr << "failed to load image" << endl;
		exit(-1);
	}
	return img;
}

bool FindCorners(Mat src, int nw, int nh, Mat & corners, Mat & rgb){
	corners = Mat(nw, nh, CV_32FC2);
	bool patternfound = findChessboardCorners(src, Size(nw, nh), corners, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_NORMALIZE_IMAGE);
	if (patternfound){
		cornerSubPix(src, corners, Size(11, 11), Size(-1, -1),
			TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
	}
	cvtColor(src, rgb, CV_GRAY2BGR);
	drawChessboardCorners(rgb, Size(nw, nh), corners, patternfound);
	return patternfound;
}

void saveFile(string filename, Mat src){
	if (imwrite(filename, src)){
		cout << "succeed to save image:" << filename << endl;
	}
	else{
		cout << "failed to save image:" << filename << endl;
	}
}

string getSavedFileName(int argc, char * argv[]){
	string foutname;//name of the file to save the result
	if (argc == 5){
		foutname = argv[4];
	}
	else{
		foutname = foutname + argv[1] + "-corners.yml";
	}
	return foutname;
}

int main(int argc,char** argv){
	ShowTips(argc);
	Mat img = loadGrayImg(argv[1]);	
	Mat corners, rgb;
	bool ret = FindCorners(img, atoi(argv[2]), atoi(argv[3]), corners, rgb);
	if (ret)
		saveFile(string(argv[1]) + ".corners.jpg", rgb);
	else{
		cout << "not all corners were found" << endl;
	}
	
	string foutname = getSavedFileName(argc, argv);

	//save as yml 
	FileStorage fs(foutname, CV_STORAGE_WRITE);
	fs << "corners" << corners;
	fs.release();

	cout<<"success to save data in "<<foutname<<endl;
	return 0;
}