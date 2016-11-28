/**********************************************************************
    
  Show3dPoints
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
#include "glut.h"
#include <cstdlib>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>

using namespace std;
using namespace cv;

#pragma comment(lib, "glut32.lib")

///////////////////////////////////////////////////
GLsizei winWidth=500,winHeight=500;
vector<Point3d> pointVec;
//perspective parameters
Point3d eyePos;
static int radius=1000;
static float angleHorizontal=0;
static float angleVertical=0;
static float transHorizontal=0;
static float transVertical=0;

static unsigned int timerInterval=50;
bool isTimerOn=false;

static const float PI=3.1415926;
static bool isLButtonDown=false;
static bool isMButtonDown=false;
static CvPoint oldPoint;

vector<Mat> ComputeDis(vector<Point3d> & points, Mat matl, Mat matr, vector<Point2d> & left, vector<Point2d> & right);


///////////////////////////////////////////////////
//cv

void reconstruct3Dpoint(Mat matl, Mat matr, Point2d left, Point2d right, Point3d & point){
	Mat A(4, 4, CV_64F);
	Mat pl1 = matl.row(0);
	Mat pl2 = matl.row(1);
	Mat pl3 = matl.row(2);
	Mat pr1 = matr.row(0);
	Mat pr2 = matr.row(1);
	Mat pr3 = matr.row(2);
	double xl = left.x, yl = left.y;
	double xr = right.x, yr = right.y;
	A.row(0) = xl * pl3 - pl1;
	A.row(1) = yl * pl3 - pl2;
	A.row(2) = xr * pr3 - pr1;
	A.row(3) = yr * pr3 - pr2;
	
	Mat res;
	SVD::solveZ(A, res);
	point.x = res.at<double>(0, 0) / res.at<double>(3, 0);
	point.y = res.at<double>(1, 0) / res.at<double>(3, 0);
	point.z = res.at<double>(2, 0) / res.at<double>(3, 0);
}

void getPerpectiveProjectionMat(vector<Mat> & mats){
	mats = vector<Mat>(2);
	vector<string> names{ "../data/images/coorfile0.ifl-parameters.yml", "../data/images/coorfile1.ifl-parameters.yml" };
	for (int i = 0; i < 2; i++){
		FileStorage fs(names[i], CV_STORAGE_READ);
		if (!fs.isOpened()){
			throw exception("无法打开相应的棋盘格点文件");
		}

		Mat r, t, intri, rr;
		fs["rotate0"] >> r;
		fs["translate0"] >> t;
		fs["intrinsic"] >> intri;
		Rodrigues(r, rr);

		Mat m;
		hconcat(rr, t, m);
		m = intri * m;
		mats[i] = m;

		fs.release();
	}
}

bool saveToText(vector<Point3d> & data, string fileName){
	ofstream output(fileName);
	for (auto item : data){
		output << setw(20) << fixed << setprecision(15) << item.x << "\t" <<
			setw(20) << fixed << setprecision(15) << item.y << "\t" <<
			setw(20) << fixed << setprecision(15) << item.z << endl;
	}
	return true;
}

void get3dPoints(vector<Point3d> & points, string & yml1, string & yml2){
	vector<Mat> mats;
	getPerpectiveProjectionMat(mats);
	//read two 2d points sequences
	vector<vector<Point2d>> fingers(2);
	vector<string> fsname{ yml1, yml2 };
	for (int i = 0; i < 2; i++){
		FileStorage fs(fsname[i], CV_STORAGE_READ);
		fs["fingertip"] >> fingers[i];
		fs.release();
	}
	points.clear();

	//reconstruct
	vector<Point2d> lefts, rights;
	for (int i = 0; i < fingers[0].size(); i++){
		Point2d left = fingers[0][i];
		Point2d right = fingers[1][i];
		Point3d point;
		if (left.x > 0 && left.y > 0 && right.x > 0 && right.y > 0){
			reconstruct3Dpoint(mats[0], mats[1], left, right, point);
			points.push_back(point);
			lefts.push_back(left);
			rights.push_back(right);
		}
	}

	auto res = ComputeDis(points, mats[0], mats[1], lefts, rights);
	saveToText(points, "3dpoints.txt");
}

void drawLocusOnImage(){
	vector<string> names{ "../data/images/coorfile0.ifl-parameters.yml", "../data/images/coorfile1.ifl-parameters.yml" };
	vector<string> imgName{ "../data/images/image0_3.jpg", "../data/images/image1_3.jpg" };
	for (int i = 0; i < 2; i++){
		Mat r, t, intri, distortion;
		FileStorage fs(names[i], CV_STORAGE_READ);
		fs["rotate0"] >> r;
		fs["translate0"] >> t;
		fs["intrinsic"] >> intri;
		fs["distortion"] >> distortion;

		Mat imagePoints;
		//reproject coordinates from 3d to window
		projectPoints(pointVec, r, t, intri, distortion, imagePoints);

		//show and save images
		Mat img = imread(imgName[i]);
		if (!img.empty()){
			cerr << "failed to load image" << endl;
			exit(-1);
		}
		for (int j = 1; j < imagePoints.rows; j++){
			Point2d point1(imagePoints.at<double>(i - 1, 0), imagePoints.at<double>(i - 1, 1));
			Point2d point2(imagePoints.at<double>(i, 0), imagePoints.at<double>(i, 1));
			line(img, point1, point2, CV_RGB(255, 0, 0), 2);
		}

		string name = imgName[i] + ".locus.jpg";
		imshow(name, img);
		imwrite(name, img);
	}
}

/////////////////////////////////////////////////
//gl

void updateEyePosition();
void writeCharacter(Point3d p, char* str);
void init(void)
{
	glClearColor(1.0,1.0,1.0,1.0);
	glClearDepth(1.0);
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glLoadIdentity(); 	
	
	updateEyePosition();
	
	gluLookAt(eyePos.x,eyePos.y,eyePos.z,0.0,0.0,0.0,0.0,1.0,0.0);
	//gluLookAt(0, 0, -50, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	

	//glPushMatrix();
	//glTranslatef(transHorizontal,transVertical,0);
	
	//draw coordinate axis
	glColor3ub(255,0,0);//x
	glBegin(GL_LINES);
	glVertex3f(0,0,0);
	glVertex3f(300,0,0);
	glEnd();
	glColor3ub(0,255,0);//y
	glBegin(GL_LINES);
	glVertex3f(0,0,0);
	glVertex3f(0,300,0);
	glEnd();
	glColor3ub(0,0,255);//z
	glBegin(GL_LINES);
	glVertex3f(0,0,0);
	glVertex3f(0,0,300);
	glEnd();

	float blue=0;
	float red=255;
	float step=255/pointVec.size();

	//	
	vector<Point3d>::iterator ite;
	for (ite=pointVec.begin();ite!=pointVec.end();ite++){		
		//char
		char buf[4]={0};
		writeCharacter(*ite,_itoa(ite-pointVec.begin(),buf,10));
		//draw little sphere around the points
		glColor3ub(GLubyte(red),0,GLubyte(blue));
		red-=step;
		blue+=step;
		glPushMatrix();
		glTranslatef(ite->x,ite->y,ite->z);
		glutSolidSphere(3.0,5,5);
		glPopMatrix();
	}
	//line points up
	glColor3ub(0,0,0);
	glBegin(GL_LINE_STRIP);
	for (ite=pointVec.begin();ite!=pointVec.end();ite++){
		glVertex3f(ite->x,ite->y,ite->z);
	}
	glEnd();
	
	//glPopMatrix();
	glFlush();
}

void winReshapeFcn(GLint width,GLint height)
{
	winWidth=width;
	winHeight=height;

	glViewport(0,0,width,height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//gluPerspective(60,(GLfloat)width/(GLfloat)height,0.1,2000.0);
	gluPerspective(60, (GLfloat)width / (GLfloat)height, 0.1, 10000.0);
	glMatrixMode(GL_MODELVIEW);
}

void timerFunc(int id)
{
	if (isTimerOn){
		angleHorizontal+=5;
		if (angleHorizontal>360){
			angleHorizontal-=360;
		}
		//repaint
		glutPostRedisplay();
	}
	
}
void writeCharacter(Point3d p,char* str)
{
	//write
	glColor3ub(0,0,0);//black
	int len=strlen(str);
	glRasterPos3d(p.x,p.y,p.z);
	//glRasterPos2d(winX,winY);
	for (int i=0;i<len;i++){
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12,str[i]);
	}
}
void timerSwitch(int option)
{
	switch (option){
		case 1:
			isTimerOn=true;
			break;
		case 2:
			isTimerOn=false;
			break;
		default:
			break;
	}
}
void keyFunc(GLubyte key,int xMouse,int yMouse)
{
	//change radius
	if (key=='-'){//zoom out
		radius+=50;
		glutPostRedisplay();
	}else if (key=='+'){//zoom in
		radius-=50;
		glutPostRedisplay();
	}else if (key=='s'){
		//show locus in images
		drawLocusOnImage();
	}else if (key==27){//esc
		exit(0);
	}	
}
void mouseFunc(int button,int action,int xMouse,int yMouse)
{
	if (button==GLUT_LEFT_BUTTON){
		if (action==GLUT_DOWN){
			oldPoint.x=xMouse;
			oldPoint.y=yMouse;
			isLButtonDown=true;
		}else if (action==GLUT_UP){
			isLButtonDown=false;
		}
	}/*else if (button==GLUT_MIDDLE_BUTTON){
		if (action==GLUT_DOWN){
			oldPoint.x=xMouse;
			oldPoint.y=yMouse;
			isMButtonDown=true;
		}else if (action==GLUT_UP){
			isMButtonDown=false;
		}
	}*/
	
}
void mouseMotionFunc(int xMouse,int yMouse)
{
	if (isLButtonDown){//rotate perspective
		//change eyePos
		angleVertical+=(yMouse-oldPoint.y)*180/winHeight;
		angleHorizontal-=(xMouse-oldPoint.x)*180/winWidth;
		if (angleVertical>360){
			angleVertical-=360;
		}else if (angleVertical<-360){
			angleVertical+=360;
		}
		if (angleHorizontal>360){
			angleHorizontal-=360;
		}else if (angleHorizontal<-360){
			angleHorizontal+=360;
		}
		//update oldPoint
		oldPoint.x=xMouse;
		oldPoint.y=yMouse;
		glutPostRedisplay();
	}//else if (isMButtonDown){//transform
	//	transHorizontal=oldPoint.x-xMouse;
	//	transVertical=oldPoint.y-yMouse;
	//	//update oldPoint
	//	oldPoint.x=xMouse;
	//	oldPoint.y=yMouse;
	//	glutPostRedisplay();
	//}
}

