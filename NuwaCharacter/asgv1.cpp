#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <math.h>
#include <vector>
#include <stdio.h>
#include <time.h> 

#pragma comment (lib, "OpenGL32.lib")
#pragma comment (lib, "GLU32.lib")

#define WINDOW_TITLE "OpenGL Window"

struct Particle {
	float x, y, z;          // Position
	float vx, vy, vz;       // Velocity
	float r, g, b, a;       // Color (with alpha for fading)
	float life;             // Current life (0.0 - 1.0)
	float fade;             // How fast it fades
	float size;             // Size of the particle
	float initialSize;      // Store the starting size
};

std::vector<Particle> g_particles; // Our particle array
const int MAX_PARTICLES = 1000;    // Max particles at any time
bool g_isFiringEars = false;      // True if 'B' is pressed and ears should emit
float g_lastParticleEmitTime = 0.0f; // To control emission rate
float g_emitInterval = 0.05f;

// --- NEW: Animation State Variables ---
bool g_isHaloAnimating = false;
bool g_isHaloVisible = true;
float g_haloZ = -0.5f;
float g_haloScale = 0.38f;

// Store the light's initial and current animated position
GLfloat g_initialLightPos[4] = { 5.0f, 5.0f, 5.0f, 1.0f };
GLfloat g_animatedLightPos[4];

GLuint g_fireTextureID = 0;
GLuint g_goldTextureID = 0;
GLuint g_redTextureID = 0;

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

void resetAnimation() {
	g_isHaloAnimating = false;
	g_isHaloVisible = true;
	g_haloZ = -0.5f;
	g_haloScale = 0.38f;

	// Reset the light's position
	memcpy(g_animatedLightPos, g_initialLightPos, sizeof(GLfloat) * 4);

	// --- NEW: Reset particle system state ---
	g_isFiringEars = false;
	g_particles.clear();
}

LRESULT WINAPI WindowProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
		// --- Handle all keyboard input ---

		// Exit the application
		if (wParam == VK_ESCAPE) PostQuitMessage(0);

		// Start the halo animation
		if (wParam == 'A') {
			g_isHaloAnimating = true;
		}

		// --- THIS WAS MISSING ---
		// Toggle the ear fire particle effect
		if (wParam == 'B') {
			g_isFiringEars = !g_isFiringEars;
		}

		// Reset all animations
		if (wParam == VK_SPACE) {
			resetAnimation();
		}
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

GLuint loadTextureBMP(const char* imagepath) {
	printf("Reading image %s\n", imagepath);

	unsigned char header[54];
	unsigned int dataPos;
	unsigned int width, height;
	unsigned int imageSize;
	unsigned char* data;

	FILE* file;
	fopen_s(&file, imagepath, "rb");
	if (!file) {
		printf("Image could not be opened\n");
		return 0;
	}

	if (fread(header, 1, 54, file) != 54) {
		printf("Not a correct BMP file\n");
		fclose(file); // Close the file on error
		return 0;
	}
	if (header[0] != 'B' || header[1] != 'M') {
		printf("Not a correct BMP file\n");
		fclose(file); // Close the file on error
		return 0;
	}

	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	width = *(int*)&(header[0x12]);
	height = *(int*)&(header[0x16]);

	if (imageSize == 0)    imageSize = width * height * 3;
	if (dataPos == 0)      dataPos = 54;

	data = new unsigned char[imageSize];
	fread(data, 1, imageSize, file);
	fclose(file);

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, data);

	// THE FIX IS HERE: The delete[] data; line was moved from here...

	// Texture filtering and wrapping
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	// Generate Mipmaps AFTER creating the main texture and BEFORE deleting the data
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, width, height, GL_BGR_EXT, GL_UNSIGNED_BYTE, data);

	// ...to here. Now we can safely free our copy of the data.
	delete[] data;

	return textureID;
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

