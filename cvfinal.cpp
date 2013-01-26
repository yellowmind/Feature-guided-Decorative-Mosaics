#include <stdio.h>
#include <iostream>
#include <cmath>
#include <time.h>
#include <algorithm>
#include <vector>

//#include "GLee.h"
//#include <GL/glut.h>
#include <cv.h>
#include <highgui.h>

#include "delaunay.h"
#include "cvfinal.h"

using namespace std;

IplImage *R, *G, *B;
IplImage *ImageC, *GradientX, *GradientY, *Laplace, *Canny, *DF,*GradientZ, *DF2, *Skeleton;


//class Point
//{
//	public: 
//		int x,y;
//};
//
//vector<Point> pointList;

void randomP(int n)
{
	srand(time(NULL));
	for(int i=0; i<n; i++)
	{
		Point p;
		p.x=(rand()%winWidth);
		p.y=(rand()%winHeight);
		pointList.push_back(p);
	}
}

void drawTile(float size)
{
	static bool intial=false;
	static bool test=true;
	
	/*if(intial == false)
	{
		randomP(5);
		intial =true;
	}*/
	
	float colorVar = 1 / ( pow( (float)pointList.size(), (float)1/3) + 1);
	int r=0,g=0,b=0;

	for(int i=0; i<pointList.size(); i++)
	{
		int x = pointList.at(i).x;
		int y = pointList.at(i).y;

		double gradientX = cvGetReal2D(GradientX, y, x);
		double gradientY = cvGetReal2D(GradientY, y, x);
		double gradientZ = sqrt( pow(gradientX, 2) +  pow(gradientY, 2) );
		if( gradientX * gradientY < 0)
		{
			gradientZ *= -1;
		}

		double alpha = atan2(gradientY, gradientX);
		double beta = atan2(gradientZ, 1/cos(alpha));
		
		glPushMatrix();
			glTranslated(-winWidth/2 + x, -winHeight/2 + y, 0);
			

			r += colorVar;
			
			/*if(r >= 1)
			{
				r -= 1;
				g += colorVar;
			}
			if(g >= 1)
			{
				g -= 1;
				b += colorVar;
			}
			glColor3f(r, g, b);*/

			

			r = cvGetReal2D(R, y, x);
			g = cvGetReal2D(G, y, x);
			b = cvGetReal2D(B, y, x);
			glColor3f((float)(r)/255, (float)(g)/255, (float)(b)/255);

			/*if(i == pointList.size() - 1)
			{
				test = false;
				cout << colorVar << endl;
			}
			if(test)
			{
				cout << r <<" " << g << " " <<b << endl;
			}*/
			
			
			glPushMatrix();
				glRotated(45 + alpha/M_PI*180, 0, 0, 1);
				//glRotated(beta/M_PI*180, 1, 0, 0);
				glTranslated(0, 0, -winWidth);
				glutSolidCone( size, winWidth/sqrt( (float)5 ), 4, 1);	
			glPopMatrix();

			//glColor3f(1, 1, 1);
			//glBegin(GL_POINTS);
				//glTranslated(0, 0, winWidth*2);
				//glVertex3i( 0, 0, winWidth*3/2);
				//glutSolidCone(2, 2, 4, 1);	
			//glEnd();

		glPopMatrix();
	}
	
	//glutSolidCone(winWidth, winHeight*2, 4, 1);
}

void extractLine(IplImage *src, IplImage *dst, int interval, int linewidth)
{
	int m = cvGetSize(src).width;
	int n = cvGetSize(src).height;
	
	cvSet( dst, cvScalar(255) );

	int value =255;
	
	for(int level=0; level<255; level++)
	{
		for(int i=0; i<n; i++)
		{
			for(int j=0; j<m; j++)
			{
				value = cvGetReal2D(src, i, j);
				if(level*interval <= value && level*interval+linewidth > value)
				{
					cvSetReal2D(dst, i, j, 0);
				}
			}
		}
	}
}

