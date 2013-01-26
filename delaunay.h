#include <highgui.h>
#include <stdio.h>

using namespace std;

class Point
{
	public: 
		int x,y;
};

vector<Point> pointList;

void help()
{
	printf("\nThis program demostrates iterative construction of\n"
			"delaunay triangulation and voronoi tesselation.\n"
			"It draws a random set of points in an image and then delaunay triangulates them.\n"
            "Usage: \n"
            "./delaunay \n"
			"\nThis program builds the traingulation interactively, you may stop this process by\n"
			"hitting any key.\n");
}

CvSubdiv2D* init_delaunay( CvMemStorage* storage,
                           CvRect rect )
{
    CvSubdiv2D* subdiv;

    subdiv = cvCreateSubdiv2D( CV_SEQ_KIND_SUBDIV2D, sizeof(*subdiv),
                               sizeof(CvSubdiv2DPoint),
                               sizeof(CvQuadEdge2D),
                               storage );
    cvInitSubdivDelaunay2D( subdiv, rect );

    return subdiv;
}


void draw_subdiv_point( IplImage* img, CvPoint2D32f fp, CvScalar color )
{
    cvCircle( img, cvPoint(cvRound(fp.x), cvRound(fp.y)), 1, color, CV_FILLED, 8, 0 );
}


void draw_subdiv_edge( IplImage* img, CvSubdiv2DEdge edge, CvScalar color )
{
    CvSubdiv2DPoint* org_pt;
    CvSubdiv2DPoint* dst_pt;
    CvPoint2D32f org;
    CvPoint2D32f dst;
    CvPoint iorg, idst;

    org_pt = cvSubdiv2DEdgeOrg(edge);
    dst_pt = cvSubdiv2DEdgeDst(edge);

    if( org_pt && dst_pt )
    {
        org = org_pt->pt;
        dst = dst_pt->pt;

        iorg = cvPoint( cvRound( org.x ), cvRound( org.y ));
        idst = cvPoint( cvRound( dst.x ), cvRound( dst.y ));

        cvLine( img, iorg, idst, color, 1, CV_AA, 0 );
    }
}


void draw_subdiv( IplImage* img, CvSubdiv2D* subdiv,
                  CvScalar delaunay_color, CvScalar voronoi_color )
{
    CvSeqReader  reader;
    int i, total = subdiv->edges->total;
    int elem_size = subdiv->edges->elem_size;

    cvStartReadSeq( (CvSeq*)(subdiv->edges), &reader, 0 );

    for( i = 0; i < total; i++ )
    {
        CvQuadEdge2D* edge = (CvQuadEdge2D*)(reader.ptr);

        if( CV_IS_SET_ELEM( edge ))
        {
            draw_subdiv_edge( img, (CvSubdiv2DEdge)edge + 1, voronoi_color );
            draw_subdiv_edge( img, (CvSubdiv2DEdge)edge, delaunay_color );
        }

        CV_NEXT_SEQ_ELEM( elem_size, reader );
    }
}


void locate_point( CvSubdiv2D* subdiv, CvPoint2D32f fp, IplImage* img,
                   CvScalar active_color )
{
    CvSubdiv2DEdge e;
    CvSubdiv2DEdge e0 = 0;
    CvSubdiv2DPoint* p = 0;

    cvSubdiv2DLocate( subdiv, fp, &e0, &p );

    if( e0 )
    {
        e = e0;
        do
        {
            draw_subdiv_edge( img, e, active_color );
            e = cvSubdiv2DGetEdge(e,CV_NEXT_AROUND_LEFT);
        }
        while( e != e0 );
    }

    draw_subdiv_point( img, fp, active_color );
}


void draw_subdiv_facet( IplImage* img, CvSubdiv2DEdge edge )
{
    CvSubdiv2DEdge t = edge;
    int i, count = 0;
    CvPoint* buf = 0;

    // count number of edges in facet
    do
    {
        count++;
        t = cvSubdiv2DGetEdge( t, CV_NEXT_AROUND_LEFT );
    } while (t != edge );

    buf = (CvPoint*)malloc( count * sizeof(buf[0]));

    // gather points
    t = edge;
    for( i = 0; i < count; i++ )
    {
        CvSubdiv2DPoint* pt = cvSubdiv2DEdgeOrg( t );
        if( !pt ) break;
        buf[i] = cvPoint( cvRound(pt->pt.x), cvRound(pt->pt.y));
        t = cvSubdiv2DGetEdge( t, CV_NEXT_AROUND_LEFT );
    }

    if( i == count )
    {
        CvSubdiv2DPoint* pt = cvSubdiv2DEdgeDst( cvSubdiv2DRotateEdge( edge, 1 ));
        cvFillConvexPoly( img, buf, count, CV_RGB(rand()&255,rand()&255,rand()&255), CV_AA, 0 );
        //cvPolyLine( img, &buf, &count, 1, 1, CV_RGB(0,0,0), 1, CV_AA, 0);
        //draw_subdiv_point( img, pt->pt, CV_RGB(0,0,0));
    }
    free( buf );
}

