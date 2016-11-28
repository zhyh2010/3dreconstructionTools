/**********************************************************************
    
  ChessboardImageGenerator
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

using namespace std;
using namespace cv;

void ShowHelp(int argc){
	if (argc<4 || argc>5){
		cout << "usage: chessboardGenerator h w l [f]"
			<< "\n\th\tlattice number in height"
			<< "\n\tw\tlattice number in width"
			<< "\n\tl\tlength of lattice in pixel"
			<< "\n\tf\tfilename with proper extension for saving the generated image" << endl;
		exit(0);
	}
}

void drawBlocks(Mat src, bool isLight, int offsetx, int offsety, int blockLen){
	for (int i = 0; i < blockLen; i++){
		for (int j = 0; j < blockLen; j++){
			src.at<double>(offsetx + i, offsety + j) = (isLight ? 255 : 0);
		}
	}
}

void drawChessBox(Mat & src, int h, int w, int len){
	int height = h*len;
	int width = w*len;
	src = Mat(width, height, CV_64FC1);
	for (int i = 0; i < h; i++){
		for (int j = 0; j < w; j++){
			drawBlocks(src, (i + j) % 2, i * len, j * len, len);
		}
	}
}

string getSavedFileName(int argc, char * argv[]){
	string str;
	if (argc == 5){
		str = argv[4];
	}
	else{
		str = "chessboard";
		for (int i = 1; i < 4; i++){
			str += "-";
			str += argv[i];
		}
		str += ".jpg";
	}
	return str;
}

void saveFile(string filename, Mat src){
	if (imwrite(filename, src)){
		cout << "succeed to save image:" << filename << endl;
	}
	else{
		cout << "failed to save image:" << filename << endl;
	}
}

int main(int argc, char** argv){
	Mat src;
	ShowHelp(argc);	
	drawChessBox(src, atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
	string fileName = getSavedFileName(argc, argv);	
	saveFile(fileName, src);	
	return 0;
}
