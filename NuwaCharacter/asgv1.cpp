#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <math.h>
#include <vector>

#pragma comment (lib, "OpenGL32.lib")
#pragma comment (lib, "GLU32.lib")

#define WINDOW_TITLE "OpenGL Window"

// --- NEW variables for Delta Time calculation ---
LARGE_INTEGER g_timer_frequency;
LARGE_INTEGER g_last_frame_time;

float g_rainbow_offset = 0.0f; // For halo animation

HDC g_hDC = nullptr;   // global device context
HGLRC g_hRC = nullptr; // global rendering context

// These variables will store the rotation angles.
float rotateX = 15.0f;
float rotateY = 0.0f;
float zoomFactor = -5.0f; // Global variable for camera zoom

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

	case WM_MOUSEWHEEL:
	{
		int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		if (zDelta > 0)
		{
			zoomFactor += 0.5f;
		}
		else if (zDelta < 0)
		{
			zoomFactor -= 0.5f;
		}
		if (zoomFactor > -2.0f) zoomFactor = -2.0f;
		if (zoomFactor < -20.0f) zoomFactor = -20.0f;
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

void drawHand(bool isLeftHand)
{
	glColor3f(1.0f, 0.84f, 0.0f); // Golden yellow

	// --- 1. Draw the Palm ---
	glPushMatrix();
	// Position the palm correctly at the wrist
	glTranslatef(0.0f, -0.05f, 0.0f);
	glRotatef(10.0f, 1.0f, 0.0f, 0.0f); // Slight downward angle for the palm
	// Draw a flattened cuboid for the palm (Width, Height, Depth)
	drawCuboid(0.18f, 0.22f, 0.06f);
	glPopMatrix();


	// --- 2. Draw the Four Fingers ---
	for (int i = 0; i < 4; i++)
	{
		glPushMatrix(); // Save the wrist's state

		// --- Knuckle Placement ---
		// Position each finger's knuckle on the bottom edge of the palm
		float xPos = isLeftHand ? (-0.0675f + i * 0.045f) : (0.0675f - i * 0.045f);
		glTranslatef(xPos, -0.15f, 0.01f);
		// Base rotation for the whole finger to make it curl slightly
		glRotatef(20.0f, 1.0f, 0.0f, 0.0f);

		// --- Finger Segment 1 (Base) ---
		float seg1_len = 0.07f;
		glTranslatef(0.0f, -seg1_len / 2.0f, 0.0f); // Move to the center of the segment
		drawCuboid(0.035f, seg1_len, 0.04f);      // Draw the segment

		// --- Finger Segment 2 (Middle) ---
		glTranslatef(0.0f, -seg1_len / 2.0f, 0.0f); // Move to the joint at the end of the first segment
		glRotatef(25.0f, 1.0f, 0.0f, 0.0f);      // Rotate the joint
		float seg2_len = 0.06f;
		glTranslatef(0.0f, -seg2_len / 2.0f, 0.0f); // Move to the center of the second segment
		drawCuboid(0.032f, seg2_len, 0.037f);     // Draw the segment

		// --- Finger Segment 3 (Tip) ---
		glTranslatef(0.0f, -seg2_len / 2.0f, 0.0f); // Move to the joint at the end of the second segment
		glRotatef(25.0f, 1.0f, 0.0f, 0.0f);      // Rotate the joint
		float seg3_len = 0.05f;
		glTranslatef(0.0f, -seg3_len / 2.0f, 0.0f); // Move to the center of the third segment
		drawCuboid(0.03f, seg3_len, 0.034f);      // Draw the segment

		glPopMatrix(); // Restore to the wrist's state for the next finger
	}

	// --- 3. Draw the Thumb ---
	glPushMatrix(); // Save the wrist's state

	// Position the thumb's base on the side of the palm
	glTranslatef(isLeftHand ? -0.1f : 0.1f, -0.08f, -0.01f);
	glRotatef(isLeftHand ? 40.0f : -40.0f, 0.0f, 1.0f, 0.0f); // Rotate it outwards
	glRotatef(30.0f, 0.0f, 0.0f, isLeftHand ? 1.0f : -1.0f); // Angle it across the palm

	// --- Thumb Segment 1 (Base) ---
	float thumb1_len = 0.06f;
	glTranslatef(0.0f, -thumb1_len / 2.0f, 0.0f);
	drawCuboid(0.04f, thumb1_len, 0.04f);

	// --- Thumb Segment 2 (Tip) ---
	glTranslatef(0.0f, -thumb1_len / 2.0f, 0.0f);
	glRotatef(30.0f, 1.0f, 0.0f, 0.0f);
	float thumb2_len = 0.05f;
	glTranslatef(0.0f, -thumb2_len / 2.0f, 0.0f);
	drawCuboid(0.037f, thumb2_len, 0.037f);

	glPopMatrix(); // Restore to the wrist's state
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
	float upper_arm_profile[][2] = { {0.08f, 0.0f}, {0.08f, -0.5f} };
	float lower_arm_profile[][2] = { {0.07f, 0.0f}, {0.07f, -0.4f} };

	// --- Left Arm ---
	// This block now uses the rotations from the original RIGHT arm.
	glPushMatrix();
	glTranslatef(-0.6f, 0.6f, 0.0f); // Stays on the left side of the body

	// Step 1: SHOULDER: Using original Right Arm's rotation
	glRotatef(10.0f, 0.0f, 0.0f, 1.0f);

	drawLathedObject(upper_arm_profile, 2, 12);
	glTranslatef(0.0f, -0.5f, 0.0f);

	// Step 2: ELBOW: Using original Right Arm's rotation
	glRotatef(25.0f, 1.0f, 0.0f, 0.0f);

	drawLathedObject(lower_arm_profile, 2, 12);
	glTranslatef(0.0f, -0.4f, 0.0f);

	// Step 3: WRIST: Using original Right Arm's rotation
	glRotatef(-70.0f, 0.0f, 1.0f, 0.0f);

	drawHand(false); // Using original Right Arm's hand model (isLeftHand = false)
	glPopMatrix();

	// --- Right Arm ---
	// This block now uses the rotations from the original LEFT arm.
	glPushMatrix();
	glTranslatef(0.6f, 0.6f, 0.0f); // Stays on the right side of the body

	// Step 1: SHOULDER: Using original Left Arm's rotation
	glRotatef(-10.0f, 0.0f, 0.0f, 1.0f);

	drawLathedObject(upper_arm_profile, 2, 12);
	glTranslatef(0.0f, -0.5f, 0.0f);

	// Step 2: ELBOW: Using original Left Arm's rotation
	glRotatef(25.0f, 1.0f, 0.0f, 0.0f);

	drawLathedObject(lower_arm_profile, 2, 12);
	glTranslatef(0.0f, -0.4f, 0.0f);

	// Step 3: WRIST: Using original Left Arm's rotation
	glRotatef(70.0f, 0.0f, 1.0f, 0.0f);

	drawHand(true); // Using original Left Arm's hand model (isLeftHand = true)
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

void drawWaistBelt()
{
	glColor3f(0.8f, 0.6f, 0.2f); // Darker gold for the belt strap

	// --- Define the geometry of the V-shaped belt ---
	float belt_y_start = -0.05f;
	float belt_y_end = -0.20f;
	float belt_width = 0.1f;
	int segments = 10; // Segments for EACH strap (front-right, front-left, back)
	const float PI = 3.14159f;

	float SURFACE_OFFSET = 0.02f;

	float radius_at_y_start = 0.18f;
	float radius_at_y_end = 0.22f;

	// --- Draw Right Front Strap (V-shape) ---
	glBegin(GL_QUAD_STRIP);
	for (int i = 0; i <= segments; i++) {
		float t = (float)i / segments;
		float current_y = belt_y_start + t * (belt_y_end - belt_y_start);
		float current_radius = radius_at_y_start + t * (radius_at_y_end - radius_at_y_start);
		float angle = t * (PI / 2.0f);

		float x = cos(angle) * (current_radius + SURFACE_OFFSET);
		float z = sin(angle) * (current_radius + SURFACE_OFFSET);

		glNormal3f(cos(angle), 0.2f, sin(angle));
		glVertex3f(x, current_y + belt_width / 2.0f, z);
		glVertex3f(x, current_y - belt_width / 2.0f, z);
	}
	glEnd();

	// --- Draw Left Front Strap (V-shape) ---
	glBegin(GL_QUAD_STRIP);
	for (int i = 0; i <= segments; i++) {
		float t = (float)i / segments;
		float current_y = belt_y_start + t * (belt_y_end - belt_y_start);
		float current_radius = radius_at_y_start + t * (radius_at_y_end - radius_at_y_start);
		float angle = PI - (t * (PI / 2.0f));

		float x = cos(angle) * (current_radius + SURFACE_OFFSET);
		float z = sin(angle) * (current_radius + SURFACE_OFFSET);

		glNormal3f(cos(angle), 0.2f, sin(angle));
		glVertex3f(x, current_y + belt_width / 2.0f, z);
		glVertex3f(x, current_y - belt_width / 2.0f, z);
	}
	glEnd();

	// --- Draw the back half of the belt ---
	glBegin(GL_QUAD_STRIP);
	for (int i = 0; i <= segments; i++) {
		float t = (float)i / segments;
		// Angle from 180 (PI) to 360 (2*PI)
		float angle = PI + t * PI;
		float x = cos(angle) * (radius_at_y_start + SURFACE_OFFSET);
		float z = sin(angle) * (radius_at_y_start + SURFACE_OFFSET);

		glNormal3f(cos(angle), 0.0f, sin(angle));
		// The Y position is constant for the back strap
		glVertex3f(x, belt_y_start + belt_width / 2.0f, z);
		glVertex3f(x, belt_y_start - belt_width / 2.0f, z);
	}
	glEnd();

	// --- Buckle (only at the front) ---
	glPushMatrix();
	float t_mid = 0.5f;
	float buckle_y = belt_y_start + t_mid * (belt_y_end - belt_y_start);
	float radius_at_buckle = radius_at_y_start + t_mid * (radius_at_y_end - radius_at_y_start);
	float buckle_z = radius_at_buckle + SURFACE_OFFSET + 0.02f;
	glTranslatef(0.0f, buckle_y, buckle_z);

	glColor3f(0.8f, 0.6f, 0.2f);
	float buckle_outer_radius = 0.11f;
	float buckle_inner_radius = 0.08f;
	float buckle_depth = 0.03f;

	glBegin(GL_QUAD_STRIP);
	for (int i = 0; i <= 20; i++) {
		float angle_buckle = (float)i / 20.0f * 2.0f * PI;
		glNormal3f(cos(angle_buckle), sin(angle_buckle), 0.0f);
		glVertex3f(buckle_outer_radius * cos(angle_buckle), buckle_outer_radius * sin(angle_buckle), buckle_depth / 2.0f);
		glVertex3f(buckle_outer_radius * cos(angle_buckle), buckle_outer_radius * sin(angle_buckle), -buckle_depth / 2.0f);
	}
	glEnd();
	glBegin(GL_QUAD_STRIP);
	for (int i = 0; i <= 20; i++) {
		float angle_buckle = (float)i / 20.0f * 2.0f * PI;
		glNormal3f(-cos(angle_buckle), -sin(angle_buckle), 0.0f);
		glVertex3f(buckle_inner_radius * cos(angle_buckle), buckle_inner_radius * sin(angle_buckle), -buckle_depth / 2.0f);
		glVertex3f(buckle_inner_radius * cos(angle_buckle), buckle_inner_radius * sin(angle_buckle), buckle_depth / 2.0f);
	}
	glEnd();
	glBegin(GL_QUAD_STRIP);
	glNormal3f(0.0f, 0.0f, 1.0f);
	for (int i = 0; i <= 20; i++) {
		float angle_buckle = (float)i / 20.0f * 2.0f * PI;
		glVertex3f(buckle_outer_radius * cos(angle_buckle), buckle_outer_radius * sin(angle_buckle), buckle_depth / 2.0f);
		glVertex3f(buckle_inner_radius * cos(angle_buckle), buckle_inner_radius * sin(angle_buckle), buckle_depth / 2.0f);
	}
	glEnd();

	glColor3f(0.9f, 0.9f, 0.9f);
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, buckle_depth / 2.0f + 0.005f);
	glBegin(GL_TRIANGLES);
	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, -buckle_inner_radius * 0.7f, 0.0f);
	glVertex3f(buckle_inner_radius * 0.6f, buckle_inner_radius * 0.5f, 0.0f);
	glVertex3f(-buckle_inner_radius * 0.6f, buckle_inner_radius * 0.5f, 0.0f);
	glEnd();
	glPopMatrix();

	glPopMatrix();
}

