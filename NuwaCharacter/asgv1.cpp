#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <math.h>

#pragma comment (lib, "OpenGL32.lib")
#pragma comment (lib, "GLU32.lib")

#define WINDOW_TITLE "OpenGL Window"

// These variables will store the rotation angles.
float rotateX = 15.0f;
float rotateY = 0.0f;

// These variables will help us track the mouse movement.
bool isDragging = false;
int lastMouseX = 0;
int lastMouseY = 0;

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

	case WM_LBUTTONDOWN:
	{
		isDragging = true;
		lastMouseX = LOWORD(lParam);
		lastMouseY = HIWORD(lParam);
		break;
	}

	case WM_LBUTTONUP:
	{
		isDragging = false;
		break;
	}

	case WM_MOUSEMOVE:
	{
		if (isDragging)
		{
			int currentMouseX = LOWORD(lParam);
			int currentMouseY = HIWORD(lParam);
			float deltaX = (float)(currentMouseX - lastMouseX);
			float deltaY = (float)(currentMouseY - lastMouseY);
			rotateY += deltaX * 0.2f;
			rotateX += deltaY * 0.2f;
			lastMouseX = currentMouseX;
			lastMouseY = currentMouseY;
		}
		break;
	}

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
	int n = ChoosePixelFormat(hdc, &pfd);
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

void drawCuboid(float width, float height, float depth)
{
	float w = width / 2.0f;
	float h = height / 2.0f;
	float d = depth / 2.0f;

	glBegin(GL_QUADS);
	// Front Face
	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-w, -h, d); glVertex3f(w, -h, d); glVertex3f(w, h, d); glVertex3f(-w, h, d);
	// Back Face
	glNormal3f(0.0f, 0.0f, -1.0f);
	glVertex3f(-w, -h, -d); glVertex3f(-w, h, -d); glVertex3f(w, h, -d); glVertex3f(w, -h, -d);
	// Top Face
	glNormal3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-w, h, -d); glVertex3f(-w, h, d); glVertex3f(w, h, d); glVertex3f(w, h, -d);
	// Bottom Face
	glNormal3f(0.0f, -1.0f, 0.0f);
	glVertex3f(-w, -h, -d); glVertex3f(w, -h, -d); glVertex3f(w, -h, d); glVertex3f(-w, -h, d);
	// Right face
	glNormal3f(1.0f, 0.0f, 0.0f);
	glVertex3f(w, -h, -d); glVertex3f(w, h, -d); glVertex3f(w, h, d); glVertex3f(w, -h, d);
	// Left Face
	glNormal3f(-1.0f, 0.0f, 0.0f);
	glVertex3f(-w, -h, -d); glVertex3f(-w, -h, d); glVertex3f(-w, h, d); glVertex3f(-w, h, -d);
	glEnd();
}