int main(int argc, char **argv)
{
	
	IplImage *Image1, *ImageX, *ImageY, *ImageE, *ImageD, *ImageL, *ImageV, *ImageD2,*ImageS;
	char filename[] = "fuckbook.jpg";
	ImageC = cvLoadImage( filename, 1);
	R = cvCreateImage(cvGetSize(ImageC), ImageC->depth, 1);
	G = cvCreateImage(cvGetSize(ImageC), ImageC->depth, 1);
	B = cvCreateImage(cvGetSize(ImageC), ImageC->depth, 1);
	cvSplit(ImageC, B, G, R, 0);

	Image1 = cvLoadImage( filename, 0);
	int m = cvGetSize(Image1).width;
	int n = cvGetSize(Image1).height;
	ImageX = cvCreateImage( cvGetSize(Image1), Image1->depth, Image1->nChannels );
	ImageY = cvCreateImage( cvGetSize(Image1), Image1->depth, Image1->nChannels );
	ImageE = cvCreateImage( cvGetSize(Image1), Image1->depth, Image1->nChannels );
	ImageD = cvCreateImage( cvGetSize(Image1), Image1->depth, Image1->nChannels );
	ImageL = cvCreateImage( cvGetSize(Image1), Image1->depth, Image1->nChannels );
	ImageV = cvCreateImage( cvGetSize(Image1), 8, 3 );
	ImageD2 = cvCreateImage( cvGetSize(Image1), Image1->depth, Image1->nChannels );
	ImageS = cvCreateImage( cvGetSize(Image1), Image1->depth, Image1->nChannels );
	
	cout << "0" << std::endl;
	GradientX = cvCreateImage( cvGetSize(Image1), IPL_DEPTH_16S, Image1->nChannels );
	GradientY = cvCreateImage( cvGetSize(Image1), IPL_DEPTH_16S, Image1->nChannels );

	Laplace =  cvCreateImage( cvGetSize(Image1), IPL_DEPTH_16S, Image1->nChannels );
	Canny =  cvCreateImage( cvGetSize(Image1), IPL_DEPTH_8U, Image1->nChannels );
	cvLaplace(Image1, Laplace);

	DF = cvCreateImage( cvGetSize(Image1), IPL_DEPTH_32F, Image1->nChannels );
	DF2 = cvCreateImage( cvGetSize(Image1), IPL_DEPTH_32F, Image1->nChannels );
	GradientZ = cvCreateImage( cvGetSize(Image1), IPL_DEPTH_16S, Image1->nChannels );
	

	cout << "1" << std::endl;
   
	//cvSobel( Image1, GradientX, 1, 0);
	//cvSobel( Image1, GradientY, 0, 1);
	
	cout << "2" << std::endl;
	
	/*int value=0,max=0,min=0;
	for(int i=0; i<n; i++)
	{
		for(int j=0; j<m; j++)
		{
			value = cvGetReal2D(GradientX, i, j);
			if(value > max)
				max = value;
			if(value < min)
				min = value;
		}
	}
	cout << "max:" << max << std::endl;
	cout << "min:" << min << std::endl;*/
	

	cvConvertScaleAbs(Laplace, ImageE, 1,127);
	cvThreshold(ImageE, ImageE, 253, 255, CV_THRESH_BINARY_INV);

	cvCanny(Image1, Canny, 0, 150);
	cvThreshold(Canny, ImageE, 250, 255, CV_THRESH_BINARY_INV);

	cout << "3" << std::endl;
	int nSize=3;
	IplConvKernel* circularElem =
	cvCreateStructuringElementEx( nSize, // columns
	nSize, // rows
	floor(nSize/2.0), // anchor_x
	floor(nSize/2.0), // anchor_y
	CV_SHAPE_ELLIPSE, // shape
	NULL);
	cvMorphologyEx( ImageE, ImageE, 0, circularElem, CV_MOP_OPEN, 6 );
	cout << "4" << std::endl;
	cvDistTransform(ImageE, DF);
	cvConvertScaleAbs(DF, ImageD, 1,0);
	cout << "5" << std::endl;
	IplImage *White = cvCreateImage( cvGetSize(Image1), Image1->depth, Image1->nChannels );
	cvSet( White, cvScalar(255) );
	cvSub(White, ImageD, ImageD);
	cvSmooth(ImageD, ImageD, CV_GAUSSIAN, 5);
	cout << "6" << std::endl;
	cvSobel( ImageD, GradientX, 1, 0);
	cvSobel( ImageD, GradientY, 0, 1);
	cvConvertScaleAbs(GradientX, ImageX, 1,127);
	cvConvertScaleAbs(GradientY, ImageY, 1,127);

	int interval = 12,linewidth = 2;

	extractLine(ImageD, ImageL, interval, linewidth);
	
	run(ImageL, ImageV, interval);
	//cout << "a" << std::endl;
	DF2 = cvCreateImage( cvGetSize(Image1), IPL_DEPTH_32F, Image1->nChannels );
	//cout << "b" << std::endl;
	Skeleton=  cvCreateImage( cvGetSize(Image1), IPL_DEPTH_16S, Image1->nChannels );
	//cout << "c" << std::endl;
	cvDistTransform(ImageL, DF2);
	//cout << "d" << std::endl;
	cvConvertScaleAbs(DF2, ImageD2, 1,0);
	cvLaplace(ImageD2, Skeleton);
	//cout << "e" << std::endl;
	
	cvConvertScaleAbs(Skeleton, ImageS, 10,127);
	cvThreshold(ImageS, ImageS,25, 255, CV_THRESH_BINARY);
	//cout << "f" << std::endl;

	tileSize = (interval - linewidth -2)/ sqrt( (float)2 );
	/*int value=0,neighbor=0;
	for(int level =255; level>0; level--)
	{
		for(int i=0; i<n; i++)
		{
			for(int j=0; j<m; j++)
			{
				value = cvGetReal2D(ImageE, i, j);
				if( value == level)
				{
					for(int extY=-1; extY<2; extY++)
					{
						for(int extX=-1; extX<2; extX++)
						{
							if( extY != extX)
							{
								if(i+extY>=0 && j+extX>=0 && i+extY<n && j+extX<m)
								{
									neighbor = cvGetReal2D(ImageE, i+extY, j+extX);
									if( neighbor == 0 )
									{
										cvSetReal2D(ImageE, i+extY, j+extX, value-1);
									}
								}
							}
						}
					}
				}
			}
		}
	}*/
	
	//int m = cvGetSize(Image1).width;
	//int n = cvGetSize(Image1).height;
	//int mn = m*n;

	//double sum[Bins],sum2[Bins];
	//int round[Bins];
	//for(int i=0; i < HistogramBins; i++)
	//{
	//	round[i] = 0;
	//	if( i == 0 )
	//	{
	//		sum[i] =  cvGetReal1D(lHist1->bins, i) * (HistogramBins - 1) / mn;
	//		//cout << "Sum " << sum[i] << std::endl;
	//		sum2[i] =  cvGetReal1D(lHist1->bins, i);
	//	}
	//	else
	//	{
	//		sum[i] = sum[i-1] + cvGetReal1D(lHist1->bins, i) * (HistogramBins - 1) / mn;
	//		sum2[i] = sum2[i-1] + cvGetReal1D(lHist1->bins, i);
	//	}
	//	/*if(i>0 && i<100)
	//	{
	//		cout << "Sum " << sum[i] << std::endl;
	//	}*/
	//	cout << "Sum " << sum2[i] << std::endl;
	//	round[i] = cvRound(sum[i]);
	//	//cout << "Round " << round[i] << std::endl;
	//}
	
	//Image2 = cvCreateImage( cvGetSize(Image1), Image1->depth, Image1->nChannels );
	//Image3 = cvCreateImage( cvGetSize(Image1), Image1->depth, Image1->nChannels );

	//int m = cvGetSize(Image1).width;
	//int n = cvGetSize(Image1).height;
	//for(int i=0; i < n; i++)
	//{
	//	for(int j=0; j < m; j++)
	//	{
	//		//cout << medianFilter(Image1, Image2, i, j).at(0) << endl;
	//		cvSetReal2D( Image3, i, j, aMedianFilter(Image1, i, j) );
	//		cvSetReal2D( Image2, i, j, medianFilter(Image1, i, j, 7).at(0) );
	//		//cvSetReal2D( Image2, i, j, aMedianFilter(Image1, i, j, 7) );
	//		
	//	}
	//}
	
	/*for(int i=0; i < n; i++)
	{
		for(int j=0; j < m; j++)
		{			
			cvSetReal2D(Image2, i, j,
				round[ cvRound( cvGetReal2D(Image1, i, j) ) ]);
		}
	}*/

	/*IplImage *Image3,*Image4,*Image5,*Image6;
	Image3 = cvLoadImage("mp1a.jpg",1);

	IplImage *RedImage = cvCreateImage(cvGetSize(Image3),Image3->depth,1);
    IplImage *GreenImage = cvCreateImage(cvGetSize(Image3),Image3->depth,1);
    IplImage *BlueImage = cvCreateImage(cvGetSize(Image3),Image3->depth,1);

	cvSplit(Image3,BlueImage,GreenImage,RedImage,0);
	
	medianFilter(RedImage, RedImage);
	medianFilter(GreenImage, GreenImage);
	medianFilter(BlueImage, BlueImage);

	Image4 = cvCreateImage( cvGetSize(Image3), Image3->depth, Image3->nChannels );
	cvMerge(BlueImage,GreenImage,RedImage,0,Image4);
	
	Image5 = cvCreateImage( cvGetSize(Image3), Image3->depth, Image3->nChannels );
	cvCvtColor(Image3 ,Image5, CV_BGR2HSV);

	IplImage *HImage = cvCreateImage(cvGetSize(Image5),Image5->depth,1);
    IplImage *SImage = cvCreateImage(cvGetSize(Image5),Image5->depth,1);
    IplImage *VImage = cvCreateImage(cvGetSize(Image5),Image5->depth,1);

	cvSplit(Image5,HImage,SImage,VImage,0);

	medianFilter(VImage, VImage);
	Image6 = cvCreateImage( cvGetSize(Image3), Image3->depth, Image3->nChannels );
	cvMerge(HImage,SImage,VImage,0,Image6);
	cvCvtColor(Image6 ,Image6, CV_HSV2BGR);

	medianFilter(SImage, SImage);
	medianFilter(HImage, HImage);

	cvMerge(HImage,SImage,VImage,0,Image5);
	cvCvtColor(Image5 ,Image5, CV_HSV2BGR);

	 IplImage *Sub46 = cvCreateImage( cvGetSize(Image6), Image6->depth, Image6->nChannels );
	 cvSub(Image4, Image6, Sub46);*/
	/*CvHistogram *lHistR = cvCreateHist(1, &HistogramBins, CV_HIST_ARRAY, HistogramRange, 1);
	CvHistogram *lHistG = cvCreateHist(1, &HistogramBins, CV_HIST_ARRAY, HistogramRange, 1);
	CvHistogram *lHistB = cvCreateHist(1, &HistogramBins, CV_HIST_ARRAY, HistogramRange, 1);*/
	
	/*cvCalcHist( &RedImage, lHistR, 0, NULL );
	cvCalcHist( &GreenImage, lHistG, 0, NULL );
	cvCalcHist( &BlueImage, lHistB, 0, NULL );*/

   /* CvSize Size1=cvGetSize(Image1);
    Image2=cvCreateImage(Size1,IPL_DEPTH_8U,3);
    cvSet(Image2,cvScalar(255,255,255));*/

  /*  MaskImage=cvLoadImage("grotto_Threshold.jpg",0);*/

    /*cvErode(Image1, Image2, NULL, 2);*/

    cvNamedWindow("Show Original Image",1);
	cvNamedWindow("Show GradientX",1);
	//cvNamedWindow("Show Histogram1",1);
	cvNamedWindow("Show GradientY",1);
	cvNamedWindow("Show Edge",1);
	cvNamedWindow("Show Distance Field",1);
	cvNamedWindow("Show Offset Line",1);
	cvNamedWindow("Show Voronoi Diagram",1);
	cvNamedWindow("Show Skeleton",1);
	//cvNamedWindow("Show Histogram1  Equalization",1);
	
	cvShowImage("Show Original Image",Image1);
	cvShowImage("Show GradientX", ImageX);
	//cvShowImage("Show Histogram1",lHistImage1);
	cvShowImage("Show GradientY", ImageY);
	cvShowImage("Show Edge", ImageE);
	cvShowImage("Show Distance Field", ImageD);
	cvShowImage("Show Offset Line", ImageL);
	cvShowImage("Show Voronoi Diagram",ImageV);
	cvShowImage("Show Skeleton", ImageS);
	//cvShowImage("Show Histogram1  Equalization",lHistImage2);
	


	//cvNamedWindow("Show mp1a",1);
	////cvNamedWindow("Show Histogram1a",1);
	//cvNamedWindow("Show mp1a RGB Equalization",1);
	////cvNamedWindow("Show Histogram1a  RGB Equalization",1);
	//cvNamedWindow("Show mp1a HSV Equalization",1);
	////cvNamedWindow("Show Histogram1a  HSV Equalization",1);
	////cvNamedWindow("Show Histogram1a  Luminance Equalization",1);
	//cvNamedWindow("Show mp1a Substration",1);

 //   cvShowImage("Show mp1a",Image3);
	////cvShowImage("Show Histogram1",lHistImage1);
 //   cvShowImage("Show mp1a RGB Equalization",Image4);
	////cvShowImage("Show Histogram1  Equalization",lHistImage2);
	//cvShowImage("Show mp1a HSV Equalization",Image5);
	//cvShowImage("Show mp1a Luminance Equalization",Image6);
	//cvShowImage("Show mp1a Substration",Sub46);

	glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(m, n);
    glutCreateWindow("Voronoi Diagram");

    glutReshapeFunc(myReshape);
    glutDisplayFunc(display);
    glutIdleFunc(spinCube);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);
	glutTimerFunc(33, update, 0);

	glutMainLoop();

	glEnable(GL_DEPTH_TEST); 

    cvWaitKey(0);

	
}