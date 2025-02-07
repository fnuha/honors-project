#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define _USE_MATH_DEFINES
#include <math.h>

#ifndef F_PI
#define F_PI		((float)(M_PI))
#define F_2_PI		((float)(2.f*F_PI))
#define F_PI_2		((float)(F_PI/2.f))
#endif


#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#endif

#include "glew.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include "glut.h"

#define PICK_TOL 10.
#define PICK_BUFFER_SIZE 256

//	Author:			Faaizah Nuha, adapted from Mike Bailey

// title of these windows:

const char *WINDOWTITLE = "OpenGL / GLUT Sample -- Faaizah Nuha";
const char *GLUITITLE   = "User Interface Window";

// what the glui package defines as true and false:

const int GLUITRUE  = true;
const int GLUIFALSE = false;

// the escape key:

const int ESCAPE = 0x1b;

// initial window size:

const int INIT_WINDOW_SIZE = 600;

// size of the 3d box to be drawn:

const float BOXSIZE = 2.f;

// multiplication factors for input interaction:
//  (these are known from previous experience)

const float ANGFACT = 1.f;
const float SCLFACT = 0.005f;

// minimum allowable scale factor:

const float MINSCALE = 0.05f;

// scroll wheel button values:

const int SCROLL_WHEEL_UP   = 3;
const int SCROLL_WHEEL_DOWN = 4;

// equivalent mouse movement when we click the scroll wheel:

const float SCROLL_WHEEL_CLICK_FACTOR = 5.f;

// active mouse buttons (or them together):

const int LEFT   = 4;
const int MIDDLE = 2;
const int RIGHT  = 1;

// which projection:

enum Projections
{
	ORTHO,
	PERSP
};

// which button:

enum ButtonVals
{
	RESET,
	QUIT
};

// window background color (rgba):

const GLfloat BACKCOLOR[ ] = { 0., 0., 0., 1. };

// line width for the axes:

const GLfloat AXES_WIDTH   = 3.;

// the color numbers:
// this order must match the radio button order, which must match the order of the color names,
// 	which must match the order of the color RGB values

enum Colors
{
	RED,
	YELLOW,
	GREEN,
	CYAN,
	BLUE,
	MAGENTA
};

char * ColorNames[ ] =
{
	(char *)"Red",
	(char*)"Yellow",
	(char*)"Green",
	(char*)"Cyan",
	(char*)"Blue",
	(char*)"Magenta"
};

// the color definitions:
// this order must match the menu order

const GLfloat Colors[ ][3] = 
{
	{ 1., 0., 0. },		// red
	{ 1., 1., 0. },		// yellow
	{ 0., 1., 0. },		// green
	{ 0., 1., 1. },		// cyan
	{ 0., 0., 1. },		// blue
	{ 1., 0., 1. },		// magenta
};

// fog parameters:

const GLfloat FOGCOLOR[4] = { .0f, .0f, .0f, 1.f };
const GLenum  FOGMODE     = GL_LINEAR;
const GLfloat FOGDENSITY  = 0.30f;
const GLfloat FOGSTART    = 1.5f;
const GLfloat FOGEND      = 4.f;

// for lighting:

const float	WHITE[ ] = { 1.,1.,1.,1. };

// for animation:

const int MS_PER_CYCLE = 10000;		// 10000 milliseconds = 10 seconds


// what options should we compile-in?
// in general, you don't need to worry about these
// i compile these in to show class examples of things going wrong
//#define DEMO_Z_FIGHTING
//#define DEMO_DEPTH_BUFFER

// row goes across --> > >
// column goes down  V V V


// non-constant global variables:

int		ActiveButton;			// current button that is down
GLuint	AxesList;				// list to hold the axes
int		AxesOn;					// != 0 means to draw the axes
//GLuint	ObjList;				// object display list
//GLuint	PickedObjList;			// picked object display list
//GLuint	LineList;				// line display list
int		DebugOn;				// != 0 means to print debugging info
int		DepthCueOn;				// != 0 means to use intensity depth cueing
int		DepthBufferOn;			// != 0 means to use the z-buffer
int		DepthFightingOn;		// != 0 means to force the creation of z-fighting
int		MainWindow;				// window id for main graphics window
int		NowColor;				// index into Colors[ ]
int		NowProjection;		// ORTHO or PERSP
float	Scale;					// scaling factor
int		ShadowsOn;				// != 0 means to turn shadows on
float	Time;					// used for animation, this has a value between 0. and 1.
int		Xmouse, Ymouse;			// mouse values
float	Xrot, Yrot;				// rotation angles in degrees