void paint_voronoi( CvSubdiv2D* subdiv, IplImage* img )
{
    CvSeqReader  reader;
    int i, total = subdiv->edges->total;
    int elem_size = subdiv->edges->elem_size;

    cvCalcSubdivVoronoi2D( subdiv );

    cvStartReadSeq( (CvSeq*)(subdiv->edges), &reader, 0 );

    for( i = 0; i < total; i++ )
    {
        CvQuadEdge2D* edge = (CvQuadEdge2D*)(reader.ptr);

        if( CV_IS_SET_ELEM( edge ))
        {
            CvSubdiv2DEdge e = (CvSubdiv2DEdge)edge;
            // left
            draw_subdiv_facet( img, cvSubdiv2DRotateEdge( e, 1 ));

            // right
            draw_subdiv_facet( img, cvSubdiv2DRotateEdge( e, 3 ));
        }

        CV_NEXT_SEQ_ELEM( elem_size, reader );
    }
}

void paint_line( IplImage* line, IplImage* img )
{
	int m = cvGetSize(img).width;
	int n = cvGetSize(img).height;
	
	int value;
	for(int i=0; i<n; i++)
	{
			for(int j=0; j<m; j++)
			{
				value = cvGetReal2D(line, i, j);
				if(value == 0)
				{
					cvCircle( img, cvPoint(j, i), 1, CV_RGB( 0,0,0), CV_FILLED, 8, 0 );
				}
			}
	}
}

void findNeighbor(IplImage *r, IplImage *g,  IplImage *b, IplImage *dst, int row, int colum)
{
	//cout << "fN1" << endl;
	int m = cvGetSize(r).width;
	int n = cvGetSize(r).height;
	
	int colorR = cvGetReal2D(r, row, colum);
	int colorG = cvGetReal2D(g, row, colum);
	int colorB = cvGetReal2D(b, row, colum);

	/*if( colorR == 0 &&  colorG == 0 && colorB == 0 )
	{
		return;
	}*/

	//cout << "fN2" << endl;

	for(int i=-1; i<2;i++)
	{
		if( row+i<0 || row+i>=n )
		{
			continue;
		}
		for(int j=-1; j<2;j++)
		{
			//cout << "fN3" << endl;
			if( colum+j<0 || colum+j>=m )
			{
				continue;
			}
			if( cvGetReal2D(r, row+i, colum+j) )
			{
				if( cvGetReal2D(r, row+i, colum+j) == colorR )
				{
					if( cvGetReal2D(g, row+i, colum+j) == colorG )
					{
						if( cvGetReal2D(b, row+i, colum+j) == colorB )
						{
							if( cvGetReal2D(dst, row+i, colum+j) == 0)
							{
								//cout << "fN4" << endl;
								cvSetReal2D(dst, row+i, colum+j, 255);
								findNeighbor(r, g, b, dst, row+i, colum+j);
							}
						}
					}
				}
			}
		}
	}
}

void getCentroid(IplImage *src, Point &p)
{
	//cout << "gC1" << endl;
	int m = cvGetSize(src).width;
	int n = cvGetSize(src).height;

	int number = 0;
	int x=0,y=0;
	//cout << "gC2" << endl;
	for(int i=0; i<n; i++)
	{
			for(int j=0; j<m; j++)
			{
				if( cvGetReal2D(src, i, j) == 255)
				{
					x += j;
					y += i;
					number++;
				}
			}
	}
	//cout << "gC3" << endl;
	if(number < 50)
	{
		x=0;
		y=0;
	}
	else
	{
		x /= number;
		y /= number;
	}

	p.x = x;
	p.y = y;

}