// --- Modified drawArmorCollar (REMOVE old neck part) ---
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

	// The old neck part is removed from here
	// No more: glColor3f(1.0f, 0.84f, 0.0f); float neck_profile[][2] = {{0.17f, 0.88f}, {0.17f, 0.95f}}; drawLathedObject(neck_profile, 2, 16);
}

// ----------------------------
// Lathed Sphere (array version)
// ----------------------------
void drawSphere(float r, int slices, int stacks)
{
	const int MAX_STACKS = 50; // adjust if needed
	float profile[MAX_STACKS + 1][2];

	int count = 0;
	for (int i = 0; i <= stacks; i++)
	{
		float theta = (float)i / stacks * 3.14159f; // 0 → PI
		float y = r * cos(theta);
		float x = r * sin(theta);
		profile[count][0] = x;
		profile[count][1] = y;
		count++;
	}

	drawLathedObject(profile, count, slices);
}

// ----------------------------
// Lathed Cone (array version)
// ----------------------------
void drawCone(float base, float height, int slices, int stacks)
{
	float profile[2][2] = {
		{base, 0.0f},
		{0.0f, height}
	};
	drawLathedObject(profile, 2, slices);
}

// --- UPDATED drawNeck function ---
// This version has a lower position and a smoother, multi-point curve.
void drawNeck()
{
	glColor3f(1.0f, 0.84f, 0.0f); // Golden yellow, same as body

	// This multi-point profile creates a fully curved shape.
	// The Y-values have been lowered to connect with the collar (from 0.95f down to 0.88f).
	float neck_profile[][2] = {
		{0.17f, 0.88f},  // Point 1: Base connection to the collar
		{0.14f, 0.93f},  // Point 2: Start of the inward curve
		{0.11f, 0.98f},  // Point 3: Thinnest part of the neck
		{0.115f, 1.03f}, // Point 4: Start of the outward curve to the head
		{0.12f, 1.08f}   // Point 5: Top connection to the head
	};
	int neck_points = sizeof(neck_profile) / sizeof(neck_profile[0]);

	// Use the existing lathed object function to draw the new curved neck
	drawLathedObject(neck_profile, neck_points, 20);
}