// function prototypes:

void	Animate( );
void	Display( );
void	DoAxesMenu( int );
void	DoColorMenu( int );
void	DoDepthBufferMenu( int );
void	DoDepthFightingMenu( int );
void	DoDepthMenu( int );
void	DoDebugMenu( int );
void	DoMainMenu( int );
void	DoProjectMenu( int );
void	DoRasterString( float, float, float, char * );
void	DoStrokeString( float, float, float, float, char * );
float	ElapsedSeconds( );
void	InitGraphics( );
void	InitLists( );
void	InitMenus( );
void	Keyboard( unsigned char, int, int );
void	MouseButton( int, int, int, int );
void	MouseMotion( int, int );
void	Reset( );
void	Resize( int, int );
void	Visibility( int );

void			Axes( float );
// utility to create an array from 3 separate values:

float *
Array3( float a, float b, float c )
{
	static float array[4];

	array[0] = a;
	array[1] = b;
	array[2] = c;
	array[3] = 1.;
	return array;
}

// utility to create an array from a multiplier and an array:

float *
MulArray3( float factor, float array0[ ] )
{
	static float array[4];

	array[0] = factor * array0[0];
	array[1] = factor * array0[1];
	array[2] = factor * array0[2];
	array[3] = 1.;
	return array;
}


float *
MulArray3(float factor, float a, float b, float c )
{
	static float array[4];

	float* abc = Array3(a, b, c);
	array[0] = factor * abc[0];
	array[1] = factor * abc[1];
	array[2] = factor * abc[2];
	array[3] = 1.;
	return array;
}

// these are here for when you need them -- just uncomment the ones you need:

#include "setmaterial.cpp"
#include "keytime.cpp"


Keytimes NodeZ;


//changeable variables
float	Weight = 0.4;
float	LENGTH0 = 0.2;
int		rows = 10;
int		cols = 10;
float	k = 5.;
float	damp = 3.;
int		ptSize = 4;
float	gapSize = 2.;
bool	SomethingPicked;
int		Nhits;

struct derivs {
	float vel[3];
	float acc[3];
};

struct nodeState {
	float pos[3];
	float vel[3];
	float acc[3];
	bool isPicked;
};

//struct node {
//	struct nodeState nodeState;
//	struct derivs derivs;
//};

//float nodetime;

nodeState** stateList; //[rows >][cols V]
derivs** derivList;
int		Freeze = 0; //0 means no freeze, 1 means yes freeze

unsigned int PickBuffer[PICK_BUFFER_SIZE];
int RenderMode; //GL_RENDER vs GL_SELECT mode

void getOneDDerivs() {

		for (int i = 1; i < rows; i++) { // plus one row v
			for (int j = 0; j < cols; j++) { // plus one col >
				float sumfy = -Weight; // downwards weight
				float ym = stateList[i - 1][j].pos[1] - stateList[i][j].pos[1]; // prev node pos minus curr node pos to find upwards force
				float stretch = ym - LENGTH0; // upwards stretch minus default stretch w no forces
				sumfy += k * stretch; //adding stretch upwards to downwards weight
				sumfy -= damp * derivList[i][j].vel[1]; //adding velocity to stretch
				
				float sumfx = 0; //no downwards weight
				float xm = stateList[i-1][j].pos[0] - stateList[i][j].pos[0]; //prev node minus curr node pos
				stretch = xm; //horizontal force
				sumfx += k * stretch; //adding stretch to movement
				sumfx -= damp * derivList[i][j].vel[0]; //adding velocity to stretch

				float sumfz = 0;
				float zm = stateList[i - 1][j].pos[2] - stateList[i][j].pos[2];
				stretch = zm;
				sumfz += k * stretch;
				sumfz -= damp * derivList[i][j].vel[2];

				if (i > 0 && j < cols - 1) { //if past first row and not at end col
					float dx = stateList[i - 1][j + 1].pos[0] - stateList[i][j].pos[0];
					float dy = stateList[i - 1][j + 1].pos[1] - stateList[i][j].pos[1];
					float dz = stateList[i - 1][j + 1].pos[2] - stateList[i][j].pos[2];

					float length = sqrt(dx * dx + dy * dy + dz * dz);
					dx /= length;
					dy /= length;
					dz /= length;
					float stretch2 = length - LENGTH0;
					float force = 0.5 * stretch2;
					sumfx += force * dx;
					sumfy += force * dy;
					sumfz += force * dz;

				}

				if (i < rows - 1 && j > 0) { // if not at bottom row and not at first col
					float dx = stateList[i + 1][j - 1].pos[0] - stateList[i][j].pos[0];
					float dy = stateList[i + 1][j - 1].pos[1] - stateList[i][j].pos[1];
					float dz = stateList[i + 1][j - 1].pos[2] - stateList[i][j].pos[2];

					float length = sqrt(dx * dx + dy * dy + dz * dz);
					dx /= length;
					dy /= length;
					dz /= length;
					float stretch2 = length - LENGTH0;
					float force = 0.5 * stretch2;
					sumfx += force * dx;
					sumfy += force * dy;
					sumfz += force * dz;

				}

				derivList[i][j].vel[0] = stateList[i][j].vel[0]; //updating vel
				derivList[i][j].acc[0] = sumfx / 1; //updating acc
				derivList[i][j].vel[1] = stateList[i][j].vel[1]; //updating velocity
				derivList[i][j].acc[1] = sumfy / 1; //updating acceleration
				derivList[i][j].vel[2] = stateList[i][j].vel[2];
				derivList[i][j].acc[2] = sumfz / 1;
			}
		}


}