void setCentroid(IplImage *img, IplImage *centroid1, IplImage *centroid2)
{
	int m = cvGetSize(img).width;
	int n = cvGetSize(img).height;

	IplImage *r = cvCreateImage(cvGetSize(img), img->depth, 1);
    IplImage *g = cvCreateImage(cvGetSize(img), img->depth, 1);
    IplImage *b = cvCreateImage(cvGetSize(img), img->depth, 1);
	cvSplit(img, b, g, r, 0);

	IplImage *subArea = cvCreateImage( cvGetSize(centroid1), centroid1->depth, 1 );
	cvSetZero(subArea);
	cvSetZero(centroid2);
	
	for(int i=0; i<n; i++)
	{
			for(int j=0; j<m; j++)
			{
				if( cvGetReal2D(centroid1, i, j) )
				{
					if( cvGetReal2D(subArea, i, j)  == 0)
					{
						//subArea = cvCreateImage( cvGetSize(src), src->depth, 1 );
						cvSetZero(subArea);
						findNeighbor( r, g, b, subArea, i, j);
						
						//cout << "d4" << endl;
						Point p;
						getCentroid(subArea, p);
						//cout << "d5" << endl;
						if( cvGetReal2D(r, p.y, p.x) || cvGetReal2D(g, p.y, p.x) || cvGetReal2D(b, p.y, p.x) )
						{
							if( p.x != 0 && p.y != 0  )
							{
								//cout << "px:" << p.x << " py:" << p.y << endl; 
								cvSetReal2D(centroid2, p.y, p.x, 255);
							}
						}
											
						//cout << "i:" << i << " j:" << j << endl;
					}
				}
			}
	}
}

bool equalImg(IplImage *im1, IplImage *im2)
{
	int m = cvGetSize(im1).width;
	int n = cvGetSize(im1).height;

	for(int i=0; i<n; i++)
	{
			for(int j=0; j<m; j++)
			{
				if( cvGetReal2D(im1, i, j) != cvGetReal2D(im2, i, j) )
				{
					cout << "not equal" << endl;
					return false;
				}
			}
	}
	return true;
}