// ----------------------------
// Head Decoration (halo + wings + crown spike)
// ----------------------------
void drawHeadDeco()
{
	glPushMatrix();
	glTranslatef(0.0f, 1.20f, 0.0f);

	// Halo arc (line strip)
	glColor3f(1.0f, 0.84f, 0.0f);
	float radius = 0.6f;
	glBegin(GL_LINE_STRIP);
	for (int i = 0; i <= 180; i += 5)
	{
		float angle = i * 3.14159f / 180.0f;
		float x = radius * cos(angle);
		float y = radius * sin(angle);
		glVertex3f(x, y, 0.0f);
	}
	glEnd();

	// Left wing
	glPushMatrix();
	glTranslatef(-0.6f, 0.2f, 0.0f);
	glRotatef(20, 0, 0, 1);
	glScalef(0.4f, 0.15f, 0.05f);
	drawCuboid(1, 1, 0.2f);
	glPopMatrix();

	// Right wing
	glPushMatrix();
	glTranslatef(0.6f, 0.2f, 0.0f);
	glRotatef(-20, 0, 0, 1);
	glScalef(0.4f, 0.15f, 0.05f);
	drawCuboid(1, 1, 0.2f);
	glPopMatrix();

	// Crown spike
	glPushMatrix();
	glTranslatef(0.0f, 0.6f, 0.0f);
	glRotatef(-90, 1, 0, 0);
	glColor3f(1.0f, 0.84f, 0.0f);
	drawCone(0.4f, 1.0f, 32, 32);
	glPopMatrix();

	glPopMatrix();
}