void drawCurvedShoulderPads()
{
	// Common material for the shoulder pads
	GLfloat mat_ambient[] = { 0.45f, 0.38f, 0.1f, 1.0f };
	GLfloat mat_diffuse[] = { 1.0f, 0.84f, 0.0f, 1.0f };
	GLfloat mat_specular[] = { 1.0f, 1.0f, 0.8f, 1.0f };
	GLfloat mat_shininess[] = { 100.0f };
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

	// Lambda function to draw one shoulder pad (left or right)
	auto drawOneShoulder = [&](bool isLeft) {
		glPushMatrix();

		// 1. Position the entire shoulder pad assembly relative to the body
		glTranslatef(isLeft ? -0.38f : 0.38f, 0.7f, 0.0f); // Adjust Y-pos to sit on the shoulder

		// 2. Initial rotation to angle the entire pauldron outwards and downwards
		glRotatef(isLeft ? 20.0f : -20.0f, 0.0f, 0.0f, 1.0f); // Angle outwards
		glRotatef(-25.0f, 1.0f, 0.0f, 0.0f);                  // Tilt downwards

		// Set color for the dark trim
		glColor3f(0.5f, 0.35f, 0.05f); // Darker gold

		// --- Bottom Layer (Largest, Darker Panel) ---
		glPushMatrix();
		glTranslatef(0.0f, 0.0f, -0.05f); // Slightly behind the main layer
		glScalef(1.2f, 0.8f, 0.1f);      // Wider and slightly thinner
		glRotatef(isLeft ? 10.0f : -10.0f, 0.0f, 1.0f, 0.0f); // Slight curve for this layer
		drawCuboid(0.4f, 0.2f, 1.0f);    // Base dimensions (width, height, depth)
		glPopMatrix();

		// Set color for the main gold panels
		glColor3f(1.0f, 0.84f, 0.0f); // Bright gold

		// --- Middle Layer (Main Panel) ---
		glPushMatrix();
		glTranslatef(0.0f, 0.02f, 0.0f); // Slightly above and forward from the bottom layer
		glScalef(1.0f, 0.9f, 0.12f);    // Main size
		glRotatef(isLeft ? -5.0f : 5.0f, 0.0f, 1.0f, 0.0f); // Slight curve for this layer
		drawCuboid(0.4f, 0.2f, 1.0f);
		glPopMatrix();

		// --- Top Layer (Smaller, More Angular Panel) ---
		glPushMatrix();
		glTranslatef(0.0f, 0.05f, 0.05f); // Even further up and forward
		glScalef(0.8f, 0.8f, 0.15f);    // Smaller, slightly thicker
		drawCuboid(0.4f, 0.2f, 1.0f);
		glPopMatrix();

		glPopMatrix(); // Restore matrix state for drawing the next shoulder
		};

	// Draw both shoulders
	drawOneShoulder(true);  // Left shoulder
	drawOneShoulder(false); // Right shoulder
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

			// --- REVERTED NORMALS ---
			float normal_x_profile = -dy; // Revert to -dy
			float normal_y_profile = dx;  // Revert to dx

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

			// Add texture coordinates
			float u = (float)j / sides;
			float v1 = (float)i / (num_points - 1);
			float v2 = (float)(i + 1) / (num_points - 1);

			glTexCoord2f(u, v1); // Texture coord for the first vertex
			glVertex3f(profile[i][0] * cos_angle, profile[i][1], profile[i][0] * sin_angle);
			glNormal3f(normal_x_3d, normal_y_profile, normal_z_3d);

			glTexCoord2f(u, v2); // Texture coord for the second vertex
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
	// CHANGED: Raised the shoulder height from 0.6f to 0.7f to lift the hand.
	glTranslatef(-0.6f, 0.7f, 0.0f); // Stays on the left side of the body

	// Step 1: SHOULDER: Using original Right Arm's rotation
	glRotatef(10.0f, 0.0f, 0.0f, 1.0f);

	drawLathedObject(upper_arm_profile, 2, 12);
	glTranslatef(0.0f, -0.5f, 0.0f);

	// Step 2: ELBOW: Using original Right Arm's rotation
	glRotatef(-25.0f, 1.0f, 0.0f, 0.0f);

	drawLathedObject(lower_arm_profile, 2, 12);
	glTranslatef(0.0f, -0.4f, 0.0f);

	// Step 3: WRIST: Using original Right Arm's rotation
	glRotatef(-70.0f, 0.0f, 1.0f, 0.0f);

	drawHand(false); // Using original Right Arm's hand model (isLeftHand = false)
	glPopMatrix();

	// --- Right Arm ---
	// This block now uses the rotations from the original LEFT arm.
	glPushMatrix();
	// CHANGED: Raised the shoulder height from 0.6f to 0.7f to lift the hand.
	glTranslatef(0.6f, 0.7f, 0.0f); // Stays on the right side of the body

	// Step 1: SHOULDER: Using original Left Arm's rotation
	glRotatef(-10.0f, 0.0f, 0.0f, 1.0f);

	drawLathedObject(upper_arm_profile, 2, 12);
	glTranslatef(0.0f, -0.5f, 0.0f);

	// Step 2: ELBOW: Using original Left Arm's rotation
	glRotatef(-25.0f, 1.0f, 0.0f, 0.0f);

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

void drawBackSashes()
{
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

	const float TILT_DOWNWARD_X = 25.0f;
	const float FLARE_SIDEWAYS_Y = 35.0f;
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

		// 将基础颜色设为白色，这样纹理就不会被其他颜色影响
		glColor3f(1.0f, 1.0f, 1.0f);

		// 3. DRAW THE CLOTH with the physical offset and new curve.

		// <--- 关键修改：启用并绑定红色纹理 --->
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, g_redTextureID);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		glBegin(GL_QUAD_STRIP);
		for (int i = 0; i <= segments; ++i) {
			float t = (float)i / segments;
			float y = -t * sash_length;
			float z = SURFACE_OFFSET + (-0.5f * pow(t, 2.0f));
			float current_width = base_sash_width + t * flare_factor;
			float x_offset = sin(t * PI) * (isLeftSash ? -0.2f : 0.2f);
			glNormal3f(isLeftSash ? 0.45f : -0.45f, 0.5f, -0.75f);

			// <--- 关键修改：为每个顶点添加纹理坐标 --->
			// U 坐标代表图片的横向（0.0是左边，1.0是右边）
			// V 坐标 (t) 代表图片的纵向，跟随饰带的长度变化
			glTexCoord2f(0.0f, t);
			glVertex3f(x_offset - current_width * 0.5f, y, z);

			glTexCoord2f(1.0f, t);
			glVertex3f(x_offset + current_width * 0.5f, y, z);
		}
		glEnd();

		// <--- 关键修改：绘制完后禁用纹理，以免影响珠子 --->
		glDisable(GL_TEXTURE_2D);


		// Draw the beads
		glColor3f(0.9f, 0.7f, 0.1f); // 把颜色改回金色来画珠子
		for (int i = 1; i <= 5; ++i) {
			glPushMatrix();
			float t = i * 0.2f;
			float y = -sash_length * t;
			float z = SURFACE_OFFSET + (-0.5f * pow(t, 2.0f));
			float current_width = base_sash_width + t * flare_factor;
			float x_offset = sin(t * PI) * (isLeftSash ? -0.2f : 0.2f);
			float sphere_radius = 0.06f;
			float bead_x = x_offset + (isLeftSash ? +current_width * 0.5f - 0.04f : -current_width * 0.5f + 0.04f);
			glTranslatef(bead_x, y, z + 0.02f);
			gluSphere(quad, sphere_radius, 12, 12);
			glPopMatrix();
		}
		glPopMatrix();
		};

	drawOneSash(true);   // Draw the left sash
	drawOneSash(false);  // Draw the right sash

	gluDeleteQuadric(quad);
}

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

