#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <math.h>

#pragma comment (lib, "OpenGL32.lib")
#pragma comment (lib, "GLU32.lib")

#define WINDOW_TITLE "OpenGL Window"

// These variables will store the rotation angles.
// We make them global so both WindowProcedure and display can use them.
float rotateX = 15.0f;
float rotateY = 0.0f;

// These variables will help us track the mouse movement.
bool isDragging = false; // Is the left mouse button being held down?
int lastMouseX = 0;      // Last recorded X position of the mouse.
int lastMouseY = 0;      // Last recorded Y position of the mouse.

LRESULT WINAPI WindowProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) PostQuitMessage(0);
		break;

		// --- (START) MOUSE HANDLING CODE ---

	case WM_LBUTTONDOWN: // When the left mouse button is pressed
	{
		isDragging = true; // Start dragging.
		lastMouseX = LOWORD(lParam); // Record the starting X position.
		lastMouseY = HIWORD(lParam); // Record the starting Y position.
		break;
	}

	case WM_LBUTTONUP: // When the left mouse button is released
	{
		isDragging = false; // Stop dragging.
		break;
	}

	case WM_MOUSEMOVE: // When the mouse is moved
	{
		if (isDragging) // Only do this if we are currently dragging
		{
			int currentMouseX = LOWORD(lParam); // Get the new X position.
			int currentMouseY = HIWORD(lParam); // Get the new Y position.

			// Calculate the difference in position since the last frame.
			float deltaX = (float)(currentMouseX - lastMouseX);
			float deltaY = (float)(currentMouseY - lastMouseY);

			// Update the rotation angles based on the mouse movement.
			// You can change 0.2f to make it more or less sensitive.
			rotateY += deltaX * 0.2f;
			rotateX += deltaY * 0.2f;

			// Update the last mouse position for the next frame.
			lastMouseX = currentMouseX;
			lastMouseY = currentMouseY;
		}
		break;
	}

	// --- (END) MOUSE HANDLING CODE ---

	default:
		break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}
//--------------------------------------------------------------------

bool initPixelFormat(HDC hdc)
{
	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

	pfd.cAlphaBits = 8;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 0;

	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;

	pfd.iLayerType = PFD_MAIN_PLANE;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;

	// choose pixel format returns the number most similar pixel format available
	int n = ChoosePixelFormat(hdc, &pfd);

	// set pixel format returns whether it sucessfully set the pixel format
	if (SetPixelFormat(hdc, n, &pfd))
	{
		return true;
	}
	else
	{
		return false;
	}
}
//--------------------------------------------------------------------

// --- Helper Functions to Draw Body Parts ---

// Draws a 3D cuboid WITH NORMALS for lighting
void drawCuboid(float width, float height, float depth)
{
	float w = width / 2.0f;
	float h = height / 2.0f;
	float d = depth / 2.0f;

	glBegin(GL_QUADS);
	// Front Face
	glNormal3f(0.0f, 0.0f, 1.0f); // Normal points towards viewer
	glVertex3f(-w, -h, d);
	glVertex3f(w, -h, d);
	glVertex3f(w, h, d);
	glVertex3f(-w, h, d);
	// Back Face
	glNormal3f(0.0f, 0.0f, -1.0f); // Normal points away from viewer
	glVertex3f(-w, -h, -d);
	glVertex3f(-w, h, -d);
	glVertex3f(w, h, -d);
	glVertex3f(w, -h, -d);
	// Top Face
	glNormal3f(0.0f, 1.0f, 0.0f); // Normal points up
	glVertex3f(-w, h, -d);
	glVertex3f(-w, h, d);
	glVertex3f(w, h, d);
	glVertex3f(w, h, -d);
	// Bottom Face
	glNormal3f(0.0f, -1.0f, 0.0f); // Normal points down
	glVertex3f(-w, -h, -d);
	glVertex3f(w, -h, -d);
	glVertex3f(w, -h, d);
	glVertex3f(-w, -h, d);
	// Right face
	glNormal3f(1.0f, 0.0f, 0.0f); // Normal points right
	glVertex3f(w, -h, -d);
	glVertex3f(w, h, -d);
	glVertex3f(w, h, d);
	glVertex3f(w, -h, d);
	// Left Face
	glNormal3f(-1.0f, 0.0f, 0.0f); // Normal points left
	glVertex3f(-w, -h, -d);
	glVertex3f(-w, -h, d);
	glVertex3f(-w, h, d);
	glVertex3f(-w, h, -d);
	glEnd();
}