//update position of eye,according to angleHorizontal and angleVertical
void updateEyePosition()
{
	float radVertical=angleVertical*PI/180;
	float radHorizontal=angleHorizontal*PI/180;
	eyePos.y=radius*sin(radVertical);
	float t=radius*cos(radVertical);
	eyePos.x=t*sin(radHorizontal);
	eyePos.z=t*cos(radHorizontal);
}

vector<Mat> ComputeDis(vector<Point3d> & points, Mat matl, Mat matr, vector<Point2d> & left, vector<Point2d> & right){
	Mat diffl(points.size(), 2, CV_64F), diffr(points.size(), 2, CV_64F);
	for (int i = 0; i < points.size(); i++){
		Mat w3d(4, 1, CV_64F);
		w3d.at<double>(0, 0) = points[i].x;
		w3d.at<double>(1, 0) = points[i].y;
		w3d.at<double>(2, 0) = points[i].z;
		w3d.at<double>(3, 0) = 1.0;
		Mat resl = matl * w3d;
		diffl.at<double>(i, 0) = resl.at<double>(0, 0) / resl.at<double>(2, 0) - left[i].x;
		diffl.at<double>(i, 1) = resl.at<double>(1, 0) / resl.at<double>(2, 0) - left[i].y;

		Mat resr = matr * w3d;
		diffr.at<double>(i, 0) = resr.at<double>(0, 0) / resr.at<double>(2, 0) - right[i].x;
		diffr.at<double>(i, 1) = resr.at<double>(1, 0) / resr.at<double>(2, 0) - right[i].y;
	}
	return vector<Mat>{diffl, diffr};
}