void drawCone(float base, float height, int slices, int stacks)
{
	// THE FIX: The points are reversed to go from the tip to the base.
	// This makes the normal calculation in drawLathedObject work correctly for the cone.
	float profile[2][2] = {
		{0.0f, height}, // Tip of the cone (Top)
		{base, 0.0f}    // Base of the cone (Bottom)
	};
	drawLathedObject(profile, 2, slices);
}

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

	// --- NEW: Enable and apply the Gold texture ---
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, g_goldTextureID); // Bind the new gold texture
	// Use GL_MODULATE to combine the gold texture with the material lighting
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// Define the triangle's vertices to control its size and shape.
	float base_width = 0.35f;
	float height = 0.55f;

	glBegin(GL_TRIANGLES);
	glNormal3f(0.0f, 0.0f, 1.0f); // Normal for the front face

	// MODIFIED: Vertices for an upside-down (downward-pointing) triangle with Texture Coordinates
	glTexCoord2f(0.0f, 1.0f); // Top-left of texture maps to top-left of triangle
	glVertex3f(-base_width, 0.0f, 0.0f);

	glTexCoord2f(1.0f, 1.0f); // Top-right of texture maps to top-right of triangle
	glVertex3f(base_width, 0.0f, 0.0f);

	glTexCoord2f(0.5f, 0.0f); // Middle-bottom of texture maps to bottom tip of triangle
	glVertex3f(0.0f, -height, 0.0f);
	glEnd();

	// --- NEW: Disable texturing after drawing the visor ---
	glDisable(GL_TEXTURE_2D);

	glPopMatrix();
}