void advOneTimeStep() {
	getOneDDerivs();
	for (int i = 1; i < rows; i++) { // plus one row V
		for (int j = 0; j < cols; j++) { // plus one col >
			for (int k = 0; k < 3; k++) { // three dimensions x y and z
				stateList[i][j].pos[k] = stateList[i][j].pos[k] + (derivList[i][j].vel[k] * 0.05);
				stateList[i][j].vel[k] = stateList[i][j].vel[k] + (derivList[i][j].acc[k] * 0.05);
			}

		}
	}

}

void InitArray(int cols, int rows) {

	stateList = new nodeState* [rows];
	derivList = new derivs* [rows];

	for (int i = 0; i < rows; i++) {
		derivList[i] = new derivs[cols];
		stateList[i] = new nodeState[cols];
	}

	for (int i = 0; i < rows; i++) { //plus one cols >
		for (int j = 0; j < cols; j++) { // plus one row V
			for (int k = 0; k < 3; k++) { //x, y, z
				// three dimensions x y and z

				stateList[i][j].vel[k] = 0;

				stateList[i][j].acc[k] = 0;

				derivList[i][j].vel[k] = 0;
				derivList[i][j].acc[k] = 0;

				stateList[i][j].isPicked = false;
			}
			stateList[i][j].pos[0] = j;
			stateList[i][j].pos[1] = -i;
			stateList[i][j].pos[2] = 0;
			stateList[i][j].isPicked = false;
		}
	}
}


// main program:

int
main( int argc, char *argv[ ] )
{
	// turn on the glut package:
	// (do this before checking argc and argv since glutInit might
	// pull some command line arguments out)

	glutInit( &argc, argv );

	// setup all the graphics stuff:

	InitArray(cols, rows);

	InitGraphics( ); //set up nodes here

	// create the display lists that **will not change**:

	InitLists( );

	// init all the global variables used by Display( ):
	// this will also post a redisplay

	Reset( );

	// setup all the user interface stuff:

	InitMenus( );


	// draw the scene once and wait for some interaction:
	// (this will never return)

	glutSetWindow( MainWindow );
	glutMainLoop( );

	// glutMainLoop( ) never actually returns
	// the following line is here to make the compiler happy:

	return 0;
}


// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display( ) from here -- let glutPostRedisplay( ) do it

