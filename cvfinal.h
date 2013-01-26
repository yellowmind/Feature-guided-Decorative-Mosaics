//#include "GLee.h"
#include <GL/glut.h>
#include "font.h"

#ifndef M_PI
#define M_PI 3.14159
#endif

int winWidth, winHeight;
float tileSize;

float 	angle = 0.0, axis[3], trans[3];
GLdouble TRACKM[16]={1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
bool 	trackingMouse = false;
bool 	redrawContinue = false;
bool    trackballMove = false;
GLdouble Angle1=0, Angle2=0;
GLint TICK=0;

GLFONT *Font;

GLfloat vertices[][3] = {
    {-1.0,-1.0,-1.0},{1.0,-1.0,-1.0}, {1.0,1.0,-1.0}, {-1.0,1.0,-1.0}, 
    {-1.0,-1.0,1.0}, {1.0,-1.0,1.0}, {1.0,1.0,1.0}, {-1.0,1.0,1.0}
};

void drawTile(float size = 5);

void myReshape(int w, int h)
{
    winWidth = w;
    winHeight = h;
}

void OpenglLine(GLdouble x1, GLdouble y1, GLdouble z1, GLdouble x2, GLdouble y2, GLdouble z2)
{
	glBegin(GL_LINES);
		glVertex3f(x1, y1, z1);
		glVertex3f(x2, y2, z2);
	glEnd();
}

void OpenglLine(int index1, int index2)
{
	OpenglLine(vertices[index1][0], vertices[index1][1], vertices[index1][2],
		       vertices[index2][0], vertices[index2][1], vertices[index2][2]);
}

void WireQuad(int a, int b, int c , int d, bool USINGOPENGL)
{
	if(USINGOPENGL) {
		OpenglLine(a, b);
		OpenglLine(b, c);
		OpenglLine(c, d);
		OpenglLine(d, a);
	} 
}

void glWireCube(void)
{
    // map vertices to faces */
    WireQuad(1,0,3,2, true);
    WireQuad(3,7,6,2, true);
    WireQuad(7,3,0,4, true);
    WireQuad(2,6,5,1, true);
    WireQuad(4,5,6,7, true);
    WireQuad(5,4,0,1, true);
}

void openglPath(void)
{
    //view transform
	glViewport(0, 0, winWidth, winHeight);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	glOrtho(-winWidth*0.5, winWidth*0.5, -winHeight*0.5, winHeight*0.5, -winWidth*3, winWidth*3);
	//gluPerspective(60, (GLfloat)(winWidth*0.5)/winHeight, 0.1, 25); 

    glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, 0, 0, 0, 0, -1*winWidth, 0, 1, 0);
	//multiple trackball matrix
	glMultMatrixd(TRACKM);

	glPushMatrix();
		glClearColor(0, 0, 0, 1.0);
		glScaled(1, 1, 1);
		glColor3f(1, 0, 0);
		//glRotated(180, 0, 1, 0);
		drawTile( tileSize );
	glPopMatrix();

		
	glTranslated(0, 2, 0);
	glRotated(Angle1, 0, 0, 1);
	glRotated(Angle2, 0, 1, 0);
		glPushMatrix();
			glTranslated(0, 2, 0);
			glScaled(1, 2, 1);
			glColor3f(0, 1, 0);
			//glWireCube();
		glPopMatrix();

		glTranslated(0, 4, 0);
		glRotated(Angle1, 0, 0, 1);
		
		glPushMatrix();
			glTranslated(0, 2, 0);
			glScaled(1, 2, 1);
			glColor3f(0, 0, 1);
			//glWireCube();
		glPopMatrix();

}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    //Font
	glColor3f(1.0, 1.0, 1.0);  //set font color
	//glWindowPos2i(10, winHeight-20);    //set font start position
    FontPrintf(Font, 1, "OpenGL");  //print font in screen
	

	/*	test case	*/
    /*GLdouble ma[16]={1,1,4,1,2,1,3,1,3,1,2,1,4,1,1,1};
    GLdouble mb[16]={1,2,3,4,4,3,2,1,1,1,1,1,1,1,1,1};
	swMatrixMode(GL_MODELVIEW);
    swLoadMatrixf(ma);
    swMultMatrixf(mb); 
    swLoadIdentitd();
	swPushMatrix();
    	swRotated(90,0,0,1);
        swPushMatrix();
            swTranslated(0,0,10);
        swPopMatrix();
	swPopMatrix();
    //swFrustum(-25*sqrt(2),25*sqrt(2),-25,25,25,150);
    //swuPerspective(60, sqrt(2),25 ,150);
    printCTM(Font, winWidth, winHeight);*/
	/*	test case	*/

    if (trackballMove) {
		glPushMatrix();
			glLoadMatrixd(TRACKM);
			glRotatef(angle, axis[0], axis[1], axis[2]);
			glGetDoublev(GL_MODELVIEW_MATRIX, TRACKM);
		glPopMatrix();	    
	}

	openglPath();

    glutSwapBuffers();
}