void drawHalo()
{
	if (!g_isHaloVisible) {
		return; // Don't draw if it's out of range
	}

	glPushMatrix();

	// MODIFIED: Changed Z-translation to -1.2f to make the gap 4 times larger.
	glTranslatef(0.0f, 1.5f, g_haloZ);
	glScalef(g_haloScale, g_haloScale, g_haloScale);

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

void updateParticles(float deltaTime)
{
	// Emit new particles if firing
	if (g_isFiringEars) {
		if (g_lastParticleEmitTime >= g_emitInterval) {

			// --- CORRECTED: Define ear tip position in LOCAL space ---
			// This is the position relative to the character's center pivot
			float earTipY = 1.18f + 0.2f; // Head's Y-offset + Ear's Y-offset
			float earTipX = 0.3f + 0.6f;  // Ear's X-offset + Ear's length

			// --- Emit from Right Ear ---
			if (g_particles.size() < MAX_PARTICLES) {
				Particle p;
				// Position is in simple, un-rotated local space
				p.x = earTipX;
				p.y = earTipY;
				p.z = 0.0f;

				// Velocity is in simple, un-rotated local space (points away from the ear)
				p.vx = 2.0f + ((float)rand() / RAND_MAX);
				p.vy = (float)rand() / RAND_MAX * 0.5f - 0.25f;
				p.vz = (float)rand() / RAND_MAX * 0.5f - 0.25f;

				// Set particle properties
				int colorType = rand() % 2;
				if (colorType == 0) { p.r = 1.0f; p.g = 1.0f; p.b = (float)rand() / RAND_MAX * 0.5f; }
				else { p.r = 1.0f; p.g = 0.5f + ((float)rand() / RAND_MAX * 0.2f); p.b = 0.0f; }
				p.life = 1.0f;
				p.fade = (float)rand() / RAND_MAX * 0.7f + 1.0f;
				p.size = (float)rand() / RAND_MAX * 0.15f + 0.1f;
				p.initialSize = p.size;
				p.a = 0.7f + ((float)rand() / RAND_MAX * 0.3f);
				g_particles.push_back(p);
			}

			// --- Emit from Left Ear ---
			if (g_particles.size() < MAX_PARTICLES) {
				Particle p;
				// Position is in simple, un-rotated local space
				p.x = -earTipX;
				p.y = earTipY;
				p.z = 0.0f;

				// Velocity is in simple, un-rotated local space
				p.vx = -2.0f - ((float)rand() / RAND_MAX);
				p.vy = (float)rand() / RAND_MAX * 0.5f - 0.25f;
				p.vz = (float)rand() / RAND_MAX * 0.5f - 0.25f;

				// Set particle properties
				int colorType = rand() % 2;
				if (colorType == 0) { p.r = 1.0f; p.g = 1.0f; p.b = (float)rand() / RAND_MAX * 0.5f; }
				else { p.r = 1.0f; p.g = 0.5f + ((float)rand() / RAND_MAX * 0.2f); p.b = 0.0f; }
				p.life = 1.0f;
				p.fade = (float)rand() / RAND_MAX * 0.7f + 1.0f;
				p.size = (float)rand() / RAND_MAX * 0.15f + 0.1f;
				p.initialSize = p.size;
				p.a = 0.7f + ((float)rand() / RAND_MAX * 0.3f);
				g_particles.push_back(p);
			}
			g_lastParticleEmitTime = 0.0f;
		}
		else {
			g_lastParticleEmitTime += deltaTime;
		}
	}

	// Update existing particles
	for (auto it = g_particles.begin(); it != g_particles.end(); ) {
		// Note: The "wind" effect here is now relative to the character's orientation
		it->vx += 0.5f * deltaTime;
		it->vy += 0.8f * deltaTime;
		it->x += it->vx * deltaTime;
		it->y += it->vy * deltaTime;
		it->z += it->vz * deltaTime;
		it->life -= it->fade * deltaTime;
		it->a = it->life;
		if (it->life <= 0.0f) {
			it = g_particles.erase(it);
		}
		else {
			++it;
		}
	}
}

void drawParticles() {
	if (g_particles.empty()) return;

	// Save OpenGL state
	glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT | GL_POINT_BIT | GL_DEPTH_BUFFER_BIT);

	glDisable(GL_LIGHTING); // Particles are self-illuminated
	glEnable(GL_BLEND);     // Enable transparency
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal alpha blending
	glDepthMask(GL_FALSE);  // Allow particles to render correctly without depth issues

	// --- NEW: Make points soft and circular instead of square ---
	glEnable(GL_POINT_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

	// --- NEW: Begin drawing points, not quads ---
	glBegin(GL_POINTS);
	for (const auto& p : g_particles) {
		// Set the colour and transparency for this particle
		glColor4f(p.r, p.g, p.b, p.a);

		// Calculate the current size for the pulsing effect
		float currentSize = p.initialSize * sin(p.life * 3.14159f);

		// Set the size of the point in pixels
		// This is the main number to adjust for the flame's visual size
		glPointSize(currentSize * 60.0f);

		// Draw the single point for the particle
		glVertex3f(p.x, p.y, p.z);
	}
	glEnd();

	// Restore the original OpenGL state
	glPopAttrib();
}

void drawEars()
{
	glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT);

	GLfloat bright_ear_ambient[] = { 0.6f, 0.5f, 0.2f, 1.0f };
	GLfloat bright_ear_diffuse[] = { 1.0f, 0.9f, 0.3f, 1.0f };
	GLfloat bright_ear_specular[] = { 1.0f, 1.0f, 0.8f, 1.0f };
	GLfloat bright_ear_shininess[] = { 100.0f };

	glMaterialfv(GL_FRONT, GL_AMBIENT, bright_ear_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, bright_ear_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, bright_ear_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, bright_ear_shininess);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, g_fireTextureID);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	float ear_base_radius = 0.15f;
	float ear_height = 0.6f;

	// --- Right Ear ---
	glPushMatrix();
	// CORRECTED POSITIONING
	glTranslatef(0.3f, 0.2f, 0.0f);
	glRotatef(-90.0f, 0.0f, 0.0f, 1.0f);
	drawCone(ear_base_radius, ear_height, 16, 16);
	glPopMatrix();

	// --- Left Ear ---
	glPushMatrix();
	// CORRECTED POSITIONING
	glTranslatef(-0.3f, 0.2f, 0.0f);
	glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
	drawCone(ear_base_radius, ear_height, 16, 16);
	glPopMatrix();

	glDisable(GL_TEXTURE_2D);
	glPopAttrib();
}