// --- UPDATED function to draw the helmet piece ---
// This now draws the triangle upside down.
void drawHelmetVisor()
{
	glPushMatrix();

	// Set up shiny material for the visor
	GLfloat mat_ambient[] = { 0.8f, 0.7f, 0.1f, 1.0f };
	GLfloat mat_diffuse[] = { 1.0f, 0.84f, 0.0f, 1.0f };
	GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat mat_shininess[] = { 128.0f };

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

	// Define the triangle's vertices to control its size and shape.
	float base_width = 0.15f;
	float height = 0.55f;

	glBegin(GL_TRIANGLES);
	glNormal3f(0.0f, 0.0f, 1.0f);
	// MODIFIED: Vertices for an upside-down (downward-pointing) triangle
	glVertex3f(-base_width, 0.0f, 0.0f);   // Top-left vertex
	glVertex3f(base_width, 0.0f, 0.0f);    // Top-right vertex
	glVertex3f(0.0f, -height, 0.0f);       // Bottom-center tip
	glEnd();

	glPopMatrix();
}

// --- UPDATED function to draw the dynamic halo ---
void drawHalo()
{
	glPushMatrix();

	// MODIFIED: Changed Z-translation to -1.2f to make the gap 4 times larger.
	// Original was -0.3f, 4 * -0.3f = -1.2f.
	glTranslatef(0.0f, 1.5f, -0.5f);
	glScalef(0.38f, 0.38f, 0.38f);

	// --- 1. Draw the Gradient Gold Rings ---
	glDisable(GL_LIGHTING);
	glLineWidth(3.5f);
	for (int j = 0; j < 2; j++) {
		float radius = 1.0f + (j * 0.2f);
		glBegin(GL_LINE_LOOP);
		for (int i = 0; i <= 60; ++i) {
			float angle = (float)i / 60.0f * 2.0f * 3.14159f;
			float brightness = 0.7f + 0.3f * sin(angle * 4.0f + g_rainbow_offset);
			glColor3f(1.0f * brightness, 0.84f * brightness, 0.1f * brightness);
			glVertex3f(radius * cos(angle), radius * sin(angle), 0.0f);
		}
		glEnd();
	}
	glLineWidth(1.0f);

	// --- 2. Draw the Dazzling Rainbow Glow Effect ---
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	float inner_radius = 1.2f;
	float outer_radius = 4.0f;

	glBegin(GL_QUAD_STRIP);
	for (int i = 0; i <= 60; i++) {
		float angle = (float)i / 60.0f * 2.0f * 3.14159f;
		float cos_a = cos(angle);
		float sin_a = sin(angle);

		float r = 0.5f * (1.0f + sin(g_rainbow_offset * 2.0f));
		float g = 0.5f * (1.0f + sin(g_rainbow_offset * 2.0f + 2.0f));
		float b = 0.5f * (1.0f + sin(g_rainbow_offset * 2.0f + 4.0f));

		glColor4f(r, g, b, 0.35f);
		glVertex3f(inner_radius * cos_a, inner_radius * sin_a, 0.0f);

		glColor4f(r, g, b, 0.0f);
		glVertex3f(outer_radius * cos_a, outer_radius * sin_a, 0.0f);
	}
	glEnd();

	glDisable(GL_BLEND);

	glPopMatrix();
	glEnable(GL_LIGHTING);
}