// Draws the belt and buckle with better detail
void drawBelt3D() {
	// Belt Strap (thinner)
	glColor3f(0.8f, 0.6f, 0.2f); // Darker gold for the belt
	glPushMatrix();
	glTranslatef(0.0f, -0.05f, 0.0f); // Adjust position slightly down
	glScalef(2.5f, 0.25f, 2.0f); // Thinner belt
	drawCuboid(0.2f, 0.2f, 0.2f);
	glPopMatrix();

	// Buckle (more detailed triangle)
	glColor3f(0.9f, 0.9f, 0.9f); // Lighter color for the buckle gem
	float b_d = 0.05f; // Buckle depth
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, 0.2f); // Position in front
	glBegin(GL_TRIANGLES);
	// Front face
	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, -0.12f, b_d);
	glVertex3f(0.12f, 0.12f, b_d);
	glVertex3f(-0.12f, 0.12f, b_d);
	// Back face
	glNormal3f(0.0f, 0.0f, -1.0f);
	glVertex3f(0.0f, -0.12f, -b_d);
	glVertex3f(-0.12f, 0.12f, -b_d);
	glVertex3f(0.12f, 0.12f, -b_d);
	// Side 1 (bottom-left)
	glNormal3f(-0.8f, -0.6f, 0.0f);
	glVertex3f(0.0f, -0.12f, b_d); glVertex3f(-0.12f, 0.12f, b_d);
	glVertex3f(-0.12f, 0.12f, -b_d); glVertex3f(0.0f, -0.12f, -b_d);
	// Side 2 (bottom-right)
	glNormal3f(0.8f, -0.6f, 0.0f);
	glVertex3f(0.0f, -0.12f, b_d); glVertex3f(0.0f, -0.12f, -b_d);
	glVertex3f(0.12f, 0.12f, -b_d); glVertex3f(0.12f, 0.12f, b_d);
	// Side 3 (top)
	glNormal3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-0.12f, 0.12f, b_d); glVertex3f(0.12f, 0.12f, b_d);
	glVertex3f(0.12f, 0.12f, -b_d); glVertex3f(-0.12f, 0.12f, -b_d);
	glEnd();
	glPopMatrix();
}

void drawShoulderPads3D() {
	glColor3f(0.9f, 0.7f, 0.1f); // Slightly different shade of gold
	float padDepth = 0.25f;
	float d = padDepth / 2.0f;

	// --- Left Shoulder Pad ---
	glPushMatrix();
	// Adjusted Y and X position to integrate better with the new torso and arm positions.
	glTranslatef(-0.45f, 0.73f, 0.0f); // WAS: -0.4f, 0.75f, 0.0f
	glRotatef(10.0f, 0.0f, 0.0f, 1.0f); // Slight outward rotation

	glBegin(GL_QUADS);
	float v[8][3] = {
		{-0.3f, 0.2f, d}, {0.3f, 0.2f, d}, {0.4f, -0.1f, d}, {-0.4f, -0.1f, d}, // Front
		{-0.3f, 0.2f, -d}, {0.3f, 0.2f, -d}, {0.4f, -0.1f, -d}, {-0.4f, -0.1f, -d} // Back
	};

	// Front Face
	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3fv(v[0]); glVertex3fv(v[1]); glVertex3fv(v[2]); glVertex3fv(v[3]);
	// Back Face
	glNormal3f(0.0f, 0.0f, -1.0f);
	glVertex3fv(v[4]); glVertex3fv(v[7]); glVertex3fv(v[6]); glVertex3fv(v[5]);
	// Top Face (horizontal)
	glNormal3f(0.0f, 1.0f, 0.0f);
	glVertex3fv(v[0]); glVertex3fv(v[4]); glVertex3fv(v[5]); glVertex3fv(v[1]);
	// Bottom Face
	glNormal3f(0.0f, -1.0f, 0.0f);
	glVertex3fv(v[3]); glVertex3fv(v[2]); glVertex3fv(v[6]); glVertex3fv(v[7]);
	// Right Side Face (slanted normal)
	glNormal3f(0.7f, 0.3f, 0.0f);
	glVertex3fv(v[1]); glVertex3fv(v[5]); glVertex3fv(v[6]); glVertex3fv(v[2]);
	// Left Side Face (slanted normal)
	glNormal3f(-0.7f, 0.3f, 0.0f);
	glVertex3fv(v[0]); glVertex3fv(v[3]); glVertex3fv(v[7]); glVertex3fv(v[4]);
	glEnd();
	glPopMatrix();

	// --- Right Shoulder Pad ---
	glPushMatrix();
	glTranslatef(0.45f, 0.73f, 0.0f); // Adjusted Y and X position
	glRotatef(-10.0f, 0.0f, 0.0f, 1.0f); // Slight outward rotation

	glBegin(GL_QUADS);
	float v2[8][3] = {
		{-0.3f, 0.2f, d}, {0.3f, 0.2f, d}, {0.4f, -0.1f, d}, {-0.4f, -0.1f, d}, // Front
		{-0.3f, 0.2f, -d}, {0.3f, 0.2f, -d}, {0.4f, -0.1f, -d}, {-0.4f, -0.1f, -d} // Back
	};
	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3fv(v2[0]); glVertex3fv(v2[1]); glVertex3fv(v2[2]); glVertex3fv(v2[3]);
	glNormal3f(0.0f, 0.0f, -1.0f);
	glVertex3fv(v2[4]); glVertex3fv(v2[7]); glVertex3fv(v2[6]); glVertex3fv(v2[5]);
	glNormal3f(0.0f, 1.0f, 0.0f);
	glVertex3fv(v2[0]); glVertex3fv(v2[4]); glVertex3fv(v2[5]); glVertex3fv(v2[1]);
	glNormal3f(0.0f, -1.0f, 0.0f);
	glVertex3fv(v2[3]); glVertex3fv(v2[2]); glVertex3fv(v2[6]); glVertex3fv(v2[7]);
	glNormal3f(0.7f, 0.3f, 0.0f);
	glVertex3fv(v2[1]); glVertex3fv(v2[5]); glVertex3fv(v2[6]); glVertex3fv(v2[2]);
	glNormal3f(-0.7f, 0.3f, 0.0f);
	glVertex3fv(v2[0]); glVertex3fv(v2[3]); glVertex3fv(v2[7]); glVertex3fv(v2[4]);
	glEnd();
	glPopMatrix();
}