void drawFace()
{
	// Apply the same golden material to the head and ears
	GLfloat mat_ambient[] = { 0.55f, 0.38f, 0.1f, 1.0f };
	GLfloat mat_diffuse[] = { 1.0f, 0.84f, 0.0f, 1.0f };
	GLfloat mat_specular[] = { 1.0f, 1.0f, 0.8f, 1.0f };
	GLfloat mat_shininess[] = { 100.0f };

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

	glPushMatrix(); // Start of the entire head group
	glTranslatef(0.0f, 1.18f, 0.0f);

	// --- Add the Helmet Visor ---
	glPushMatrix();
	glTranslatef(0.0f, 0.28f, 0.28f);
	drawHelmetVisor();
	glPopMatrix();

	// --- NEW: Draw the Ears ---
	drawEars(); // <--- ADD THIS LINE

	// --- Main Head Shape ---
	int latitudes = 15;
	int longitudes = 20;
	float head_height = 0.5f;
	float head_max_radius = 0.28f;
	float head_base_radius = 0.10f;

	for (int i = 0; i <= latitudes; ++i) {
		float lat0 = 3.14159f * (-0.5f + (float)(i - 1) / latitudes);
		float lat1 = 3.14159f * (-0.5f + (float)i / latitudes);
		float y0 = sin(lat0) * head_height / 2.0f;
		float y1 = sin(lat1) * head_height / 2.0f;
		float r0 = cos(lat0) * head_max_radius;
		float r1 = cos(lat1) * head_max_radius;
		if (i == 0) r0 = head_base_radius;
		if (i == 1) r0 = head_base_radius;
		glBegin(GL_QUAD_STRIP);
		for (int j = 0; j <= longitudes; ++j) {
			float lng = 2.0f * 3.14159f * (float)j / longitudes;
			float x = cos(lng);
			float z = sin(lng);
			float nx0 = cos(lat0) * x, ny0 = sin(lat0), nz0 = cos(lat0) * z;
			glNormal3f(nx0, ny0, nz0);
			glVertex3f(r0 * x, y0, r0 * z);
			float nx1 = cos(lat1) * x, ny1 = sin(lat1), nz1 = cos(lat1) * z;
			glNormal3f(nx1, ny1, nz1);
			glVertex3f(r1 * x, y1, r1 * z);
		}
		glEnd();
	}
	glPopMatrix(); // End of the entire head group
}