void drawBackSashes() {
	const float PI = 3.14159f;
	GLUquadric* quad = gluNewQuadric();

	// --- Parameters for cloth shape and position ---
	float base_sash_width = 0.18f;
	float sash_length = 2.5f;
	float flare_factor = 1.2f;
	int   segments = 30;

	// Anchor point on the belt
	float belt_top_back_y = -0.05f;
	float belt_back_radius = 0.18f;
	float start_x_offset = base_sash_width / 2.0f;

	// --- Key values for the new, correct behaviour ---
	// Increased downward tilt to make the cloth kick out more aggressively.
	const float TILT_DOWNWARD_X = 25.0f; // Increased from 15.0f
	// The angle for flaring the sashes out to the sides
	const float FLARE_SIDEWAYS_Y = 35.0f;
	// A tiny physical gap to ensure the cloth renders on the outside
	const float SURFACE_OFFSET = 0.02f;

	// A reusable function to draw one sash
	auto drawOneSash = [&](bool isLeftSash) {
		glPushMatrix();
		// 1. ANCHOR DIRECTLY ON THE BODY'S SURFACE at the belt line.
		glTranslatef(isLeftSash ? -start_x_offset : +start_x_offset,
			belt_top_back_y,
			-belt_back_radius);

		// 2. ROTATE to flare sideways and then tilt down and away from the body.
		glRotatef(isLeftSash ? +FLARE_SIDEWAYS_Y : -FLARE_SIDEWAYS_Y, 0.0f, 1.0f, 0.0f);
		glRotatef(TILT_DOWNWARD_X, 1.0f, 0.0f, 0.0f);

		// Set cloth and bead colours
		glColor3f(0.9f, 0.5f, 0.0f);

		// 3. DRAW THE CLOTH with the physical offset and new curve.
		glBegin(GL_QUAD_STRIP);
		for (int i = 0; i <= segments; ++i) {
			float t = (float)i / segments;
			float y = -t * sash_length;

			// NEW Z-CURVE CALCULATION:
			// This makes the sash project outwards more strongly at the beginning (small 't'),
			// then curve backwards more gently as it goes down.
			// Using a power function (e.g., t^0.5 or 1-cos(t*PI/2)) can give different curve feels.
			// Let's try a softer, more exponential outward curve.
			float z = SURFACE_OFFSET + (-0.5f * pow(t, 2.0f)); // Starts at offset, then curves back gently. Adjust 0.5f to control curvature.

			float current_width = base_sash_width + t * flare_factor;
			float x_offset = sin(t * PI) * (isLeftSash ? -0.2f : 0.2f);

			glNormal3f(isLeftSash ? 0.45f : -0.45f, 0.5f, -0.75f);

			glVertex3f(x_offset - current_width * 0.5f, y, z);
			glVertex3f(x_offset + current_width * 0.5f, y, z);
		}
		glEnd();

		// Draw the beads
		glColor3f(0.9f, 0.7f, 0.1f);
		for (int i = 1; i <= 5; ++i) {
			glPushMatrix();
			float t = i * 0.2f;
			float y = -sash_length * t;
			// Use the same new Z-curve for beads
			float z = SURFACE_OFFSET + (-0.5f * pow(t, 2.0f));

			float current_width = base_sash_width + t * flare_factor;
			float x_offset = sin(t * PI) * (isLeftSash ? -0.2f : 0.2f);
			float sphere_radius = 0.06f;

			float bead_x = x_offset + (isLeftSash ? +current_width * 0.5f - 0.04f : -current_width * 0.5f + 0.04f);
			glTranslatef(bead_x, y, z + 0.02f);
			gluSphere(quad, sphere_radius, 12, 12);
			glPopMatrix();
		}


// --- UPDATED display function ---
void display(float deltaTime)
{
	// Increment the global offset each frame to animate the halo colours
	float animation_speed = 0.1f;
	g_rainbow_offset += animation_speed * deltaTime;

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

	glTranslatef(0.0f, -0.5f, zoomFactor);

	glRotatef(rotateX, 1.0f, 0.0f, 0.0f);
	glRotatef(rotateY, 0.0f, 1.0f, 0.0f);

	// --- Drawing Calls ---
	drawSmoothChest();
	drawWaistWithVerticalLines();
	drawSmoothLowerBodyAndSkirt();

	// Use glPolygonOffset to draw the belt on top of the skirt
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(-1.0f, -1.0f);
	drawWaistBelt();
	glDisable(GL_POLYGON_OFFSET_FILL);

	drawArmorCollar();
	drawSmoothArms();
	drawShoulderPads3D();

	glPushMatrix();
	drawNeck();
	drawFace();
	// Draw the halo after the face so it's positioned relative to the final head location
	drawHalo();
	glPopMatrix();
}


//--------------------------------------------------------------------

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow)
{
	// --- Register Window Class ---
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hInstance = hInst;
	wc.lpfnWndProc = WindowProcedure;
	wc.lpszClassName = WINDOW_TITLE;
	wc.style = CS_HREDRAW | CS_VREDRAW;

	if (!RegisterClassEx(&wc)) return -1;

	// --- Create Window ---
	HWND hWnd = CreateWindow(
		WINDOW_TITLE, WINDOW_TITLE,
		WS_OVERLAPPEDWINDOW, // <-- CORRECTED TYPO HERE
		CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
		NULL, NULL, hInst, NULL
	);

	if (!hWnd) return -1;

	// --- Device Context (global) ---
	g_hDC = GetDC(hWnd);
	if (!g_hDC) return -1;

	// --- Set Pixel Format ---
	initPixelFormat(g_hDC);

	// --- Rendering Context (global) ---
	g_hRC = wglCreateContext(g_hDC);
	if (!g_hRC) return -1;

	if (!wglMakeCurrent(g_hDC, g_hRC)) return -1;

	ShowWindow(hWnd, nCmdShow);

	// --- Initialize the high-precision timer for delta time ---
	QueryPerformanceFrequency(&g_timer_frequency);
	QueryPerformanceCounter(&g_last_frame_time);

	// --- Message Loop ---
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

		// --- Calculate Delta Time to ensure constant animation speed ---
		LARGE_INTEGER current_frame_time;
		QueryPerformanceCounter(&current_frame_time);
		float deltaTime = (float)(current_frame_time.QuadPart - g_last_frame_time.QuadPart) / g_timer_frequency.QuadPart;
		g_last_frame_time = current_frame_time;

		// --- Render ---
		display(deltaTime); // Pass delta time to the display function
		SwapBuffers(g_hDC);
	}

	// --- Cleanup ---
	wglMakeCurrent(NULL, NULL);
	if (g_hRC) wglDeleteContext(g_hRC);
	if (g_hDC) ReleaseDC(hWnd, g_hDC);
	DestroyWindow(hWnd);
	UnregisterClass(WINDOW_TITLE, hInst);

	return static_cast<int>(msg.wParam);
}