// Draws the hip guards with a more angled and integrated design
void drawHipGuards3D() {
	glColor3f(0.8f, 0.6f, 0.2f); // Darker gold
	float guardDepth = 0.2f;
	float d = guardDepth / 2.0f;

	// --- Left Hip Guard ---
	glPushMatrix();
	// Position and rotate to flow from the lower torso
	glTranslatef(-0.4f, -0.8f, 0.0f);
	glRotatef(25.0f, 0.0f, 0.0f, 1.0f); // More pronounced angle

	glBegin(GL_QUADS);
	// Define points for a trapezoid-like prism
	float v[8][3] = {
		{-0.2f, 0.1f, d}, {0.2f, 0.1f, d}, {0.3f, -0.3f, d}, {-0.3f, -0.3f, d}, // Front
		{-0.2f, 0.1f, -d}, {0.2f, 0.1f, -d}, {0.3f, -0.3f, -d}, {-0.3f, -0.3f, -d} // Back
	};

	// Front Face
	glNormal3f(0.0f, 0.0f, 1.0f); glVertex3fv(v[0]); glVertex3fv(v[1]); glVertex3fv(v[2]); glVertex3fv(v[3]);
	// Back Face
	glNormal3f(0.0f, 0.0f, -1.0f); glVertex3fv(v[4]); glVertex3fv(v[7]); glVertex3fv(v[6]); glVertex3fv(v[5]);
	// Top Face
	glNormal3f(0.0f, 1.0f, 0.0f); glVertex3fv(v[0]); glVertex3fv(v[4]); glVertex3fv(v[5]); glVertex3fv(v[1]);
	// Bottom Face
	glNormal3f(0.0f, -1.0f, 0.0f); glVertex3fv(v[3]); glVertex3fv(v[2]); glVertex3fv(v[6]); glVertex3fv(v[7]);
	// Right Side Face
	glNormal3f(0.7f, 0.7f, 0.0f); glVertex3fv(v[1]); glVertex3fv(v[5]); glVertex3fv(v[6]); glVertex3fv(v[2]);
	// Left Side Face
	glNormal3f(-0.7f, 0.7f, 0.0f); glVertex3fv(v[0]); glVertex3fv(v[3]); glVertex3fv(v[7]); glVertex3fv(v[4]);
	glEnd();
	glPopMatrix();

	// --- Right Hip Guard ---
	glPushMatrix();
	glTranslatef(0.4f, -0.8f, 0.0f);
	glRotatef(-25.0f, 0.0f, 0.0f, 1.0f);

	glBegin(GL_QUADS);
	float v2[8][3] = {
		{-0.2f, 0.1f, d}, {0.2f, 0.1f, d}, {0.3f, -0.3f, d}, {-0.3f, -0.3f, d}, // Front
		{-0.2f, 0.1f, -d}, {0.2f, 0.1f, -d}, {0.3f, -0.3f, -d}, {-0.3f, -0.3f, -d} // Back
	};
	glNormal3f(0.0f, 0.0f, 1.0f); glVertex3fv(v2[0]); glVertex3fv(v2[1]); glVertex3fv(v2[2]); glVertex3fv(v2[3]);
	glNormal3f(0.0f, 0.0f, -1.0f); glVertex3fv(v2[4]); glVertex3fv(v2[7]); glVertex3fv(v2[6]); glVertex3fv(v2[5]);
	glNormal3f(0.0f, 1.0f, 0.0f); glVertex3fv(v2[0]); glVertex3fv(v2[4]); glVertex3fv(v2[5]); glVertex3fv(v2[1]);
	glNormal3f(0.0f, -1.0f, 0.0f); glVertex3fv(v2[3]); glVertex3fv(v2[2]); glVertex3fv(v2[6]); glVertex3fv(v2[7]);
	glNormal3f(0.7f, 0.7f, 0.0f); glVertex3fv(v2[1]); glVertex3fv(v2[5]); glVertex3fv(v2[6]); glVertex3fv(v2[2]);
	glNormal3f(-0.7f, 0.7f, 0.0f); glVertex3fv(v2[0]); glVertex3fv(v2[3]); glVertex3fv(v2[7]); glVertex3fv(v2[4]);
	glEnd();
	glPopMatrix();
}