void drawDiamondKneeJoint()
{
	glPushMatrix();
	// NOTE: This function inherits the gold material from drawLegs()

	// --- NEW: Enable and apply the fire texture ---
	glEnable(GL_TEXTURE_2D);
	// We use g_fireTextureID because it already has Fire.bmp loaded
	glBindTexture(GL_TEXTURE_2D, g_fireTextureID);
	// This blends the fire texture with the existing gold material and lighting
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// Y-scale (height) is kept long to allow for overlap
	glScalef(0.2f, 0.45f, 0.2f); // X, Y (height), Z scale

	// This rotation orients the diamond correctly
	glRotatef(45.0f, 0.0f, 1.0f, 0.0f);

	// Define the 6 points of our diamond (an octahedron)
	float p[6][3] = {
		{ 0.0f,  1.0f,  0.0f}, // Top
		{ 0.0f, -1.0f,  0.0f}, // Bottom
		{ 1.0f,  0.0f,  0.0f}, // Right
		{-1.0f,  0.0f,  0.0f}, // Left
		{ 0.0f,  0.0f,  1.0f}, // Front
		{ 0.0f,  0.0f, -1.0f}  // Back
	};

	// --- NEW: Define texture coordinates for the 6 points ---
	// This maps points on the 2D fire image to the 3D diamond points.
	float t[6][2] = {
		{0.5f, 1.0f}, // Top-middle of texture
		{0.5f, 0.0f}, // Bottom-middle of texture
		{1.0f, 0.5f}, // Middle-right of texture
		{0.0f, 0.5f}, // Middle-left of texture
		{0.5f, 0.5f}, // Center of texture
		{0.5f, 0.5f}  // Center of texture
	};

	// An array to hold the calculated normals for each of the 8 faces
	float n[8][3] = {
		{ 0.707f,  0.707f,  0.707f}, { -0.707f,  0.707f,  0.707f},
		{ -0.707f,  0.707f, -0.707f}, {  0.707f,  0.707f, -0.707f},
		{ 0.707f, -0.707f,  0.707f}, { -0.707f, -0.707f,  0.707f},
		{ -0.707f, -0.707f, -0.707f}, {  0.707f, -0.707f, -0.707f}
	};

	glBegin(GL_TRIANGLES);
	// Top-Front-Right face
	glNormal3fv(n[0]);
	glTexCoord2fv(t[0]); glVertex3fv(p[0]);
	glTexCoord2fv(t[4]); glVertex3fv(p[4]);
	glTexCoord2fv(t[2]); glVertex3fv(p[2]);
	// Top-Front-Left face
	glNormal3fv(n[1]);
	glTexCoord2fv(t[0]); glVertex3fv(p[0]);
	glTexCoord2fv(t[3]); glVertex3fv(p[3]);
	glTexCoord2fv(t[4]); glVertex3fv(p[4]);
	// Top-Back-Left face
	glNormal3fv(n[2]);
	glTexCoord2fv(t[0]); glVertex3fv(p[0]);
	glTexCoord2fv(t[5]); glVertex3fv(p[5]);
	glTexCoord2fv(t[3]); glVertex3fv(p[3]);
	// Top-Back-Right face
	glNormal3fv(n[3]);
	glTexCoord2fv(t[0]); glVertex3fv(p[0]);
	glTexCoord2fv(t[2]); glVertex3fv(p[2]);
	glTexCoord2fv(t[5]); glVertex3fv(p[5]);

	// Bottom-Front-Right face
	glNormal3fv(n[4]);
	glTexCoord2fv(t[1]); glVertex3fv(p[1]);
	glTexCoord2fv(t[2]); glVertex3fv(p[2]);
	glTexCoord2fv(t[4]); glVertex3fv(p[4]);
	// Bottom-Front-Left face
	glNormal3fv(n[5]);
	glTexCoord2fv(t[1]); glVertex3fv(p[1]);
	glTexCoord2fv(t[4]); glVertex3fv(p[4]);
	glTexCoord2fv(t[3]); glVertex3fv(p[3]);
	// Bottom-Back-Left face
	glNormal3fv(n[6]);
	glTexCoord2fv(t[1]); glVertex3fv(p[1]);
	glTexCoord2fv(t[3]); glVertex3fv(p[3]);
	glTexCoord2fv(t[5]); glVertex3fv(p[5]);
	// Bottom-Back-Right face
	glNormal3fv(n[7]);
	glTexCoord2fv(t[1]); glVertex3fv(p[1]);
	glTexCoord2fv(t[5]); glVertex3fv(p[5]);
	glTexCoord2fv(t[2]); glVertex3fv(p[2]);
	glEnd();

	// --- NEW: Disable texturing so it doesn't affect other objects ---
	glDisable(GL_TEXTURE_2D);

	glPopMatrix();
}