void run(IplImage *src, IplImage *dst,int interval=16)
{
    int m = cvGetSize(src).width;
	int n = cvGetSize(src).height;
	
	char win[] = "source";
    CvRect rect = { 0, 0, m, n };
    CvMemStorage* storage;
    CvSubdiv2D* subdiv;
    IplImage* img;
    CvScalar active_facet_color, delaunay_color, voronoi_color, bkgnd_color;

    active_facet_color = CV_RGB( 255, 0, 0 );
    delaunay_color  = CV_RGB( 0,0,0);
    voronoi_color = CV_RGB(0, 180, 0);
    bkgnd_color = CV_RGB(255,255,255);

    img = cvCreateImage( cvGetSize(src), 8, 3 );
    cvSet( img, bkgnd_color, 0 );

    cvNamedWindow( "centroid", 1 );
	cvNamedWindow( "centroid2", 1 );
	cvNamedWindow( win, 1 );

    storage = cvCreateMemStorage(0);
    subdiv = init_delaunay( storage, rect );

	IplImage *centroid = cvCreateImage( cvGetSize(src), src->depth, 1 );
	IplImage *centroid2 = cvCreateImage( cvGetSize(src), src->depth, 1 );
	cvSetZero(centroid);

    printf("Delaunay triangulation will be build now interactively.\n"
           "To stop the process, press any key\n\n");
	int npoint = m / interval ;
	//cout << "d1" << endl;
    for(int i = 1; i <= n; i+=interval )
    {
		for(int j = 1; j <= m; j+=interval ) 
		{
		/*CvPoint2D32f fp = cvPoint2D32f( (float)(rand()%(m-10)+5),
                                        (float)(rand()%(n-10)+5));*/
			if( cvGetReal2D(src, i, j) )
			{
				cvSetReal2D(centroid, i, j, 255);

				CvPoint2D32f fp = cvPoint2D32f( (float)(j), (float)(i));
				locate_point( subdiv, fp, img, active_facet_color );
				cvShowImage( win, img );

				/*if( cvWaitKey( 100 ) >= 0 )
					break;*/

				cvSubdivDelaunay2DInsert( subdiv, fp );

			   /* if( cvWaitKey( 100 ) >= 0 )
					break;*/
			}  
		}
    }
	//cout << "d2" << endl;
	cvCalcSubdivVoronoi2D( subdiv );
	cvSet( img, bkgnd_color, 0 );
	draw_subdiv( img, subdiv, delaunay_color, voronoi_color );
	cvShowImage( win, img );

    cvSet( img, bkgnd_color, 0 );
    paint_voronoi( subdiv, img );
	
	paint_line(src, img);
	int value;
	//for(int i=0; i<n; i++)
	//{
	//		for(int j=0; j<m; j++)
	//		{
	//			value = cvGetReal2D(src, i, j);
	//			if(value == 0)
	//			{
	//				cvCircle( img, cvPoint(j, i), 1, CV_RGB( 0,0,0), CV_FILLED, 8, 0 );
	//			}
	//		}
	//}
	cvShowImage( win, img );
	
	//IplImage *subArea = cvCreateImage( cvGetSize(src), src->depth, 1 );
	
	//cvSetZero(subArea);
	//cvSetZero(centroid);
	//cvSetZero(centroid2);

	//cout << "d3" << endl;

	/***** update centroid *****/
	setCentroid(img, centroid, centroid2);
	/*for(int i=0; i<n; i++)
	{
			for(int j=0; j<m; j++)
			{
				if( cvGetReal2D(src, i, j) && cvGetReal2D(subArea, i, j)  == 0 )
				{
					//subArea = cvCreateImage( cvGetSize(src), src->depth, 1 );
					cvSetZero(subArea);
					findNeighbor( r, g, b, subArea, i, j);
					
					//cout << "d4" << endl;
					Point p;
					getCentroid(subArea, p);
					//cout << "d5" << endl;
					if(p.x != 0 && p.y != 0)
					{
						//cout << "px:" << p.x << " py:" << p.y << endl; 
						cvSetReal2D(centroid, p.y, p.x, 255);
					}
										
					//cout << "i:" << i << " j:" << j << endl;
				}
			}
	}*/
	cvShowImage( "centroid", centroid );
	//cvCopy(centroid, centroid2);
	
	IplImage *centroidTmp = cvCreateImage( cvGetSize(src), src->depth, 1 );
	
	bool converge = false;
	/*while( !converge )
	{*/
	/*for(int i=0;i<=40; i++)
	{*/
		cout << "!converge" << endl;
		cvReleaseMemStorage( &storage );
		storage = cvCreateMemStorage(0);
		subdiv = init_delaunay( storage, rect );
		
		cvCopy(centroid, centroidTmp);
		cvCopy(centroid2, centroid);
		for(int i=0; i<n; i++)
		{
				for(int j=0; j<m; j++)
				{
						if( cvGetReal2D(centroid, i, j) /*&& cvGetReal2D(src, i, j)*/ )
						{
							CvPoint2D32f fp = cvPoint2D32f( (float)(j), (float)(i));
							locate_point( subdiv, fp, img, active_facet_color );
							//cvShowImage( win, img );

							cvSubdivDelaunay2DInsert( subdiv, fp );
							cvCalcSubdivVoronoi2D( subdiv );
							cvSet( img, bkgnd_color, 0 );
							draw_subdiv( img, subdiv, delaunay_color, voronoi_color );
						}  
				}
		}
		//cvShowImage( win, img );

		cvSet( img, bkgnd_color, 0 );
		paint_voronoi( subdiv, img );
		paint_line(src, img);
		cvShowImage( win, img );
		
		setCentroid(img, centroidTmp, centroid2);
		cvShowImage( "centroid2", centroid2 );

		converge = equalImg(centroid, centroid2);
		cout << converge << endl;
	/*}*/
	/***** update centroid *****/
	//cout << "d6" << endl;

    //cvShowImage( win, centroid );

	paint_voronoi( subdiv, dst );
	paint_line(src, img);

	for(int i=0; i<n; i++)
	{
				for(int j=0; j<m; j++)
				{
						if( cvGetReal2D(centroid2, i, j)  )
						{
							Point p;
							p.x = j;
							p.y = i;
							pointList.push_back(p);
						}  
				}
	}
	//int value;
	/*for(int i=0; i<n; i++)
	{
			for(int j=0; j<m; j++)
			{
				value = cvGetReal2D(src, i, j);
				if(value == 0)
				{
					cvCircle( dst, cvPoint(j, i), 1, CV_RGB( 0,0,0), CV_FILLED, 8, 0 );
				}
			}
	}*/
	//*dst = *img;

   // cvWaitKey(0);

    /*cvReleaseMemStorage( &storage );
    cvReleaseImage(&img);
    cvDestroyWindow( win );*/
}

//int main( int argc, char** argv )
//{
//#ifdef _MSC_VER
//    argc, argv;
//#endif
//    help();
//    run();
//    return 0;
//}
//
//#ifdef _EiC
//main( 1, "delaunay.c" );
//#endif