void
Animate( )
{
	// put animation stuff in here -- change some global variables for Display( ) to find:

	int ms = glutGet(GLUT_ELAPSED_TIME);
	ms %= MS_PER_CYCLE;							// makes the value of ms between 0 and MS_PER_CYCLE-1
	Time = (float)ms / (float)MS_PER_CYCLE;		// makes the value of Time between 0. and slightly less than 1.

	// for example, if you wanted to spin an object in Display( ), you might call: glRotatef( 360.f*Time,   0., 1., 0. );

	// force a call to Display( ) next time it is convenient:

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// draw the complete scene:

void
Display( )
{
	//if (DebugOn != 0)
		//fprintf(stderr, "Starting Display.\n");

	// set which window we want to do the graphics into:
	glutSetWindow( MainWindow );

	// erase the background:
	glDrawBuffer( GL_BACK );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glEnable( GL_DEPTH_TEST );
#ifdef DEMO_DEPTH_BUFFER
	if( DepthBufferOn == 0 )
		glDisable( GL_DEPTH_TEST );
#endif


	// specify shading to be flat:

	glShadeModel( GL_FLAT );

	// set the viewport to be a square centered in the window:

	GLsizei vx = glutGet( GLUT_WINDOW_WIDTH );
	GLsizei vy = glutGet( GLUT_WINDOW_HEIGHT );
	GLsizei v = vx < vy ? vx : vy;			// minimum dimension
	GLint xl = ( vx - v ) / 2;
	GLint yb = ( vy - v ) / 2;
	glViewport( xl, yb,  v, v );


	// set the viewing volume:
	// remember that the Z clipping  values are given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D( ) IF YOU ARE DOING 2D !

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );


	if (RenderMode == GL_SELECT)
	{
		int viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);
		if (DebugOn)
		{
			fprintf(stderr, "Xmouse, Ymouse' = %d, %d\n", Xmouse, vy - Ymouse);
			fprintf(stderr, "viewport = %d, %d, %d, %d\n",
				viewport[0], viewport[1], viewport[2], viewport[3]);
		}
		gluPickMatrix((double)Xmouse, (double)(vy - Ymouse), PICK_TOL, PICK_TOL, viewport);
	}


	if( NowProjection == ORTHO )
		glOrtho( -2.f, 2.f,     -2.f, 2.f,     0.1f, 1000.f );
	else
		gluPerspective( 70.f, 1.f,	0.1f, 1000.f );

	// place the objects into the scene:

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );

	// set the eye position, look-at position, and up-vector:

	gluLookAt( 0.f, 0.f, 3.f,     0.f, 0.f, 0.f,     0.f, 1.f, 0.f );

	// rotate the scene:

	glRotatef( (GLfloat)Yrot, 0.f, 1.f, 0.f );
	glRotatef( (GLfloat)Xrot, 1.f, 0.f, 0.f );

	// uniformly scale the scene:

	if( Scale < MINSCALE )
		Scale = MINSCALE;
	glScalef( (GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale );


	// turn # msec into the cycle ( 0 - MSEC-1 ):
	int msec = glutGet(GLUT_ELAPSED_TIME) % MS_PER_CYCLE;

	// turn that into a time in seconds:
	float nowTime = (float)msec / 1000.;

	// set the fog parameters:

	if( DepthCueOn != 0 )
	{
		glFogi( GL_FOG_MODE, FOGMODE );
		glFogfv( GL_FOG_COLOR, FOGCOLOR );
		glFogf( GL_FOG_DENSITY, FOGDENSITY );
		glFogf( GL_FOG_START, FOGSTART );
		glFogf( GL_FOG_END, FOGEND );
		glEnable( GL_FOG );
	}
	else
	{
		glDisable( GL_FOG );
	}

	// possibly draw the axes:

	if( AxesOn != 0 )
	{
		glColor3fv( &Colors[NowColor][0] );
		glCallList( AxesList );
	}

	// since we are using glScalef( ), be sure the normals get unitized:

	glEnable( GL_NORMALIZE );

	if (RenderMode == GL_SELECT)
	{
		glInitNames();
		glPushName(-1);
	}
	
	if (Freeze != 1) {

		for (int j = 0; j < rows; j++) {

			stateList[j][0].pos[0] = NodeZ.GetValue(nowTime) / 5.;

		}

		advOneTimeStep();
	}


	GLuint x = 0;

	glPointSize(5);
	if (RenderMode == GL_SELECT) {
		for (int i = 0; i < rows; i++) { //rows + 1 goes V
			for (int j = 0; j < cols; j++) { // cols + 1 goes ->
				glLoadName(x);
				fprintf(stderr, "%d %d: %s\n", i, j, stateList[i][j].isPicked ? "picked" : "not picked");
				glBegin(GL_POINTS);
				glVertex3f(stateList[i][j].pos[0], stateList[i][j].pos[1], stateList[i][j].pos[2]);
				glEnd();
				x++;
			}
		}
	
	}

	if (RenderMode == GL_RENDER) {
		glBegin(GL_POINTS);

		for (int i = 0; i < rows; i++) { //rows + 1 goes V
			for (int j = 0; j < cols; j++) { //cols + 1 goes ->


				if (!stateList[i][j].isPicked) {
					glColor3f(0., 0.5, 0.5);
				}
				else {
					glColor3f(0.5, 0.5, 0.5);
				}
				glVertex3f(stateList[i][j].pos[0], stateList[i][j].pos[1], stateList[i][j].pos[2]);


			}
		}
		glEnd();
	}

	

#ifdef DEMO_Z_FIGHTING
	if( DepthFightingOn != 0 )
	{
		glPushMatrix( );
			glRotatef( 90.f,   0.f, 1.f, 0.f );
			glCallList( ObjList );
		glPopMatrix( );
	}
#endif


	// draw some gratuitous text that just rotates on top of the scene:
	// i commented out the actual text-drawing calls -- put them back in if you have a use for them
	// a good use for thefirst one might be to have your name on the screen
	// a good use for the second one might be to have vertex numbers on the screen alongside each vertex

	
	// draw some gratuitous text that is fixed on the screen:
	//
	// the projection matrix is reset to define a scene whose
	// world coordinate system goes from 0-100 in each axis
	//
	// this is called "percent units", and is just a convenience
	//
	// the modelview matrix is reset to identity as we don't
	// want to transform these coordinates

	if (RenderMode == GL_RENDER)
	{
		glDisable(GL_DEPTH_TEST);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(0., 100., 0., 100.);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glColor3f(1., 1., 1.);
		char str[1024];
		sprintf(str, "NumHits = %d", Nhits);
		DoRasterString(5., 5., 0., str);
	}

	// swap the double-buffered framebuffers:

	if (RenderMode == GL_RENDER)
		glutSwapBuffers();

	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !

	glFlush( );
}