void drawLegs()
{
	// --- Material Properties for Golden Armour (for ALL leg parts) ---
	GLfloat mat_ambient[] = { 0.45f, 0.38f, 0.1f, 1.0f };
	GLfloat mat_diffuse[] = { 1.0f, 0.84f, 0.0f, 1.0f };
	GLfloat mat_specular[] = { 1.0f, 1.0f, 0.8f, 1.0f };
	GLfloat mat_shininess[] = { 100.0f };

	// Apply the material properties for ALL leg parts
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

	auto drawOneLeg = []() {
		glPushMatrix(); // Save the state at the hip joint

		// --- Part 1: Upper Leg (Thigh) ---
		float thigh_height = 0.8f;
		glPushMatrix();
		glTranslatef(0.0f, -thigh_height / 2.0f, 0.0f);
		{
			float v[8][3] = {
				{-0.15f,  thigh_height / 2.0f,  0.12f}, { 0.15f,  thigh_height / 2.0f,  0.12f},
				{ 0.15f,  thigh_height / 2.0f, -0.12f}, {-0.15f,  thigh_height / 2.0f, -0.12f},
				{-0.10f, -thigh_height / 2.0f,  0.10f}, { 0.10f, -thigh_height / 2.0f,  0.10f},
				{ 0.10f, -thigh_height / 2.0f, -0.10f}, {-0.10f, -thigh_height / 2.0f, -0.10f}
			};
			glBegin(GL_QUADS);
			glNormal3f(0.0, 0.0, 1.0); glVertex3fv(v[0]); glVertex3fv(v[1]); glVertex3fv(v[5]); glVertex3fv(v[4]);
			glNormal3f(0.0, 0.0, -1.0); glVertex3fv(v[3]); glVertex3fv(v[2]); glVertex3fv(v[6]); glVertex3fv(v[7]);
			glNormal3f(-1.0, 0.0, 0.0); glVertex3fv(v[0]); glVertex3fv(v[3]); glVertex3fv(v[7]); glVertex3fv(v[4]);
			glNormal3f(1.0, 0.0, 0.0); glVertex3fv(v[1]); glVertex3fv(v[2]); glVertex3fv(v[6]); glVertex3fv(v[5]);
			glEnd();
		}
		glPopMatrix();

		// --- Position for the Knee Joint ---
		glTranslatef(0.0f, -thigh_height, 0.0f);
		glRotatef(-5.0f, 1.0f, 0.0f, 0.0f);

		// --- Draw the Diamond Knee Joint ---
		drawDiamondKneeJoint();

		// --- Part 2: Lower Leg (Shin) ---
		// MODIFIED: Reduced the downward translation to create an overlap with the knee.
		glTranslatef(0.0f, -0.22f, 0.0f);

		float shin_height = 0.7f;
		glPushMatrix();
		glTranslatef(0.0f, -shin_height / 2.0f, 0.0f);
		{
			float v[8][3] = {
				{-0.10f,  shin_height / 2.0f,  0.10f}, { 0.10f,  shin_height / 2.0f,  0.10f},
				{ 0.10f,  shin_height / 2.0f, -0.10f}, {-0.10f,  shin_height / 2.0f, -0.10f},
				{-0.14f, -shin_height / 2.0f,  0.14f}, { 0.14f, -shin_height / 2.0f,  0.14f},
				{ 0.14f, -shin_height / 2.0f, -0.14f}, {-0.14f, -shin_height / 2.0f, -0.14f}
			};
			glBegin(GL_QUADS);
			glNormal3f(0.0, 0.0, 1.0); glVertex3fv(v[0]); glVertex3fv(v[1]); glVertex3fv(v[5]); glVertex3fv(v[4]);
			glNormal3f(0.0, 0.0, -1.0); glVertex3fv(v[3]); glVertex3fv(v[2]); glVertex3fv(v[6]); glVertex3fv(v[7]);
			glNormal3f(-1.0, 0.0, 0.0); glVertex3fv(v[0]); glVertex3fv(v[3]); glVertex3fv(v[7]); glVertex3fv(v[4]);
			glNormal3f(1.0, 0.0, 0.0); glVertex3fv(v[1]); glVertex3fv(v[2]); glVertex3fv(v[6]); glVertex3fv(v[5]);
			glEnd();
		}
		glPopMatrix();

		// --- Part 3: Foot ---
		glTranslatef(0.0f, -shin_height, 0.0f);
		glRotatef(5.0f, 1.0f, 0.0f, 0.0f);

		glPushMatrix();
		{
			float v[10][3] = {
				{-0.12f, 0.0f,  0.14f}, {0.12f, 0.0f,  0.14f},
				{0.12f, 0.0f, -0.25f}, {-0.12f, 0.0f, -0.25f},
				{-0.12f, -0.15f,  0.14f}, {0.12f, -0.15f,  0.14f},
				{0.12f, -0.15f, -0.25f}, {-0.12f, -0.15f, -0.25f},
				{0.0f, -0.15f, 0.4f},
				{0.0f, -0.15f, -0.4f}
			};

			glBegin(GL_QUADS);
			glNormal3f(0.0, 1.0, 0.0); glVertex3fv(v[0]); glVertex3fv(v[1]); glVertex3fv(v[2]); glVertex3fv(v[3]);
			glNormal3f(0.0, -1.0, 0.0); glVertex3fv(v[4]); glVertex3fv(v[7]); glVertex3fv(v[6]); glVertex3fv(v[5]);
			glNormal3f(0.0, 0.0, -1.0); glVertex3fv(v[3]); glVertex3fv(v[2]); glVertex3fv(v[6]); glVertex3fv(v[7]);
			glNormal3f(-1.0, 0.0, 0.0); glVertex3fv(v[0]); glVertex3fv(v[3]); glVertex3fv(v[7]); glVertex3fv(v[4]);
			glNormal3f(1.0, 0.0, 0.0); glVertex3fv(v[1]); glVertex3fv(v[2]); glVertex3fv(v[6]); glVertex3fv(v[5]);
			glEnd();

			glBegin(GL_TRIANGLES);
			glNormal3f(0.0, 0.0, 1.0); glVertex3fv(v[0]); glVertex3fv(v[1]); glVertex3f(0.0, 0.0, 0.25);
			glNormal3f(0.7, -0.3, 0.7); glVertex3fv(v[1]); glVertex3fv(v[8]); glVertex3fv(v[5]);
			glNormal3f(-0.7, -0.3, 0.7); glVertex3fv(v[0]); glVertex3fv(v[4]); glVertex3fv(v[8]);
			glNormal3f(0.7, -0.3, -0.7); glVertex3fv(v[2]); glVertex3fv(v[6]); glVertex3fv(v[9]);
			glNormal3f(-0.7, -0.3, -0.7); glVertex3fv(v[3]); glVertex3fv(v[9]); glVertex3fv(v[7]);
			glEnd();
		}
		glPopMatrix();

		glPopMatrix(); // Restore to the hip joint state
		};

	// --- Draw Left Leg ---
	glPushMatrix();
	glTranslatef(-0.18f, -1.0f, 0.0f);
	drawOneLeg();
	glPopMatrix();

	// --- Draw Right Leg ---
	glPushMatrix();
	glTranslatef(0.18f, -1.0f, 0.0f);
	drawOneLeg();
	glPopMatrix();
}

