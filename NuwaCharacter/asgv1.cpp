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

// This function draws the torso with more segments for a smoother, curvier look.
void drawTorso3D()
{
	glColor3f(1.0f, 0.84f, 0.0f); // Golden yellow color
	float bodyDepth = 0.3f; // INCREASED: Overall depth of the torso
	float d = bodyDepth / 2.0f;

	// Define vertices for the torso outline (front face)
	// We will mirror these for the back and connect them for the sides
	// Each vertex needs its own normal for smooth shading

	// A utility to get a vertex coordinate more easily
#define V_FRONT(x, y) glVertex3f(x, y, d)
#define V_BACK(x, y)  glVertex3f(x, y, -d)

// Vertices to define the smooth curves of the torso (Y-coordinates: top to bottom)
// X values define the width at each Y level.
// The sketch has a very defined chest, then a narrow waist, then wider hips.
	float chestTopX = 0.35f;
	float chestMidY = 0.6f;
	float chestMidX = 0.28f; // Slightly narrower than top
	float waistY = 0.1f;
	float waistX = 0.1f;    // Very narrow waist
	float hipMidY = -0.4f;
	float hipMidX = 0.3f;   // Wider hips
	float hipBottomY = -0.8f;
	float hipBottomX = 0.4f; // Widest at bottom of hips

	// --- Torso Segments (Top to Bottom) ---
	glBegin(GL_QUADS);
	// --- Front Faces ---
	glNormal3f(0.0f, 0.0f, 1.0f); // General normal for front
	// Top chest segment
	V_FRONT(-chestTopX, 0.8f); V_FRONT(chestTopX, 0.8f); V_FRONT(chestMidX, chestMidY); V_FRONT(-chestMidX, chestMidY);
	// Middle chest to waist segment
	V_FRONT(-chestMidX, chestMidY); V_FRONT(chestMidX, chestMidY); V_FRONT(waistX, waistY); V_FRONT(-waistX, waistY);
	// Waist to hip segment
	V_FRONT(-waistX, waistY); V_FRONT(waistX, waistY); V_FRONT(hipMidX, hipMidY); V_FRONT(-hipMidX, hipMidY);
	// Hip to bottom segment
	V_FRONT(-hipMidX, hipMidY); V_FRONT(hipMidX, hipMidY); V_FRONT(hipBottomX, hipBottomY); V_FRONT(-hipBottomX, hipBottomY);

	// --- Back Faces --- (vertices in reverse order for correct normal)
	glNormal3f(0.0f, 0.0f, -1.0f); // General normal for back
	V_BACK(-chestTopX, 0.8f); V_BACK(-chestMidX, chestMidY); V_BACK(chestMidX, chestMidY); V_BACK(chestTopX, 0.8f);
	V_BACK(-chestMidX, chestMidY); V_BACK(-waistX, waistY); V_BACK(waistX, waistY); V_BACK(chestMidX, chestMidY);
	V_BACK(-waistX, waistY); V_BACK(-hipMidX, hipMidY); V_BACK(hipMidX, hipMidY); V_BACK(waistX, waistY);
	V_BACK(-hipMidX, hipMidY); V_BACK(-hipBottomX, hipBottomY); V_BACK(hipBottomX, hipBottomY); V_BACK(hipMidX, hipMidY);

	// --- Side Faces --- (connect front and back)
	// Top cap
	glNormal3f(0.0f, 1.0f, 0.0f); // Normal pointing up
	V_FRONT(-chestTopX, 0.8f); V_BACK(-chestTopX, 0.8f); V_BACK(chestTopX, 0.8f); V_FRONT(chestTopX, 0.8f);

	// Left Side Segments (normals point left, slightly outwards for curve)
	glNormal3f(-1.0f, 0.0f, 0.0f); V_FRONT(-chestTopX, 0.8f); V_FRONT(-chestMidX, chestMidY); V_BACK(-chestMidX, chestMidY); V_BACK(-chestTopX, 0.8f);
	glNormal3f(-1.0f, 0.0f, 0.0f); V_FRONT(-chestMidX, chestMidY); V_FRONT(-waistX, waistY); V_BACK(-waistX, waistY); V_BACK(-chestMidX, chestMidY);
	glNormal3f(-1.0f, 0.0f, 0.0f); V_FRONT(-waistX, waistY); V_FRONT(-hipMidX, hipMidY); V_BACK(-hipMidX, hipMidY); V_BACK(-waistX, waistY);
	glNormal3f(-1.0f, 0.0f, 0.0f); V_FRONT(-hipMidX, hipMidY); V_FRONT(-hipBottomX, hipBottomY); V_BACK(-hipBottomX, hipBottomY); V_BACK(-hipMidX, hipMidY);

	// Right Side Segments (normals point right, slightly outwards for curve)
	glNormal3f(1.0f, 0.0f, 0.0f); V_FRONT(chestTopX, 0.8f); V_BACK(chestTopX, 0.8f); V_BACK(chestMidX, chestMidY); V_FRONT(chestMidX, chestMidY);
	glNormal3f(1.0f, 0.0f, 0.0f); V_FRONT(chestMidX, chestMidY); V_BACK(chestMidX, chestMidY); V_BACK(waistX, waistY); V_FRONT(waistX, waistY);
	glNormal3f(1.0f, 0.0f, 0.0f); V_FRONT(waistX, waistY); V_BACK(waistX, waistY); V_BACK(hipMidX, hipMidY); V_FRONT(hipMidX, hipMidY);
	glNormal3f(1.0f, 0.0f, 0.0f); V_FRONT(hipMidX, hipMidY); V_BACK(hipMidX, hipMidY); V_BACK(hipBottomX, hipBottomY); V_FRONT(hipBottomX, hipBottomY);

	// Bottom Cap
	glNormal3f(0.0f, -1.0f, 0.0f); // Normal pointing down
	V_FRONT(-hipBottomX, hipBottomY); V_BACK(-hipBottomX, hipBottomY); V_BACK(hipBottomX, hipBottomY); V_FRONT(hipBottomX, hipBottomY);
	glEnd();

#undef V_FRONT
#undef V_BACK
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

// Draws the neck as a trapezoidal prism, wider at the top,
// to connect smoothly to the head (which will be added later).
void drawNeck3D() {
	glColor3f(1.0f, 0.84f, 0.0f); // Gold colour

	float neckDepth = 0.2f; // Depth of the neck
	float d = neckDepth / 2.0f;

	glPushMatrix();
	// Position the neck to sit on top of the torso
	glTranslatef(0.0f, 0.85f, 0.0f); // Adjusted Y position to sit on the torso

	glBegin(GL_QUADS);
	// Define vertices for the neck. Wider at top, narrower at bottom.
	// Y-coordinates: top_Y, bottom_Y
	// X-coordinates: top_left_X, top_right_X, bottom_left_X, bottom_right_X

	// Top of neck (where head would connect)
	float topX_outer = 0.15f;
	float topX_inner = 0.1f;
	float topY = 0.2f;

	// Bottom of neck (where it connects to torso)
	float bottomX_outer = 0.1f;
	float bottomX_inner = 0.05f;
	float bottomY = 0.0f;

	// Front Face
	glNormal3f(0.0f, 0.0f, 1.0f); // Normal pointing forward
	glVertex3f(-bottomX_outer, bottomY, d); // Bottom-left
	glVertex3f(bottomX_outer, bottomY, d);  // Bottom-right
	glVertex3f(topX_outer, topY, d);    // Top-right
	glVertex3f(-topX_outer, topY, d);   // Top-left

	// Back Face (reverse order of front vertices to ensure correct normal direction)
	glNormal3f(0.0f, 0.0f, -1.0f); // Normal pointing backward
	glVertex3f(-bottomX_outer, bottomY, -d);
	glVertex3f(-topX_outer, topY, -d);
	glVertex3f(topX_outer, topY, -d);
	glVertex3f(bottomX_outer, bottomY, -d);

	// Right Side Face
	glNormal3f(1.0f, 0.0f, 0.0f); // Normal pointing right
	glVertex3f(bottomX_outer, bottomY, d);
	glVertex3f(bottomX_outer, bottomY, -d);
	glVertex3f(topX_outer, topY, -d);
	glVertex3f(topX_outer, topY, d);

	// Left Side Face
	glNormal3f(-1.0f, 0.0f, 0.0f); // Normal pointing left
	glVertex3f(-bottomX_outer, bottomY, d);
	glVertex3f(-topX_outer, topY, d);
	glVertex3f(-topX_outer, topY, -d);
	glVertex3f(-bottomX_outer, bottomY, -d);

	// Top Face (horizontal)
	glNormal3f(0.0f, 1.0f, 0.0f); // Normal pointing up
	glVertex3f(-topX_outer, topY, d);
	glVertex3f(topX_outer, topY, d);
	glVertex3f(topX_outer, topY, -d);
	glVertex3f(-topX_outer, topY, -d);

	// Bottom Face (horizontal, connects to torso)
	glNormal3f(0.0f, -1.0f, 0.0f); // Normal pointing down
	glVertex3f(-bottomX_outer, bottomY, d);
	glVertex3f(-bottomX_outer, bottomY, -d);
	glVertex3f(bottomX_outer, bottomY, -d);
	glVertex3f(bottomX_outer, bottomY, d);

	glEnd();
	glPopMatrix();
}

// Draws the shoulder pads as 3D prisms, more akin to the sketch
void drawShoulderPads3D() {
	glColor3f(0.9f, 0.7f, 0.1f); // Slightly different shade of gold
	float padDepth = 0.25f; // How thick the pads are
	float d = padDepth / 2.0f;

	// --- Left Shoulder Pad ---
	glPushMatrix();
	// Position further out and slightly lower to align with new torso
	glTranslatef(-0.4f, 0.75f, 0.0f); // Adjusted Y and X position
	glRotatef(10.0f, 0.0f, 0.0f, 1.0f); // Slight outward rotation

	glBegin(GL_QUADS);
	// Adjusted vertices for a wider, more angular pad
	// Note: Vertices are defined relative to the current translated/rotated origin
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
	glTranslatef(0.4f, 0.75f, 0.0f); // Adjusted Y and X position
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

// Draws the arms and hands
void drawArms()
{
	glColor3f(1.0f, 0.84f, 0.0f); // Gold colour
	float arm_depth = 0.15f; // Make arms a bit thicker

	// Left Arm
	glPushMatrix();
	glTranslatef(-0.6f, 0.45f, 0.0f); // Position at left shoulder

	// Upper arm
	glPushMatrix();
	glScalef(1.0f, 5.0f, 1.0f); // Make the cuboid long and thin
	drawCuboid(0.1f, 0.1f, 0.1f);
	glPopMatrix();

	// Forearm
	glPushMatrix();
	glTranslatef(0.0f, -0.5f, 0.0f); // Move down
	glScalef(1.0f, 4.0f, 1.0f);
	drawCuboid(0.1f, 0.1f, 0.1f);
	glPopMatrix();

	glPopMatrix();

	// Right Arm (similar logic)
	glPushMatrix();
	glTranslatef(0.6f, 0.45f, 0.0f); // Position at right shoulder

	// Upper arm
	glPushMatrix();
	glScalef(1.0f, 5.0f, 1.0f);
	drawCuboid(0.1f, 0.1f, 0.1f);
	glPopMatrix();

	// Forearm
	glPushMatrix();
	glTranslatef(0.0f, -0.5f, 0.0f);
	glScalef(1.0f, 4.0f, 1.0f);
	drawCuboid(0.1f, 0.1f, 0.1f);
	glPopMatrix();

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

// -- Main Display Function --

void display()
{
	// Because rotateX and rotateY are now global,
	// we must REMOVE the "static float" declarations from here.

	glClearColor(1.0, 1.0, 1.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
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

	// The rotation angles are now controlled by the mouse via the global variables.
	glRotatef(rotateX, 1.0f, 0.0f, 0.0f);
	glRotatef(rotateY, 0.0f, 1.0f, 0.0f);

	// --- DRAWING FUNCTIONS ---
	drawTorso3D();
	drawBelt3D();
	drawNeck3D();
	drawShoulderPads3D();
	drawArms();
	drawHipGuards3D();

	// --- (START) REMOVE AUTOMATIC ROTATION ---
	// The following lines that made the model spin automatically are now deleted.
	// rotateY += 0.1f; 
	// if (rotateY > 360.0f) {
	//     rotateY -= 360.0f;
	// }
	// --- (END) REMOVE AUTOMATIC ROTATION ---

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