// A powerful helper function to create a smooth, revolved 3D object (a lathe).
// It takes a 2D profile (an array of x,y points) and revolves it around the Y-axis.
// - profile: An array of 2D points defining the shape's outline.
// - num_points: The number of points in the profile array.
// - sides: The number of radial segments. More sides = smoother circle.
void drawLathedObject(float profile[][2], int num_points, int sides)
{
	// Loop through each segment of the profile line
	for (int i = 0; i < num_points - 1; ++i)
	{
		// Use GL_TRIANGLE_STRIP for efficiency and smooth connections
		glBegin(GL_TRIANGLE_STRIP);

		// Loop through each side of the revolution
		for (int j = 0; j <= sides; ++j)
		{
			float angle = (float)j / (float)sides * 2.0f * 3.14159f;
			float x, z;

			// --- Vertex at the current level (i) ---
			x = profile[i][0] * cos(angle);
			z = profile[i][0] * sin(angle);

			// For a simple revolved shape, the normal points outwards from the Y-axis.
			// We can add a Y component to the normal to account for the profile's slope
			// for more accurate lighting, but this is a great start.
			glNormal3f(cos(angle), 0.0f, sin(angle));
			glVertex3f(x, profile[i][1], z);

			// --- Vertex at the next level (i+1) ---
			x = profile[i + 1][0] * cos(angle);
			z = profile[i + 1][0] * sin(angle);

			// The normal here should ideally be calculated based on the slope.
			// For simplicity, we use a similar outward-pointing normal.
			glNormal3f(cos(angle), 0.0f, sin(angle));
			glVertex3f(x, profile[i + 1][1], z);
		}
		glEnd();
	}
}

// Draws a smooth, curved torso using the lathe function.
void drawSmoothTorso()
{
	glColor3f(1.0f, 0.84f, 0.0f); // Golden yellow

	// Define the 2D profile for the torso's hourglass shape.
	// Each pair is { X-radius, Y-height }.
	float torso_profile[][2] = {
		{0.18f, 0.85f},  // Top of torso (connects to neck)
		{0.35f, 0.70f},  // Widest part of chest
		{0.25f, 0.30f},  // Tapering towards waist
		{0.15f, 0.10f},  // Narrowest part of waist
		{0.20f, -0.1f},  // Start of hips
		{0.38f, -0.5f},  // Widest part of hips
		{0.35f, -0.8f}   // Bottom of torso
	};
	int torso_points = sizeof(torso_profile) / sizeof(torso_profile[0]);

	// Generate the 3D model with 20 sides for a smooth look.
	drawLathedObject(torso_profile, torso_points, 20);
}