void display(float deltaTime)
{
	// --- NEW: Update particle system at the start of the frame ---
	updateParticles(deltaTime);

	// --- Halo Animation Logic Block ---
	if (g_isHaloAnimating) {
		// Define animation speeds
		const float HALO_MOVE_SPEED = 5.0f;   // How fast it moves forward
		const float HALO_SCALE_SPEED = 2.0f;  // How fast it grows (using your faster value)
		const float HALO_DISAPPEAR_Z = 4.0f;  // The Z-position where it disappears

		// Update position and scale based on delta time for smooth animation
		g_haloZ += HALO_MOVE_SPEED * deltaTime;
		g_haloScale += HALO_SCALE_SPEED * deltaTime;
		g_animatedLightPos[2] += HALO_MOVE_SPEED * deltaTime;

		// Check if the halo is out of range
		if (g_haloZ > HALO_DISAPPEAR_Z) {
			g_isHaloVisible = false;   // Make it invisible
			g_isHaloAnimating = false; // Stop the animation from updating further
		}
	}

	// Increment the global offset each frame to animate the halo colours
	float animation_speed = 0.09f;
	g_rainbow_offset += animation_speed * deltaTime;

	glClearColor(1.0, 1.0, 1.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	glShadeModel(GL_SMOOTH);

	glEnable(GL_NORMALIZE);

	// --- Main "Key Light" ---
	glLightfv(GL_LIGHT0, GL_POSITION, g_animatedLightPos);
	GLfloat ambient_light[] = { 0.5f, 0.5f, 0.5f, 1.0f }; // Using your brighter value
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient_light);

	// --- Second "Fill Light" ---
	glEnable(GL_LIGHT1);
	GLfloat light1_pos[] = { 0.0f, 2.0f, 5.0f, 1.0f };
	GLfloat light1_diffuse[] = { 0.4f, 0.4f, 0.4f, 1.0f };
	GLfloat light1_specular[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light1_specular);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, 800.0 / 600.0, 1.0, 100.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f, -0.5f, zoomFactor);

	glRotatef(rotateX, 1.0f, 0.0f, 0.0f);
	glRotatef(rotateY, 0.0f, 1.0f, 0.0f);

	// --- Drawing Calls for the Character ---
	drawSmoothChest();
	drawWaistWithVerticalLines();
	drawSmoothLowerBodyAndSkirt();
	drawLegs();

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(-1.0f, -1.0f);
	drawWaistBelt();
	glDisable(GL_POLYGON_OFFSET_FILL);

	drawArmorCollar();
	drawSmoothArms();
	drawCurvedShoulderPads();
	drawBackSashes();

	glPushMatrix();
	drawNeck();
	drawFace();
	drawHalo();
	glPopMatrix();

	// --- NEW: Draw particle system after the character ---
	drawParticles();

	// --- NEW: Swap buffers at the end of all drawing ---
	SwapBuffers(g_hDC);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow)
{
	// --- NEW: Initialise the random number generator ---
	// This is crucial for the particle system to look different each time.
	srand(time(NULL));

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
		WS_OVERLAPPEDWINDOW,
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

	// --- Load textures ---
	// CORRECTED: The global variable is named g_skyTextureID, not g_fireTextureID
	g_fireTextureID = loadTextureBMP("Textures/Fire.bmp");
	if (g_fireTextureID == 0) {
		MessageBox(hWnd, "Could not load Textures/Fire.bmp. Make sure the file is in the Textures folder.", "Texture Error", MB_OK | MB_ICONERROR);
		return -1; // Exit if the texture fails to load
	}

	g_goldTextureID = loadTextureBMP("Textures/Gold.bmp");
	if (g_goldTextureID == 0) {
		MessageBox(hWnd, "Could not load Textures/Gold.bmp. Make sure the file is in the Textures folder.", "Texture Error", MB_OK | MB_ICONERROR);
		return -1;
	}

	g_redTextureID = loadTextureBMP("Textures/Red.bmp");
	if (g_redTextureID == 0) {
		MessageBox(hWnd, "Could not load Textures/Red.bmp. Make sure the file is in the Textures folder.", "Texture Error", MB_OK | MB_ICONERROR);
		return -1;
	}

	// --- Set the initial animation state ---
	resetAnimation();

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
		display(deltaTime);
		// Note: SwapBuffers is now called inside display() in the particle system version
	}

	// --- Cleanup ---
	wglMakeCurrent(NULL, NULL);
	if (g_hRC) wglDeleteContext(g_hRC);
	if (g_hDC) ReleaseDC(hWnd, g_hDC);
	DestroyWindow(hWnd);
	UnregisterClass(WINDOW_TITLE, hInst);

	return static_cast<int>(msg.wParam);
}