void
DoAxesMenu( int id )
{
	AxesOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoColorMenu( int id )
{
	NowColor = id - RED;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDebugMenu( int id )
{
	DebugOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthBufferMenu( int id )
{
	DepthBufferOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthFightingMenu( int id )
{
	DepthFightingOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthMenu( int id )
{
	DepthCueOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// main menu callback:

void
DoMainMenu( int id )
{
	switch( id )
	{
		case RESET:
			Reset( );
			break;

		case QUIT:
			// gracefully close out the graphics:
			// gracefully close the graphics window:
			// gracefully exit the program:
			glutSetWindow( MainWindow );
			glFinish( );
			glutDestroyWindow( MainWindow );
			exit( 0 );
			break;

		default:
			fprintf( stderr, "Don't know what to do with Main Menu ID %d\n", id );
	}

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoProjectMenu( int id )
{
	NowProjection = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// use glut to display a string of characters using a raster font:

void
DoRasterString( float x, float y, float z, char *s )
{
	glRasterPos3f( (GLfloat)x, (GLfloat)y, (GLfloat)z );

	char c;			// one character to print
	for( ; ( c = *s ) != '\0'; s++ )
	{
		glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, c );
	}
}


// use glut to display a string of characters using a stroke font:

void
DoStrokeString( float x, float y, float z, float ht, char *s )
{
	glPushMatrix( );
		glTranslatef( (GLfloat)x, (GLfloat)y, (GLfloat)z );
		float sf = ht / ( 119.05f + 33.33f );
		glScalef( (GLfloat)sf, (GLfloat)sf, (GLfloat)sf );
		char c;			// one character to print
		for( ; ( c = *s ) != '\0'; s++ )
		{
			glutStrokeCharacter( GLUT_STROKE_ROMAN, c );
		}
	glPopMatrix( );
}


// return the number of seconds since the start of the program:

float
ElapsedSeconds( )
{
	// get # of milliseconds since the start of the program:

	int ms = glutGet( GLUT_ELAPSED_TIME );

	// convert it to seconds:

	return (float)ms / 1000.f;
}


// initialize the glui window:

void
InitMenus( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitMenus.\n");

	glutSetWindow( MainWindow );

	int numColors = sizeof( Colors ) / ( 3*sizeof(float) );
	int colormenu = glutCreateMenu( DoColorMenu );
	for( int i = 0; i < numColors; i++ )
	{
		glutAddMenuEntry( ColorNames[i], i );
	}

	int axesmenu = glutCreateMenu( DoAxesMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthcuemenu = glutCreateMenu( DoDepthMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthbuffermenu = glutCreateMenu( DoDepthBufferMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthfightingmenu = glutCreateMenu( DoDepthFightingMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int debugmenu = glutCreateMenu( DoDebugMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int projmenu = glutCreateMenu( DoProjectMenu );
	glutAddMenuEntry( "Orthographic",  ORTHO );
	glutAddMenuEntry( "Perspective",   PERSP );

	int mainmenu = glutCreateMenu( DoMainMenu );
	glutAddSubMenu(   "Axes",          axesmenu);
	glutAddSubMenu(   "Axis Colors",   colormenu);

#ifdef DEMO_DEPTH_BUFFER
	glutAddSubMenu(   "Depth Buffer",  depthbuffermenu);
#endif

#ifdef DEMO_Z_FIGHTING
	glutAddSubMenu(   "Depth Fighting",depthfightingmenu);
#endif

	glutAddSubMenu(   "Depth Cue",     depthcuemenu);
	glutAddSubMenu(   "Projection",    projmenu );
	glutAddMenuEntry( "Reset",         RESET );
	glutAddSubMenu(   "Debug",         debugmenu);
	glutAddMenuEntry( "Quit",          QUIT );

// attach the pop-up menu to the right mouse button:

	glutAttachMenu( GLUT_RIGHT_BUTTON );
}



// initialize the glut and OpenGL libraries:
//	also setup callback functions

void
InitGraphics( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitGraphics.\n");

	// request the display modes:
	// ask for red-green-blue-alpha color, double-buffering, and z-buffering:

	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );

	// set the initial window configuration:

	glutInitWindowPosition( 0, 0 );
	glutInitWindowSize( INIT_WINDOW_SIZE, INIT_WINDOW_SIZE );

	// open the window and set its title:

	MainWindow = glutCreateWindow( WINDOWTITLE );
	glutSetWindowTitle( WINDOWTITLE );

	// setting up picking buffer:

	glSelectBuffer(PICK_BUFFER_SIZE, PickBuffer);

	// set the framebuffer clear values:

	glClearColor( BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3] );

	// setup the callback functions:
	// DisplayFunc -- redraw the window
	// ReshapeFunc -- handle the user resizing the window
	// KeyboardFunc -- handle a keyboard input
	// MouseFunc -- handle the mouse button going down or up
	// MotionFunc -- handle the mouse moving with a button down
	// PassiveMotionFunc -- handle the mouse moving with a button up
	// VisibilityFunc -- handle a change in window visibility
	// EntryFunc	-- handle the cursor entering or leaving the window
	// SpecialFunc -- handle special keys on the keyboard
	// SpaceballMotionFunc -- handle spaceball translation
	// SpaceballRotateFunc -- handle spaceball rotation
	// SpaceballButtonFunc -- handle spaceball button hits
	// ButtonBoxFunc -- handle button box hits
	// DialsFunc -- handle dial rotations
	// TabletMotionFunc -- handle digitizing tablet motion
	// TabletButtonFunc -- handle digitizing tablet button hits
	// MenuStateFunc -- declare when a pop-up menu is in use
	// TimerFunc -- trigger something to happen a certain time from now
	// IdleFunc -- what to do when nothing else is going on

	glutSetWindow( MainWindow );
	glutDisplayFunc( Display );
	glutReshapeFunc( Resize );
	glutKeyboardFunc( Keyboard );
	glutMouseFunc( MouseButton );
	glutMotionFunc( MouseMotion );
	glutPassiveMotionFunc(MouseMotion);
	//glutPassiveMotionFunc( NULL );
	glutVisibilityFunc( Visibility );
	glutEntryFunc( NULL );
	glutSpecialFunc( NULL );
	glutSpaceballMotionFunc( NULL );
	glutSpaceballRotateFunc( NULL );
	glutSpaceballButtonFunc( NULL );
	glutButtonBoxFunc( NULL );
	glutDialsFunc( NULL );
	glutTabletMotionFunc( NULL );
	glutTabletButtonFunc( NULL );
	glutMenuStateFunc( NULL );
	glutTimerFunc( -1, NULL, 0 );

	// setup glut to call Animate( ) every time it has
	// 	nothing it needs to respond to (which is most of the time)
	// we don't need to do this for this program, and really should set the argument to NULL
	// but, this sets us up nicely for doing animation

	glutIdleFunc( Animate );

	// init the glew package (a window must be open to do this):

#ifdef WIN32
	GLenum err = glewInit( );
	if( err != GLEW_OK )
	{
		fprintf( stderr, "glewInit Error\n" );
	}
	else
		fprintf( stderr, "GLEW initialized OK\n" );
	fprintf( stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif

	// all other setups go here, such as GLSLProgram and KeyTime setups:

	NodeZ.Init();
	NodeZ.AddTimeValue(0.0, 1.0);
	NodeZ.AddTimeValue(2.5, 0.0);
	NodeZ.AddTimeValue(5.0, 1.0);
	NodeZ.AddTimeValue(7.5, 0.0);
	NodeZ.AddTimeValue(10.0, 1.0);

	for (int i = 0; i < rows; i++) { 
		for (int j = 0; j < cols; j++) {

			stateList[i][j].pos[0] = j/(gapSize);
			stateList[i][j].pos[1] = -i;
			stateList[i][j].pos[2] = 0;

			stateList[i][j].vel[0] = 0;
			stateList[i][j].vel[1] = 0;
			stateList[i][j].vel[2] = 0;
		}
	}

}


// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList( )

void
InitLists( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitLists.\n");

	glutSetWindow( MainWindow );

	// create the object:

	int object = -1;

	//ObjList = glGenLists(1);
	//glPushMatrix();
	////SetMaterial(1.f, 1.f, 1.f, 30.f);
	//glNewList(ObjList, GL_COMPILE);
	//glPointSize(ptSize);
	//glBegin(GL_POINTS);
	//glVertex3f(0, 0, 0);
	//glEnd();
	//glPopMatrix();
	//glEndList();

	//PickedObjList = glGenLists(1);
	//glPushMatrix();
	////SetMaterial(1.f, 0.f, 0.f, 30.f);
	//glNewList(PickedObjList, GL_COMPILE);
	//glPointSize(ptSize);
	//glBegin(GL_POINTS);
	//glVertex3f(0, 0, 0);
	//glEnd();
	//glPopMatrix();
	//glEndList();


	// create the axes:

	/*AxesList = glGenLists( 1 );
	glNewList( AxesList, GL_COMPILE );
		glLineWidth( AXES_WIDTH );
			Axes( 1.5 );
		glLineWidth( 1. );
	glEndList( );*/
}


// the keyboard callback:

void
Keyboard( unsigned char c, int x, int y )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Keyboard: '%c' (0x%0x)\n", c, c );

	switch( c )
	{
		case 'o':
		case 'O':
			NowProjection = ORTHO;
			break;

		case 'p':
		case 'P':
			NowProjection = PERSP;
			break;
		case 'f':
		case 'F':
			if (Freeze == 0) {
				Freeze = 1;
			}
			else {
				Freeze = 0;
			}
			break;
			break;	
		case 'q':
		case 'Q':
		case ESCAPE:
			DoMainMenu( QUIT );	// will not return here
			break;				// happy compiler

		default:
			fprintf( stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c );
	}

	// force a call to Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// called when the mouse button transitions down or up:

void
MouseButton( int button, int state, int x, int y )
{
	int b = 0;			// LEFT, MIDDLE, or RIGHT

	if( DebugOn != 0 )
		fprintf( stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y );

	
	// get the proper button bit mask:

	switch( button )
	{
		case GLUT_LEFT_BUTTON:
			b = LEFT;		break;

		case GLUT_MIDDLE_BUTTON:
			b = MIDDLE;		break;

		case GLUT_RIGHT_BUTTON:
			b = RIGHT;		break;

		case SCROLL_WHEEL_UP:
			Scale += SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
			// keep object from turning inside-out or disappearing:
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;

		case SCROLL_WHEEL_DOWN:
			Scale -= SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
			// keep object from turning inside-out or disappearing:
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;

		default:
			b = 0;
			fprintf( stderr, "Unknown mouse button: %d\n", button );
	}

	// button down sets the bit, up clears the bit:

	if( state == GLUT_DOWN )
	{
		Xmouse = x;
		Ymouse = y;
		ActiveButton |= b;		// set the proper bit
	}
	else
	{
		ActiveButton &= ~b;		// clear the proper bit
	}

	SomethingPicked = false;

	if ((ActiveButton & LEFT) != 0) {

		RenderMode = GL_SELECT;
		glRenderMode(GL_SELECT);
		Display();
		RenderMode = GL_RENDER;
		Nhits = glRenderMode(GL_RENDER);

		if (true) {
			RenderMode = GL_SELECT;
			glRenderMode(GL_SELECT);
			Display();
			RenderMode = GL_RENDER;
			Nhits = glRenderMode(GL_RENDER);
		}

		fprintf(stderr, "#pick hits = %d\n", Nhits);

		int closestItem = -1;
		int closestZ = 0xffffffff;

		int index = 0;
		for (int i = 0; i < Nhits; i++)
		{
			int numItems = PickBuffer[index++];
			int zmin = PickBuffer[index++];
			int zmax = PickBuffer[index++];
			if (DebugOn)
			{
				fprintf(stderr, "Hit # %2d found %2d items on the name stack\n", i, numItems);
				fprintf(stderr, "\tZmin = %8u, Zmax = %8u\n", zmin, zmax);
			}

			for (int j = 0; j < numItems; j++) {

				int item = PickBuffer[index++];
				fprintf(stderr, "\nthing: %u\nrow: %u\ncol: %u\n", item, (item / rows), item % cols);
				stateList[(item / cols)][item % cols].isPicked = !(stateList[(item / cols)][item % cols].isPicked);

			}
					
		}

		


	}

	glutSetWindow(MainWindow);
	glutPostRedisplay();

}


// called when the mouse moves while a button is down:

void
MouseMotion( int x, int y )
{
	int dx = x - Xmouse;		// change in mouse coords
	int dy = y - Ymouse;

	if( ( ActiveButton & LEFT ) != 0 )
	{
		Xrot += ( ANGFACT*dy );
		Yrot += ( ANGFACT*dx );
	}

	if( ( ActiveButton & MIDDLE ) != 0 )
	{
		Scale += SCLFACT * (float) ( dx - dy );

		// keep object from turning inside-out or disappearing:

		if( Scale < MINSCALE )
			Scale = MINSCALE;
	}

	Xmouse = x;			// new current position
	Ymouse = y;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// reset the transformations and the colors:
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene

void
Reset( )
{
	ActiveButton = 0;
	AxesOn = 1;
	DebugOn = 0;
	DepthBufferOn = 1;
	DepthFightingOn = 0;
	DepthCueOn = 0;
	Scale  = 1.0;
	ShadowsOn = 0;
	NowColor = YELLOW;
	NowProjection = PERSP;
	Xrot = Yrot = 0.;
	Freeze = 0;
	Nhits = 0;


	for (int i = 0; i < rows; i++) { //plus one cols >
		for (int j = 0; j < cols; j++) { // plus one row V
			for (int k = 0; k < 3; k++) { //x, y, z
				// three dimensions x y and z

				stateList[i][j].vel[k] = 0;

				stateList[i][j].acc[k] = 0;

				derivList[i][j].vel[k] = 0;
				derivList[i][j].acc[k] = 0;

				stateList[i][j].isPicked = false;
			}
			stateList[i][j].pos[0] = j;
			stateList[i][j].pos[1] = -i;
			stateList[i][j].pos[2] = 0;
			stateList[i][j].isPicked = false;
		}
	}

}


// called when user resizes the window:

void
Resize( int width, int height )
{
	// don't really need to do anything since window size is
	// checked each time in Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// handle a change to the window's visibility:

void
Visibility ( int state )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Visibility: %d\n", state );

	if( state == GLUT_VISIBLE )
	{
		glutSetWindow( MainWindow );
		glutPostRedisplay( );
	}
	else
	{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}



///////////////////////////////////////   HANDY UTILITIES:  //////////////////////////


// the stroke characters 'X' 'Y' 'Z' :

static float xx[ ] = { 0.f, 1.f, 0.f, 1.f };

static float xy[ ] = { -.5f, .5f, .5f, -.5f };

static int xorder[ ] = { 1, 2, -3, 4 };

static float yx[ ] = { 0.f, 0.f, -.5f, .5f };

static float yy[ ] = { 0.f, .6f, 1.f, 1.f };

static int yorder[ ] = { 1, 2, 3, -2, 4 };

static float zx[ ] = { 1.f, 0.f, 1.f, 0.f, .25f, .75f };

static float zy[ ] = { .5f, .5f, -.5f, -.5f, 0.f, 0.f };

static int zorder[ ] = { 1, 2, 3, 4, -5, 6 };

// fraction of the length to use as height of the characters:
const float LENFRAC = 0.10f;

// fraction of length to use as start location of the characters:
const float BASEFRAC = 1.10f;

//	Draw a set of 3D axes:
//	(length is the axis length in world coordinates)

void
Axes( float length )
{
	glBegin( GL_LINE_STRIP );
		glVertex3f( length, 0., 0. );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., length, 0. );
	glEnd( );
	glBegin( GL_LINE_STRIP );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., 0., length );
	glEnd( );

	float fact = LENFRAC * length;
	float base = BASEFRAC * length;

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 4; i++ )
		{
			int j = xorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( base + fact*xx[j], fact*xy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 5; i++ )
		{
			int j = yorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( fact*yx[j], base + fact*yy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 6; i++ )
		{
			int j = zorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( 0.0, fact*zy[j], base + fact*zx[j] );
		}
	glEnd( );

}