void main(int argc, char** argv){
	cout << "s\tshow locus on images and save them" << endl
		<< "+\tzoom in" << endl
		<< "-\tzoom out" << endl
		<< "ESC\tquit" << endl;

	vector<string> datafile(2);
	if (argc == 3){
		datafile[0] = argv[1];
		datafile[1] = argv[2];
	}
	else{
		// 		datafile[0]="../data/videos/video_cv_glove_0.avi.yml";
		// 		datafile[1]="../data/videos/video_cv_glove_1.avi.yml";
		datafile[0] = "../data/videos/video0.avi.yml";
		datafile[1] = "../data/videos/video1.avi.yml";
	}

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(winWidth, winHeight);
	glutCreateWindow("Locus");

	//points
	get3dPoints(pointVec, datafile[0], datafile[1]);

	init();
	//menu
	glutCreateMenu(timerFunc);
	glutAddMenuEntry("Turn On Timer", 1);
	glutAddMenuEntry("Turn Off Timer", 2);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	glutDisplayFunc(display);
	glutReshapeFunc(winReshapeFcn);
	//glutIdleFunc(timerFunc);
	glutKeyboardFunc(keyFunc);
	glutMouseFunc(mouseFunc);
	glutMotionFunc(mouseMotionFunc);
	glutTimerFunc(timerInterval, &timerFunc, 0);

	glutMainLoop();

}