void drawShoulderPads3D() {
	glColor3f(0.9f, 0.7f, 0.1f);
	float padDepth = 0.25f;
	float d = padDepth / 2.0f;

	// --- Left Shoulder Pad ---
	glPushMatrix();
	glTranslatef(-0.45f, 0.73f, 0.0f);
	glRotatef(10.0f, 0.0f, 0.0f, 1.0f);
	glBegin(GL_QUADS);
	float v[8][3] = {
		{-0.3f, 0.2f, d}, {0.3f, 0.2f, d}, {0.4f, -0.1f, d}, {-0.4f, -0.1f, d},
		{-0.3f, 0.2f, -d}, {0.3f, 0.2f, -d}, {0.4f, -0.1f, -d}, {-0.4f, -0.1f, -d}
	};
	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3fv(v[0]); glVertex3fv(v[1]); glVertex3fv(v[2]); glVertex3fv(v[3]);
	glNormal3f(0.0f, 0.0f, -1.0f);
	glVertex3fv(v[4]); glVertex3fv(v[7]); glVertex3fv(v[6]); glVertex3fv(v[5]);
	glNormal3f(0.0f, 1.0f, 0.0f);
	glVertex3fv(v[0]); glVertex3fv(v[4]); glVertex3fv(v[5]); glVertex3fv(v[1]);
	glNormal3f(0.0f, -1.0f, 0.0f);
	glVertex3fv(v[3]); glVertex3fv(v[2]); glVertex3fv(v[6]); glVertex3fv(v[7]);
	glNormal3f(0.7f, 0.3f, 0.0f);
	glVertex3fv(v[1]); glVertex3fv(v[5]); glVertex3fv(v[6]); glVertex3fv(v[2]);
	glNormal3f(-0.7f, 0.3f, 0.0f);
	glVertex3fv(v[0]); glVertex3fv(v[3]); glVertex3fv(v[7]); glVertex3fv(v[4]);
	glEnd();
	glPopMatrix();

	// --- Right Shoulder Pad ---
	glPushMatrix();
	glTranslatef(0.45f, 0.73f, 0.0f);
	glRotatef(-10.0f, 0.0f, 0.0f, 1.0f);
	glBegin(GL_QUADS);
	float v2[8][3] = {
		{-0.3f, 0.2f, d}, {0.3f, 0.2f, d}, {0.4f, -0.1f, d}, {-0.4f, -0.1f, d},
		{-0.3f, 0.2f, -d}, {0.3f, 0.2f, -d}, {0.4f, -0.1f, -d}, {-0.4f, -0.1f, -d}
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


void drawLathedObject(float profile[][2], int num_points, int sides)
{
	float EPSILON = 0.0001f;
	for (int i = 0; i < num_points - 1; ++i)
	{
		glBegin(GL_TRIANGLE_STRIP);
		for (int j = 0; j <= sides; ++j)
		{
			float angle_rad = (float)j / (float)sides * 2.0f * 3.14159f;
			float cos_angle = cos(angle_rad);
			float sin_angle = sin(angle_rad);
			float dx = profile[i + 1][0] - profile[i][0];
			float dy = profile[i + 1][1] - profile[i][1];
			float normal_x_profile = -dy;
			float normal_y_profile = dx;
			float normal_len_2d = sqrt(normal_x_profile * normal_x_profile + normal_y_profile * normal_y_profile);
			if (normal_len_2d > EPSILON) {
				normal_x_profile /= normal_len_2d;
				normal_y_profile /= normal_len_2d;
			}
			else {
				normal_x_profile = 0.0f;
				normal_y_profile = 1.0f;
			}
			float normal_x_3d = normal_x_profile * cos_angle;
			float normal_z_3d = normal_x_profile * sin_angle;
			glVertex3f(profile[i][0] * cos_angle, profile[i][1], profile[i][0] * sin_angle);
			glNormal3f(normal_x_3d, normal_y_profile, normal_z_3d);
			glVertex3f(profile[i + 1][0] * cos_angle, profile[i + 1][1], profile[i + 1][0] * sin_angle);
			glNormal3f(normal_x_3d, normal_y_profile, normal_z_3d);
		}
		glEnd();
	}
}


void drawSmoothChest()
{
	glColor3f(1.0f, 0.84f, 0.0f);
	float chest_profile[][2] = {
		{0.18f, 0.85f},
		{0.35f, 0.70f},
		{0.38f, 0.55f},
		{0.30f, 0.40f},
		{0.25f, 0.25f}
	};
	int chest_points = sizeof(chest_profile) / sizeof(chest_profile[0]);
	drawLathedObject(chest_profile, chest_points, 20);
}


void drawSmoothLowerBodyAndSkirt()
{
	glColor3f(1.0f, 0.84f, 0.0f); // Golden yellow

	float lower_body_profile[][2] = {
		{0.18f, -0.05f}, // Top of lower waist (below the thin waist segment)
		{0.20f, -0.1f},  // Start of hips
		{0.38f, -0.5f},  // Widest part of hips
		{0.45f, -0.9f},  // Flare outwards for the skirt effect
		{0.40f, -1.0f},  // Gently curve inwards at the very bottom
		{0.30f, -1.0f}   // Flat bottom part for the base of the skirt
	};
	int lower_body_points = sizeof(lower_body_profile) / sizeof(lower_body_profile[0]);

	drawLathedObject(lower_body_profile, lower_body_points, 24);
}


void drawSmoothArms()
{
	glColor3f(1.0f, 0.84f, 0.0f);

	// --- Left Arm ---
	glPushMatrix();
	glTranslatef(-0.6f, 0.6f, 0.0f);
	glRotatef(10, 0, 0, 1);
	float upper_arm_profile[][2] = { {0.08f, 0.0f}, {0.08f, -0.5f} };
	drawLathedObject(upper_arm_profile, 2, 12);
	glTranslatef(0.0f, -0.5f, 0.0f);
	float lower_arm_profile[][2] = { {0.07f, 0.0f}, {0.07f, -0.4f} };
	drawLathedObject(lower_arm_profile, 2, 12);
	glTranslatef(0.0f, -0.45f, 0.0f);
	glScalef(0.16f, 0.1f, 0.16f);
	drawCuboid(1, 1, 1);
	glPopMatrix();

	// --- Right Arm ---
	glPushMatrix();
	glTranslatef(0.6f, 0.6f, 0.0f);
	glRotatef(-10, 0, 0, 1);
	drawLathedObject(upper_arm_profile, 2, 12);
	glTranslatef(0.0f, -0.5f, 0.0f);
	drawLathedObject(lower_arm_profile, 2, 12);
	glTranslatef(0.0f, -0.45f, 0.0f);
	glScalef(0.16f, 0.1f, 0.16f);
	drawCuboid(1, 1, 1);
	glPopMatrix();
}

void drawWaistWithVerticalLines()
{
	float waist_top_y = 0.25f;
	float waist_bottom_y = -0.05f;
	float top_radius = 0.25f;
	float bottom_radius = 0.18f;
	int sides = 24;
	int num_lines = 6;

	glBegin(GL_QUADS);
	for (int i = 0; i < sides; ++i)
	{
		float angle1 = (float)i / (float)sides * 2.0f * 3.14159f;
		float angle2 = (float)(i + 1) / (float)sides * 2.0f * 3.14159f;
		bool isLineSegment = (i % (sides / num_lines) == 0);

		if (isLineSegment) {
			glColor3f(0.8f, 0.6f, 0.0f);
		}
		else {
			glColor3f(1.0f, 0.84f, 0.0f);
		}

		float v1x = top_radius * cos(angle1);
		float v1z = top_radius * sin(angle1);
		float v2x = top_radius * cos(angle2);
		float v2z = top_radius * sin(angle2);
		float v3x = bottom_radius * cos(angle2);
		float v3z = bottom_radius * sin(angle2);
		float v4x = bottom_radius * cos(angle1);
		float v4z = bottom_radius * sin(angle1);

		float avg_angle = (angle1 + angle2) / 2.0f;
		glNormal3f(cos(avg_angle), 0.0f, sin(avg_angle));
		glVertex3f(v1x, waist_top_y, v1z);
		glVertex3f(v2x, waist_top_y, v2z);
		glVertex3f(v3x, waist_bottom_y, v3z);
		glVertex3f(v4x, waist_bottom_y, v4z);
	}
	glEnd();
}

// --- (MODIFIED) drawWaistBelt: Belt is now at the bottom of the thin waist and is wider ---
void drawWaistBelt()
{
	int sides = 40;
	// --- (MODIFIED) Increased belt thickness for visibility ---
	float belt_thickness = 0.06f; // WAS 0.04f

	// Position the belt at the exact bottom of the thin waist segment.
	float belt_y_position = -0.05f; // This is 'waist_bottom_y'
	float belt_radius_at_position = 0.18f; // Matches 'bottom_radius' of vertical lines.

	// --- Belt Strap ---
	glColor3f(0.8f, 0.6f, 0.2f); // Darker gold for the belt strap
	glPushMatrix();
	glTranslatef(0.0f, belt_y_position, 0.0f); // Position at the calculated Y

	glBegin(GL_QUAD_STRIP);
	for (int i = 0; i <= sides; ++i) {
		float angle = (float)i / (float)sides * 2.0f * 3.14159f;
		float cos_a = cos(angle);
		float sin_a = sin(angle);

		// Adjusted dip for style - more subtle in the Y direction
		float dip_y_offset = -0.02f * (cos_a + 1.0f) / 2.0f; // Subtle dip at front

		// Calculate outer and inner radii for the belt, making it wider at the front.
		float current_outer_radius = belt_radius_at_position + belt_thickness;
		float current_inner_radius = belt_radius_at_position - belt_thickness;

		// Make the belt itself slightly wider at the front to emphasize the V-shape (radial width)
		float radial_width_increase = 0.02f * (cos_a + 1.0f) / 2.0f;
		current_outer_radius += radial_width_increase;
		current_inner_radius -= radial_width_increase;


		// Normal for outer surface (pointing outwards in XY plane)
		glNormal3f(cos_a, 0.0f, sin_a);

		// Vertices for the top and bottom edges of the outer and inner parts of the belt
		// Outer top
		glVertex3f(current_outer_radius * cos_a, belt_thickness / 2.0f + dip_y_offset, current_outer_radius * sin_a);
		// Outer bottom
		glVertex3f(current_outer_radius * cos_a, -belt_thickness / 2.0f + dip_y_offset, current_outer_radius * sin_a);
		// Inner top
		glVertex3f(current_inner_radius * cos_a, belt_thickness / 2.0f + dip_y_offset, current_inner_radius * sin_a);
		// Inner bottom
		glVertex3f(current_inner_radius * cos_a, -belt_thickness / 2.0f + dip_y_offset, current_inner_radius * sin_a);
	}
	glEnd();
	glPopMatrix();

	// --- Buckle ---
	glPushMatrix();
	// Position the buckle at the front of the belt.
	// We need to calculate the exact front-most point of the belt, considering its variable radius.
	float front_belt_outer_radius_at_y = belt_radius_at_position + belt_thickness + 0.02f; // Max radial width at front (cos_a = 1)
	glTranslatef(0.0f, belt_y_position - 0.02f, front_belt_outer_radius_at_y + 0.01f); // Slightly in front and adjusted Y for buckle dip

	// Circular Buckle Housing
	glColor3f(0.8f, 0.6f, 0.2f); // Darker gold
	float buckle_outer_radius = 0.11f; // --- (MODIFIED) Made buckle larger --- WAS 0.09f
	float buckle_inner_radius = 0.08f; // --- (MODIFIED) Made inner ring larger proportionally --- WAS 0.07f
	float buckle_depth = 0.03f; // --- (MODIFIED) Made buckle deeper --- WAS 0.02f

	// Outer ring
	glBegin(GL_QUAD_STRIP);
	for (int i = 0; i <= 20; i++) {
		float angle = (float)i / 20.0f * 2.0f * 3.14159f;
		glNormal3f(cos(angle), sin(angle), 0.0f);
		glVertex3f(buckle_outer_radius * cos(angle), buckle_outer_radius * sin(angle), buckle_depth / 2.0f);
		glVertex3f(buckle_outer_radius * cos(angle), buckle_outer_radius * sin(angle), -buckle_depth / 2.0f);
	}
	glEnd();

	// Inner ring (to make it look hollow)
	glBegin(GL_QUAD_STRIP);
	for (int i = 0; i <= 20; i++) {
		float angle = (float)i / 20.0f * 2.0f * 3.14159f;
		glNormal3f(-cos(angle), -sin(angle), 0.0f); // Inverted normal for inner surface
		glVertex3f(buckle_inner_radius * cos(angle), buckle_inner_radius * sin(angle), -buckle_depth / 2.0f);
		glVertex3f(buckle_inner_radius * cos(angle), buckle_inner_radius * sin(angle), buckle_depth / 2.0f);
	}
	glEnd();

	// Front face of the buckle ring
	glBegin(GL_QUAD_STRIP);
	glNormal3f(0.0f, 0.0f, 1.0f);
	for (int i = 0; i <= 20; i++) {
		float angle = (float)i / 20.0f * 2.0f * 3.14159f;
		glVertex3f(buckle_outer_radius * cos(angle), buckle_outer_radius * sin(angle), buckle_depth / 2.0f);
		glVertex3f(buckle_inner_radius * cos(angle), buckle_inner_radius * sin(angle), buckle_depth / 2.0f);
	}
	glEnd();

	// Inner Triangle Gem (smaller and placed within the ring)
	glColor3f(0.9f, 0.9f, 0.9f); // Light grey/silver
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, buckle_depth / 2.0f + 0.005f); // Slightly in front of the buckle ring
	glBegin(GL_TRIANGLES);
	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, buckle_inner_radius * 0.7f, 0.0f);
	glVertex3f(buckle_inner_radius * 0.6f, -buckle_inner_radius * 0.5f, 0.0f);
	glVertex3f(-buckle_inner_radius * 0.6f, -buckle_inner_radius * 0.5f, 0.0f);
	glEnd();
	glPopMatrix();

	glPopMatrix(); // Pop buckle transform
}

