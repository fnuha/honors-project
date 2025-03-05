#include <GL/glut.h>
#include <GL/glui.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

//keytime functionality
#include "keytime.cpp"

//picking value defs
#define PICK_TOL 10.
#define PICK_BUFFER_SIZE 256

//window name defs
const char* WINDOWTITLE = "OpenGL / GLUT Sample -- Faaizah Nuha";
const char* GLUITITLE = "User Interface Window";

//window size def
const int INIT_WINDOW_SIZE = 600;

//input interaction multiplication factors
const float ANGFACT = 1.f;
const float SCLFACT = 0.005f;

// minimum allowable scale factor:
const float MINSCALE = 0.05f;

// scroll wheel button values:
const int SCROLL_WHEEL_UP = 3;
const int SCROLL_WHEEL_DOWN = 4;

// equivalent mouse movement when we click the scroll wheel:
const float SCROLL_WHEEL_CLICK_FACTOR = 5.f;

// active mouse buttons (or them together):
const int LEFT = 4;
const int MIDDLE = 2;
const int RIGHT = 1;

// window background color (rgba):
const GLfloat BACKCOLOR[] = { 0., 0., 0., 1. };

// for animation:
const int MS_PER_CYCLE = 10000;		// 10000 milliseconds = 10 

int		ActiveButton;			// current button that is down
int		DebugOn;				// != 0 means to print debugging info
int		MainWindow;				// window id for main graphics window
float	Scale;					// scaling factor
float	Time;					// used for animation, this has a value between 0. and 1.
int		Xmouse, Ymouse;			// mouse values
float	Xrot, Yrot;				// rotation angles in degrees
static float spin;
GLUI* glui;

Keytimes NodeZ;

//changeable variables
static GLfloat Weight = 0.5;
static GLfloat	LENGTH0 = 1.2;
static GLfloat	kval = 1.0;
static GLint Moveable = 0;			// 0 = selecting, 1 = pinning, 2 = moving single point, 3 = moving camera around
int		moveable_x = -1;
int		moveable_y = -1;
int		rows = 20;
int		cols = 20;
float	damp = 1.0;
int		ptSize = 4;
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
	bool isPinned;
	float nodeWeight;
	float nodeLength;
	float nodeK;
};

void	Animate();
void	Display();
void	InitGraphics();
void	Keyboard(unsigned char, int, int);
void	MouseButton(int, int, int, int);
void	MouseMotion(int, int);
void	Reset();
void	Resize(int, int);
void	Visibility(int);


nodeState** stateList; //[rows >][cols V]
derivs** derivList;
int		Freeze = 0; //0 means no freeze, 1 means yes freeze

unsigned int PickBuffer[PICK_BUFFER_SIZE];
int RenderMode; //GL_RENDER vs GL_SELECT mode