void spinCube()
{
    if (redrawContinue) glutPostRedisplay();
}

float lastPos[3] = {0.0F, 0.0F, 0.0F};
int curx, cury;
int startX, startY;

void trackball_ptov(int x, int y, int width, int height, float v[3])
{
    float d, a;

    /* project x,y onto a hemi-sphere centered within width, height */
    v[0] = (2.0F*x - width) / width;
    v[1] = (height - 2.0F*y) / height;
    d = (float) sqrt(v[0]*v[0] + v[1]*v[1]);
    v[2] = (float) cos((M_PI/2.0F) * ((d < 1.0F) ? d : 1.0F));
    a = 1.0F / (float) sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    v[0] *= a;
    v[1] *= a;
    v[2] *= a;
}

void startMotion(int x, int y)
{
    trackingMouse = true;
    redrawContinue = false;
    startX = x; startY = y;
    curx = x; cury = y;
    trackball_ptov(x, y, winWidth, winHeight, lastPos);
	trackballMove=true;
}

void stopMotion(int x, int y)
{
	trackingMouse = false;

    if (startX != x || startY != y) {
		redrawContinue = true;
    } else {
		angle = 0.0F;
		redrawContinue = false;
		trackballMove = false;
    }
}

void mouseButton(int button, int state, int x, int y)
{
	if(button==GLUT_RIGHT_BUTTON) exit(0);
	if(button==GLUT_LEFT_BUTTON) switch(state) 
	{
		case GLUT_DOWN:
			y=winHeight-y;
			startMotion(x, y);
			break;
		case GLUT_UP:
			stopMotion(x, y);
			break;
    } 
}

void mouseMotion(int x, int y)
{
    float curPos[3], dx, dy, dz;

    trackball_ptov(x, y, winWidth, winHeight, curPos);
	if(trackingMouse)
	{
		dx = curPos[0] - lastPos[0];
		dy = curPos[1] - lastPos[1];
		dz = curPos[2] - lastPos[2];

		if (dx || dy || dz) {
			angle = 90.0F * sqrt(dx*dx + dy*dy + dz*dz);

			axis[0] = lastPos[1]*curPos[2] - lastPos[2]*curPos[1];
			axis[1] = lastPos[2]*curPos[0] - lastPos[0]*curPos[2];
			axis[2] = lastPos[0]*curPos[1] - lastPos[1]*curPos[0];

			lastPos[0] = curPos[0];
			lastPos[1] = curPos[1];
			lastPos[2] = curPos[2];
		}
	} 
    glutPostRedisplay();
}

void update(int i)
{
	TICK++;
	int temp=TICK%180;
	if(temp<90)
		Angle1++;
	else
		Angle1--;

	//int temp2=TICK%90;
	if(temp<90)
		Angle2+=0.5;
	else
		Angle2-=0.5;

	glutPostRedisplay();
	glutTimerFunc(33, update, ++i);
}