void drawArmorCollar()
{
	glColor3f(0.8f, 0.6f, 0.0f);
	float lower_collar_profile[][2] = {
		{0.20f, 0.85f}, {0.38f, 0.88f}, {0.38f, 0.84f}, {0.20f, 0.82f}
	};
	int lower_collar_points = sizeof(lower_collar_profile) / sizeof(lower_collar_profile[0]);
	drawLathedObject(lower_collar_profile, lower_collar_points, 24);

	glColor3f(0.9f, 0.7f, 0.1f);
	float upper_collar_profile[][2] = {
		{0.18f, 0.88f}, {0.24f, 0.90f}, {0.24f, 0.87f}, {0.18f, 0.86f}
	};
	int upper_collar_points = sizeof(upper_collar_profile) / sizeof(upper_collar_profile[0]);
	drawLathedObject(upper_collar_profile, upper_collar_points, 20);

	glColor3f(1.0f, 0.84f, 0.0f);
	float neck_profile[][2] = {
		{0.17f, 0.88f}, {0.17f, 0.95f}
	};
	drawLathedObject(neck_profile, 2, 16);
}


void display()
{
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
	glRotatef(rotateX, 1.0f, 0.0f, 0.0f);
	glRotatef(rotateY, 0.0f, 1.0f, 0.0f);

	// --- Drawing Calls ---
	drawSmoothChest();
	drawWaistWithVerticalLines();
	drawSmoothLowerBodyAndSkirt();

	drawWaistBelt(); // Belt is now at the correct, lower waist position and is wider

	drawArmorCollar();

	drawSmoothArms();
	drawShoulderPads3D();

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
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
	HDC hdc = GetDC(hWnd);
	initPixelFormat(hdc);
	HGLRC hglrc = wglCreateContext(hdc);
	if (!wglMakeCurrent(hdc, hglrc)) return false;
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