// Draws a smooth, trapezoidal neck.
void drawSmoothNeck()
{
	glColor3f(1.0f, 0.84f, 0.0f); // Golden yellow

	// Profile for the neck: narrow at bottom, wide at top.
	float neck_profile[][2] = {
		{0.18f, 0.85f}, // Bottom of neck (matches torso top)
		{0.25f, 1.05f}  // Top of neck
	};
	int neck_points = sizeof(neck_profile) / sizeof(neck_profile[0]);
	drawLathedObject(neck_profile, neck_points, 16);
}

// Draws smooth, cylindrical arms.
void drawSmoothArms()
{
	glColor3f(1.0f, 0.84f, 0.0f); // Golden yellow

	// --- Left Arm ---
	glPushMatrix();
	glTranslatef(-0.6f, 0.8f, 0.0f); // Position the arm
	glRotatef(10, 0, 0, 1); // Slight downward angle

	// Profile for the upper arm
	float upper_arm_profile[][2] = { {0.08f, 0.0f}, {0.08f, -0.5f} };
	drawLathedObject(upper_arm_profile, 2, 12);

	// Profile for the lower arm
	glTranslatef(0.0f, -0.5f, 0.0f); // Move down
	float lower_arm_profile[][2] = { {0.07f, 0.0f}, {0.07f, -0.4f} };
	drawLathedObject(lower_arm_profile, 2, 12);

	// Hand (simple cuboid for now)
	glTranslatef(0.0f, -0.45f, 0.0f);
	glScalef(0.16f, 0.1f, 0.16f);
	drawCuboid(1, 1, 1);

	glPopMatrix();

	// --- Right Arm ---
	glPushMatrix();
	glTranslatef(0.6f, 0.8f, 0.0f);
	glRotatef(-10, 0, 0, 1);
	drawLathedObject(upper_arm_profile, 2, 12);
	glTranslatef(0.0f, -0.5f, 0.0f);
	drawLathedObject(lower_arm_profile, 2, 12);
	glTranslatef(0.0f, -0.45f, 0.0f);
	glScalef(0.16f, 0.1f, 0.16f);
	drawCuboid(1, 1, 1);
	glPopMatrix();
}

// Draws the other parts using the old cuboid method for now,
// as they are more angular. We can refine them later.
void drawAngularParts()
{
	// --- Shoulder Pads ---
	// (We reuse the old function as shoulder pads are angular, not round)
	drawShoulderPads3D();

	// --- Hip Guards ---
	// (Reusing old function)
	drawHipGuards3D();

	// --- Belt & Buckle ---
	// (Reusing old function, but we can make the belt smooth later if needed)
	drawBelt3D();
}

// -- Main Display Function --

void display()
{
	// ... (no changes to the top part of display) ...
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);

	// This is ESSENTIAL for the new smooth look.
	glShadeModel(GL_SMOOTH);

	GLfloat light_pos[] = { 5.0f, 5.0f, 5.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

	GLfloat ambient_light[] = { 0.3f, 0.3f, 0.3f, 1.0f };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient_light);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, 800.0 / 600.0, 1.0, 100.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f, -0.5f, -5.0f);
	glRotatef(rotateX, 1.0f, 0.0f, 0.0f);
	glRotatef(rotateY, 0.0f, 1.0f, 0.0f);

	// --- CALL THE NEW SMOOTH DRAWING FUNCTIONS ---
	drawSmoothTorso();
	drawSmoothNeck();
	drawSmoothArms();

	// Call a function for the remaining angular parts.
	drawAngularParts();

	glDisable(GL_LIGHTING);
}

//--------------------------------------------------------------------

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow)
{
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpfnWndProc = WindowProcedure;
	wc.lpszClassName = WINDOW_TITLE;
	wc.style = CS_HREDRAW | CS_VREDRAW;

	if (!RegisterClassEx(&wc)) return false;

	HWND hWnd = CreateWindow(WINDOW_TITLE, WINDOW_TITLE, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
		NULL, NULL, wc.hInstance, NULL);

	//--------------------------------
	//	Initialize window for OpenGL
	//--------------------------------

	HDC hdc = GetDC(hWnd);

	//	initialize pixel format for the window
	initPixelFormat(hdc);

	//	get an openGL context
	HGLRC hglrc = wglCreateContext(hdc);

	//	make context current
	if (!wglMakeCurrent(hdc, hglrc)) return false;

	//--------------------------------
	//	End initialization
	//--------------------------------

	ShowWindow(hWnd, nCmdShow);

	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		display();

		SwapBuffers(hdc);
	}

	UnregisterClass(WINDOW_TITLE, wc.hInstance);

	return true;
}
//--------------------------------------------------------------------