void getOneDDerivs() {

	//change the direct neighbor node calculations + add left, right, bottom to change to reflect
	//delta x/y/z 

	for (int i = 0; i < rows; i++) { // plus one row v
		for (int j = 0; j < cols; j++) { // plus one col >

			if ((i == moveable_x && j == moveable_y && Moveable == 2 && ((ActiveButton & LEFT) != 0)) || stateList[i][j].isPinned) {
				derivList[i][j].vel[0] = 0;
				derivList[i][j].acc[0] = 0; //updating acc
				derivList[i][j].vel[1] = 0;
				derivList[i][j].acc[1] = 0; //updating acceleration
				derivList[i][j].vel[2] = 0;
				derivList[i][j].acc[2] = 0;
				
				continue;
			}

			float sumfx = 0; //no downwards weight
			float sumfy = -stateList[i][j].nodeWeight; // downwards weight
			float sumfz = 0;


			if (i > 0) { //above neighbor

				float dx = stateList[i - 1][j].pos[0] - stateList[i][j].pos[0];
				float dy = stateList[i - 1][j].pos[1] - stateList[i][j].pos[1];
				float dz = stateList[i - 1][j].pos[2] - stateList[i][j].pos[2];

				float length = sqrt(dx * dx + dy * dy + dz * dz);
				dx /= length;
				dy /= length;
				dz /= length;
				float stretch2 = length - stateList[i][j].nodeLength;
				float force = stateList[i][j].nodeK * stretch2; //0.5 should not be hard coded, change spring constant to a slider

				sumfy -= damp * derivList[i][j].vel[1]; //adding velocity to stretch
				sumfx -= damp * derivList[i][j].vel[0]; //adding velocity to stretch
				sumfz -= damp * derivList[i][j].vel[2];

				sumfx += force * dx;
				sumfy += force * dy;
				sumfz += force * dz;

			}

			if (i < rows - 1) { //below neighbor

				float dx = stateList[i + 1][j].pos[0] - stateList[i][j].pos[0];
				float dy = stateList[i + 1][j].pos[1] - stateList[i][j].pos[1];
				float dz = stateList[i + 1][j].pos[2] - stateList[i][j].pos[2];

				float length = sqrt(dx * dx + dy * dy + dz * dz);
				dx /= length;
				dy /= length;
				dz /= length;
				float stretch2 = length - stateList[i][j].nodeLength;
				float force = stateList[i][j].nodeK * stretch2; //0.5 should not be hard coded, change spring constant to a slider

				sumfx += force * dx;
				sumfy += force * dy;
				sumfz += force * dz;

			}

			if (j > 0) { //left neighbor

				float dx = stateList[i][j - 1].pos[0] - stateList[i][j].pos[0];
				float dy = stateList[i][j - 1].pos[1] - stateList[i][j].pos[1];
				float dz = stateList[i][j - 1].pos[2] - stateList[i][j].pos[2];

				float length = sqrt(dx * dx + dy * dy + dz * dz);
				dx /= length;
				dy /= length;
				dz /= length;
				float stretch2 = length - stateList[i][j].nodeLength;
				float force = stateList[i][j].nodeK * stretch2; //0.5 should not be hard coded, change spring constant to a slider

				sumfx += force * dx;
				sumfy += force * dy;
				sumfz += force * dz;

			}

			if (j < cols - 1) { //right neighbor

				float dx = stateList[i][j + 1].pos[0] - stateList[i][j].pos[0];
				float dy = stateList[i][j + 1].pos[1] - stateList[i][j].pos[1];
				float dz = stateList[i][j + 1].pos[2] - stateList[i][j].pos[2];

				float length = sqrt(dx * dx + dy * dy + dz * dz);
				dx /= length;
				dy /= length;
				dz /= length;
				float stretch2 = length - stateList[i][j].nodeLength;
				float force = stateList[i][j].nodeK * stretch2; //0.5 should not be hard coded, change spring constant to a slider

				sumfx += force * dx;
				sumfy += force * dy;
				sumfz += force * dz;

			}

			
			if (i > 0 && j > 0) { //if past first row and past first col
				float dx = stateList[i - 1][j - 1].pos[0] - stateList[i][j].pos[0];
				float dy = stateList[i - 1][j - 1].pos[1] - stateList[i][j].pos[1];
				float dz = stateList[i - 1][j - 1].pos[2] - stateList[i][j].pos[2];

				float length = sqrt(dx * dx + dy * dy + dz * dz);
				dx /= length;
				dy /= length;
				dz /= length;
				float stretch2 = length - stateList[i][j].nodeLength;
				float force = stateList[i][j].nodeK * stretch2; //0.5 should not be hard coded, change spring constant to a slider


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
				float stretch2 = length - stateList[i][j].nodeLength;
				float force = stateList[i][j].nodeK * stretch2;


				sumfx += force * dx;
				sumfy += force * dy;
				sumfz += force * dz;

			}

			if (i < rows - 1 && j < cols - 1) { // if not at bottom row and not at last col
				float dx = stateList[i + 1][j + 1].pos[0] - stateList[i][j].pos[0];
				float dy = stateList[i + 1][j + 1].pos[1] - stateList[i][j].pos[1];
				float dz = stateList[i + 1][j + 1].pos[2] - stateList[i][j].pos[2];

				float length = sqrt(dx * dx + dy * dy + dz * dz);
				dx /= length;
				dy /= length;
				dz /= length;
				float stretch2 = length - stateList[i][j].nodeLength;
				float force = stateList[i][j].nodeK * stretch2;


				sumfx += force * dx;
				sumfy += force * dy;
				sumfz += force * dz;

			}

			if (i > 0 && j < cols - 1) { //if past first row and not at last col
				float dx = stateList[i - 1][j + 1].pos[0] - stateList[i][j].pos[0];
				float dy = stateList[i - 1][j + 1].pos[1] - stateList[i][j].pos[1];
				float dz = stateList[i - 1][j + 1].pos[2] - stateList[i][j].pos[2];

				float length = sqrt(dx * dx + dy * dy + dz * dz);
				dx /= length;
				dy /= length;
				dz /= length;
				float stretch2 = length - stateList[i][j].nodeLength;
				float force = stateList[i][j].nodeK * stretch2; //0.5 should not be hard coded, change spring constant to a slider

				sumfx += force * dx;
				sumfy += force * dy;
				sumfz += force * dz;

			}


			derivList[i][j].vel[0] = stateList[i][j].vel[0]; //updating vel
			derivList[i][j].acc[0] = sumfx / stateList[i][j].nodeWeight; //updating acc
			derivList[i][j].vel[1] = stateList[i][j].vel[1]; //updating velocity
			derivList[i][j].acc[1] = sumfy / stateList[i][j].nodeWeight; //updating acceleration
			derivList[i][j].vel[2] = stateList[i][j].vel[2];
			derivList[i][j].acc[2] = sumfz / stateList[i][j].nodeWeight;
		}
	}


}


void advOneTimeStep() {
	getOneDDerivs();
	for (int i = 0; i < rows; i++) { // plus one row V
		for (int j = 0; j < cols; j++) { // plus one col >
			for (int k = 0; k < 3; k++) { // three dimensions x y and z
				stateList[i][j].pos[k] = stateList[i][j].pos[k] + (derivList[i][j].vel[k] * 0.05);
				stateList[i][j].vel[k] = stateList[i][j].vel[k] + (derivList[i][j].acc[k] * 0.05);
			}
		}
	}

}

void InitArray(int cols, int rows) {

	stateList = new nodeState * [rows];
	derivList = new derivs * [rows];

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
			stateList[i][j].pos[0] = j*LENGTH0;
			stateList[i][j].pos[1] = -i*LENGTH0;
			stateList[i][j].pos[2] = 0;
			stateList[i][j].isPicked = false;
			if (i == 0) {
				stateList[i][j].isPinned = true;
			}
			else {
				stateList[i][j].isPinned = false;
			}
			stateList[i][j].nodeWeight = Weight;
			stateList[i][j].nodeLength = LENGTH0;
			stateList[i][j].nodeK = kval;
		}
	}
}

void Animate(void)
{
	if (glutGetWindow() != MainWindow)
		glutSetWindow(MainWindow);

	
	for (int i = 0; i < rows; i++) { //plus one cols >
		for (int j = 0; j < cols; j++) { // plus one row V
			if (stateList[i][j].isPicked) {
				stateList[i][j].nodeWeight = Weight;
				stateList[i][j].nodeLength = LENGTH0;
				stateList[i][j].nodeK = kval;
			}
		}
	}


	int ms = glutGet(GLUT_ELAPSED_TIME);
	ms %= MS_PER_CYCLE;							// makes the value of ms between 0 and MS_PER_CYCLE-1
	Time = (float)ms / (float)MS_PER_CYCLE;		// makes the value of Time between 0. and slightly less than 1.

	//fprintf(stderr, "%d\n", Moveable);
	
	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void Display(void)
{
	// set which window we want to do the graphics into:
	glutSetWindow(MainWindow);

	// erase the background:
	glDrawBuffer(GL_BACK);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
#ifdef DEMO_DEPTH_BUFFER
	if (DepthBufferOn == 0)
		glDisable(GL_DEPTH_TEST);
#endif


	// specify shading to be flat:

	glShadeModel(GL_FLAT);

	// set the viewport to be a square centered in the window:

	GLsizei vx = glutGet(GLUT_WINDOW_WIDTH);
	GLsizei vy = glutGet(GLUT_WINDOW_HEIGHT);
	GLsizei v = vx < vy ? vx : vy;			// minimum dimension
	GLint xl = (vx - v) / 2;
	GLint yb = (vy - v) / 2;
	glViewport(xl, yb, v, v);


	// set the viewing volume:
	// remember that the Z clipping  values are given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D( ) IF YOU ARE DOING 2D !

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();


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



	gluPerspective(70.f, 1.f, 0.1f, 1000.f);

	// place the objects into the scene:

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// set the eye position, look-at position, and up-vector:

	gluLookAt(0.f, 0.f, 3.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f);

	// rotate the scene:

	glRotatef((GLfloat)Yrot, 0.f, 1.f, 0.f);
	glRotatef((GLfloat)Xrot, 1.f, 0.f, 0.f);

	// uniformly scale the scene:

	if (Scale < MINSCALE)
		Scale = MINSCALE;
	glScalef((GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale);


	// turn # msec into the cycle ( 0 - MSEC-1 ):
	int msec = glutGet(GLUT_ELAPSED_TIME) % MS_PER_CYCLE;

	// turn that into a time in seconds:
	float nowTime = (float)msec / 1000.;

	// since we are using glScalef( ), be sure the normals get unitized:

	glEnable(GL_NORMALIZE);

	if (RenderMode == GL_SELECT)
	{
		glInitNames();
		glPushName(-1);
	}

	if (Freeze != 1) {

	/*	for (int j = 0; j < cols; j++) {

			stateList[0][j].pos[1] = NodeZ.GetValue(nowTime) / 5.;

		}*/

		advOneTimeStep();
	}


	GLuint x = 0;

	glPointSize(5);
	if (RenderMode == GL_SELECT) {
		for (int i = 0; i < rows; i++) { //rows + 1 goes V
			for (int j = 0; j < cols; j++) { // cols + 1 goes ->
				glLoadName(x);
				//fprintf(stderr, "%d %d: %s\n", i, j, stateList[i][j].isPicked ? "picked" : "not picked");
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
		//sprintf(str, "NumHits = %d", Nhits);
	}

	// swap the double-buffered framebuffers:

	if (RenderMode == GL_RENDER)
		glutSwapBuffers();

	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !

	glFlush();

}

void Resize(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-50.0, 50.0, -50.0, 50.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void InitGraphics() {

	// ask for red-green-blue-alpha color, double-buffering, and z-buffering:
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

	// set the initial window configuration:
	glutInitWindowSize(INIT_WINDOW_SIZE, INIT_WINDOW_SIZE);
	glutInitWindowPosition(100, 100);

	// open the window and set its title:
	MainWindow = glutCreateWindow(WINDOWTITLE);
	glutSetWindowTitle(WINDOWTITLE);

	//setting up picking buffer
	glSelectBuffer(PICK_BUFFER_SIZE, PickBuffer);

	glClearColor(BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3]);

	glutSetWindow(MainWindow);
	glutDisplayFunc(Display);
	glutReshapeFunc(Resize);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(MouseButton);
	glutMotionFunc(MouseMotion);
	glutPassiveMotionFunc(MouseMotion);
	glutVisibilityFunc(Visibility);
	glutEntryFunc(NULL);
	glutSpecialFunc(NULL);
	glutSpaceballMotionFunc(NULL);
	glutSpaceballRotateFunc(NULL);
	glutSpaceballButtonFunc(NULL);
	glutButtonBoxFunc(NULL);
	glutDialsFunc(NULL);
	glutTabletMotionFunc(NULL);
	glutTabletButtonFunc(NULL);
	glutMenuStateFunc(NULL);
	glutTimerFunc(-1, NULL, 0);

	glutIdleFunc(Animate);

	NodeZ.Init();
	NodeZ.AddTimeValue(0.0, 0.0);
	NodeZ.AddTimeValue(1.25, 20.0);
	NodeZ.AddTimeValue(2.5, 0.0);
	NodeZ.AddTimeValue(3.75, 20.0);
	NodeZ.AddTimeValue(5.0, 0.0);
	NodeZ.AddTimeValue(6.25, 20.0);
	NodeZ.AddTimeValue(7.5, 0.0);
	NodeZ.AddTimeValue(8.75, 20.0);
	NodeZ.AddTimeValue(10.0, 0.0);


}

void Deselect() {

	for (int i = 0; i < rows; i++) { //plus one cols >
		for (int j = 0; j < cols; j++) { // plus one row V
			stateList[i][j].isPicked = false;
		}
	}

}

void Select() {

	for (int i = 0; i < rows; i++) { //plus one cols >
		for (int j = 0; j < cols; j++) { // plus one row V
			stateList[i][j].isPicked = true;
		}
	}

}

void InitGLUI()
{
	glui = GLUI_Master.create_glui("GLUI");
	(new GLUI_Button(glui, "Select All", -1, (GLUI_Update_CB)Select));
	(new GLUI_Button(glui, "Deselect All", -1, (GLUI_Update_CB)Deselect));

	GLUI_Panel* type_panel = new GLUI_Panel(glui, "Mode");
	GLUI_RadioGroup* radio = new GLUI_RadioGroup(type_panel, &Moveable, 4);
	new GLUI_RadioButton(radio, "Selecting");
	new GLUI_RadioButton(radio, "Pinning");
	new GLUI_RadioButton(radio, "Move Single Point");
	new GLUI_RadioButton(radio, "Move Camera");

	//new GLUI_Checkbox(glui, "Move with cursor", &Moveable);

	GLUI_Scrollbar* sb;
	new GLUI_StaticText(glui, "Length");
	sb = new GLUI_Scrollbar(glui, "Length", GLUI_SCROLL_HORIZONTAL, &LENGTH0);
	sb->set_float_limits(0.0f, 10.0f);
	new GLUI_StaticText(glui, "K value - stretch");
	sb = new GLUI_Scrollbar(glui, "K Value", GLUI_SCROLL_HORIZONTAL, &kval);
	sb->set_float_limits(0.0f, 5.0f);
	new GLUI_StaticText(glui, "Weight");
	sb = new GLUI_Scrollbar(glui, "Weight", GLUI_SCROLL_HORIZONTAL, &Weight);
	sb->set_float_limits(0.0f, 5.0f);

	glui->set_main_gfx_window(MainWindow);

	GLUI_Master.set_glutIdleFunc(Animate);
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);

	InitArray(cols, rows);

	InitGraphics();

	Reset();

	InitGLUI();

	glutSetWindow(MainWindow);
	glutMainLoop();

	return 0;



}

void
Reset()
{
	ActiveButton = 0;
	DebugOn = 0;
	Scale = 1.0;
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

void
Visibility(int state)
{

	if (state == GLUT_VISIBLE)
	{
		glutSetWindow(MainWindow);
		glutPostRedisplay();
	}
	else
	{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}

// the keyboard callback:

void
Keyboard(unsigned char c, int x, int y)
{
	if (DebugOn != 0)
		fprintf(stderr, "Keyboard: '%c' (0x%0x)\n", c, c);

	switch (c)
	{
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
	default:
		fprintf(stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c);
	}

	// force a call to Display( ):

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void
MouseButton(int button, int state, int x, int y)
{
	int b = 0;			// LEFT, MIDDLE, or RIGHT

	if (DebugOn != 0)
		fprintf(stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y);


	// get the proper button bit mask:

	switch (button)
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
		fprintf(stderr, "Unknown mouse button: %d\n", button);
	}

	// button down sets the bit, up clears the bit:

	if (state == GLUT_DOWN)
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
				//fprintf(stderr, "\nthing: %u\nrow: %u\ncol: %u\n", item, (item / rows), item % cols);
				if (Moveable == 0) {
					stateList[(item / cols)][item % cols].isPicked = !(stateList[(item / cols)][item % cols].isPicked);
				}
				else if (Moveable == 1) {
					stateList[(item / cols)][item % cols].isPinned = !(stateList[(item / cols)][item % cols].isPinned);
				}
				moveable_x = item / cols;
				moveable_y = item % cols;

			}

		}




	}

	glutSetWindow(MainWindow);
	glutPostRedisplay();

}


// called when the mouse moves while a button is down:

void
MouseMotion(int x, int y)
{
	int dx = x - Xmouse;		// change in mouse coords
	int dy = y - Ymouse;

	if ((ActiveButton & LEFT) != 0)
	{
		if (Moveable == 1 || (Moveable == 2 && moveable_x > -1 && moveable_y > -1)) {
			stateList[moveable_x][moveable_y].pos[0] = stateList[moveable_x][moveable_y].pos[0] + dx;
			stateList[moveable_x][moveable_y].pos[1] = stateList[moveable_x][moveable_y].pos[1] - dy;
		}
		else if (Moveable == 3) {
			Xrot += (ANGFACT * dy);
			Yrot += (ANGFACT * dx);
		}
	}

	if ((ActiveButton & MIDDLE) != 0)
	{
		Scale += SCLFACT * (float)(dx - dy);

		// keep object from turning inside-out or disappearing:

		if (Scale < MINSCALE)
			Scale = MINSCALE;
	}

	Xmouse = x;			// new current position
	Ymouse = y;
	

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}