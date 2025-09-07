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
	// Define a small epsilon for normal calculation to avoid division by zero.
	float EPSILON = 0.0001f;

	// Loop through each segment of the profile line
	for (int i = 0; i < num_points - 1; ++i)
	{
		glBegin(GL_TRIANGLE_STRIP);

		for (int j = 0; j <= sides; ++j)
		{
			float angle_rad = (float)j / (float)sides * 2.0f * 3.14159f;
			float cos_angle = cos(angle_rad);
			float sin_angle = sin(angle_rad);

			// Calculate current normal more accurately for the profile slope
			float dx = profile[i + 1][0] - profile[i][0];
			float dy = profile[i + 1][1] - profile[i][1];

			// If profile is perfectly vertical or horizontal, use default normal
			float normal_x_profile = -dy; // Perpendicular to slope in 2D
			float normal_y_profile = dx;

			// Normalize the 2D normal component
			float normal_len_2d = sqrt(normal_x_profile * normal_x_profile + normal_y_profile * normal_y_profile);
			if (normal_len_2d > EPSILON) {
				normal_x_profile /= normal_len_2d;
				normal_y_profile /= normal_len_2d;
			}
			else {
				normal_x_profile = 0.0f; // Default if slope is zero
				normal_y_profile = 1.0f;
			}

			// Transform 2D normal to 3D normal, revolving it around Y-axis
			float normal_x_3d = normal_x_profile * cos_angle;
			float normal_z_3d = normal_x_profile * sin_angle;

			// --- Vertex at the current level (i) ---
			glVertex3f(profile[i][0] * cos_angle, profile[i][1], profile[i][0] * sin_angle);
			glNormal3f(normal_x_3d, normal_y_profile, normal_z_3d);


			// --- Vertex at the next level (i+1) ---
			glVertex3f(profile[i + 1][0] * cos_angle, profile[i + 1][1], profile[i + 1][0] * sin_angle);
			glNormal3f(normal_x_3d, normal_y_profile, normal_z_3d);
		}
		glEnd();
	}
}

// Draws the distinct, layered waist "lines" or armor segments as separate rings.
void drawWaistLines()
{
	glColor3f(0.8f, 0.6f, 0.0f); // Darker gold for the waist segments

	// These segments will be stacked vertically between the chest (ends at Y=0.25)
	// and the lower waist (starts at Y=-0.05).
	// Let's create 3-4 segments.

	float current_y = 0.20f; // Starting Y position (below chest, above lower waist)
	float segment_height = 0.06f; // Height of each segment
	float segment_thickness = 0.03f; // Thickness of the armor ring
	int sides = 18; // Smoothness of the ring

	// We'll vary the radius slightly to follow the body's curve
	float radii[][2] = { // {inner_radius, outer_radius} for each segment
		{0.16f, 0.20f}, // Topmost segment (smaller, starts near chest base)
		{0.14f, 0.18f},
		{0.12f, 0.16f}, // Narrowest at center
		{0.14f, 0.18f}  // Widening towards lower waist
	};

	for (int i = 0; i < 4; ++i) // Draw 4 segments
	{
		glPushMatrix();
		glTranslatef(0.0f, current_y - i * (segment_height + 0.01f), 0.0f); // Position each segment down

		float inner_r = radii[i][0];
		float outer_r = radii[i][1];

		// Draw the main ring (like a thick pipe)
		glBegin(GL_QUAD_STRIP);
		for (int j = 0; j <= sides; ++j)
		{
			float angle = (float)j / (float)sides * 2.0f * 3.14159f;
			float cos_angle = cos(angle);
			float sin_angle = sin(angle);

			// Vertices for the outer edge of the strip
			glNormal3f(cos_angle, 0.0f, sin_angle); // Normal points outwards
			glVertex3f(outer_r * cos_angle, segment_height / 2.0f, outer_r * sin_angle);

			// Vertices for the inner edge of the strip (this defines the thickness)
			glNormal3f(cos_angle, 0.0f, sin_angle); // Normal points outwards (approx)
			glVertex3f(inner_r * cos_angle, segment_height / 2.0f, inner_r * sin_angle);

			glNormal3f(cos_angle, 0.0f, sin_angle);
			glVertex3f(outer_r * cos_angle, -segment_height / 2.0f, outer_r * sin_angle);

			glNormal3f(cos_angle, 0.0f, sin_angle);
			glVertex3f(inner_r * cos_angle, -segment_height / 2.0f, inner_r * sin_angle);
		}
		glEnd();

		// Optionally, draw top/bottom caps for a more solid look
		// Top Cap (from inner_r to outer_r)
		glBegin(GL_QUAD_STRIP);
		glNormal3f(0.0f, 1.0f, 0.0f); // Normal points up
		for (int j = 0; j <= sides; ++j) {
			float angle = (float)j / (float)sides * 2.0f * 3.14159f;
			glVertex3f(outer_r * cos(angle), segment_height / 2.0f, outer_r * sin(angle));
			glVertex3f(inner_r * cos(angle), segment_height / 2.0f, inner_r * sin(angle));
		}
		glEnd();

		// Bottom Cap (from outer_r to inner_r, reversed for normal)
		glBegin(GL_QUAD_STRIP);
		glNormal3f(0.0f, -1.0f, 0.0f); // Normal points down
		for (int j = sides; j >= 0; --j) {
			float angle = (float)j / (float)sides * 2.0f * 3.14159f;
			glVertex3f(outer_r * cos(angle), -segment_height / 2.0f, outer_r * sin(angle));
			glVertex3f(inner_r * cos(angle), -segment_height / 2.0f, inner_r * sin(angle));
		}
		glEnd();

		glPopMatrix();
	}
}

// Draws the smooth, curved CHEST section.
void drawSmoothChest()
{
	glColor3f(1.0f, 0.84f, 0.0f); // Golden yellow

	// Profile for the CHEST section. It now ends above the waist.
	float chest_profile[][2] = {
		{0.18f, 0.85f},  // Top of chest (connects to neck)
		{0.35f, 0.70f},  // Widest part of chest
		{0.38f, 0.55f},  // Fuller chest volume
		{0.30f, 0.40f},  // Tapering towards bottom of chest
		{0.25f, 0.25f}   // Bottom edge of chest, above the gap
	};
	int chest_points = sizeof(chest_profile) / sizeof(chest_profile[0]);

	drawLathedObject(chest_profile, chest_points, 20);
}

// Draws the smooth, curved LOWER WAIST and HIPS section.
void drawSmoothWaistLower()
{
	glColor3f(1.0f, 0.84f, 0.0f); // Golden yellow

	// Profile for the LOWER WAIST and HIPS. Starts below the gap.
	float waist_lower_profile[][2] = {
		{0.18f, -0.05f}, // Top of lower waist (below the gap)
		{0.20f, -0.1f},  // Start of hips
		{0.38f, -0.5f},  // Widest part of hips
		{0.35f, -0.8f}   // Bottom of torso
	};
	int waist_lower_points = sizeof(waist_lower_profile) / sizeof(waist_lower_profile[0]);

	drawLathedObject(waist_lower_profile, waist_lower_points, 20);
}

// Draws a smooth, cylindrical neck.
void drawSmoothNeck()
{
	glColor3f(1.0f, 0.84f, 0.0f); // Golden yellow

	// Profile for the neck: narrower and shorter to allow space for the folded collar.
	float neck_profile[][2] = {
		{0.18f, 0.85f}, // Bottom of neck (matches chest top)
		{0.18f, 0.95f}  // Top of neck (shorter than before)
	};
	int neck_points = sizeof(neck_profile) / sizeof(neck_profile[0]);
	drawLathedObject(neck_profile, neck_points, 16);
}

// Draws the collar around the neck with a layered, more refined look.
void drawNeckCollar()
{
	glColor3f(0.8f, 0.6f, 0.0f); // Darker gold for the collar

	glPushMatrix();
	// Position the collar at the base of the neck, fitting closely.
	// Neck's base Y is 0.85, top is 1.05. Collar should wrap around Y=0.9 to Y=1.0.
	glTranslatef(0.0f, 0.95f, 0.0f); // Centered height of the collar

	float base_inner_radius = 0.18f; // Inner radius at the bottom of the collar
	float base_outer_radius = 0.23f; // Outer radius at the bottom of the collar
	float top_inner_radius = 0.20f;  // Inner radius at the top of the collar (slightly wider)
	float top_outer_radius = 0.26f;  // Outer radius at the top of the collar (slightly wider)
	float collar_height = 0.10f;     // Total height of the collar
	int sides = 20;                   // Smoothness of the ring

	// Main body of the collar, slightly conical for a better fit
	glBegin(GL_QUAD_STRIP);
	for (int i = 0; i <= sides; ++i)
	{
		float angle = (float)i / (float)sides * 2.0f * 3.14159f;
		float cos_angle = cos(angle);
		float sin_angle = sin(angle);

		// Bottom-outer vertex
		glNormal3f(cos_angle, -0.2f, sin_angle); // Normal slightly angled downwards
		glVertex3f(base_outer_radius * cos_angle, -collar_height / 2.0f, base_outer_radius * sin_angle);

		// Bottom-inner vertex
		glNormal3f(cos_angle, -0.2f, sin_angle);
		glVertex3f(base_inner_radius * cos_angle, -collar_height / 2.0f, base_inner_radius * sin_angle);

		// Top-outer vertex
		glNormal3f(cos_angle, 0.2f, sin_angle); // Normal slightly angled upwards
		glVertex3f(top_outer_radius * cos_angle, collar_height / 2.0f, top_outer_radius * sin_angle);

		// Top-inner vertex
		glNormal3f(cos_angle, 0.2f, sin_angle);
		glVertex3f(top_inner_radius * cos_angle, collar_height / 2.0f, top_inner_radius * sin_angle);
	}
	glEnd();

	// Now for the "layered" look, let's add a second, thinner ring on top.
	glColor3f(0.9f, 0.7f, 0.1f); // Lighter gold for the top layer
	float layer_height = 0.03f; // Height of this layer
	float layer_offset_y = collar_height / 2.0f - layer_height / 2.0f + 0.01f; // Position slightly above main collar

	glPushMatrix();
	glTranslatef(0.0f, layer_offset_y, 0.0f); // Move up for the layer

	float layer_inner_r = top_inner_radius - 0.01f; // Slightly smaller than top inner
	float layer_outer_r = top_outer_radius + 0.01f; // Slightly larger than top outer

	glBegin(GL_QUAD_STRIP);
	for (int i = 0; i <= sides; ++i)
	{
		float angle = (float)i / (float)sides * 2.0f * 3.14159f;
		float cos_angle = cos(angle);
		float sin_angle = sin(angle);

		glNormal3f(cos_angle, 0.0f, sin_angle);
		glVertex3f(layer_outer_r * cos_angle, layer_height / 2.0f, layer_outer_r * sin_angle);
		glNormal3f(cos_angle, 0.0f, sin_angle);
		glVertex3f(layer_inner_r * cos_angle, layer_height / 2.0f, layer_inner_r * sin_angle);
		glNormal3f(cos_angle, 0.0f, sin_angle);
		glVertex3f(layer_outer_r * cos_angle, -layer_height / 2.0f, layer_outer_r * sin_angle);
		glNormal3f(cos_angle, 0.0f, sin_angle);
		glVertex3f(layer_inner_r * cos_angle, -layer_height / 2.0f, layer_inner_r * sin_angle);
	}
	glEnd();
	glPopMatrix(); // Pop layer transform

	glPopMatrix(); // Pop main collar transform

	// Reset color to main body color if needed for subsequent parts
	glColor3f(1.0f, 0.84f, 0.0f);
}

// Draws smooth, cylindrical arms.
void drawSmoothArms()
{
	glColor3f(1.0f, 0.84f, 0.0f); // Golden yellow

	// --- Left Arm ---
	glPushMatrix();
	// Adjusted Y to connect to the bottom of the chest
	glTranslatef(-0.6f, 0.6f, 0.0f); // WAS: -0.6f, 0.8f, 0.0f
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
	// Adjusted Y to connect to the bottom of the chest
	glTranslatef(0.6f, 0.6f, 0.0f); // WAS: 0.6f, 0.8f, 0.0f
	glRotatef(-10, 0, 0, 1);
	drawLathedObject(upper_arm_profile, 2, 12);
	glTranslatef(0.0f, -0.5f, 0.0f);
	drawLathedObject(lower_arm_profile, 2, 12);
	glTranslatef(0.0f, -0.45f, 0.0f);
	glScalef(0.16f, 0.1f, 0.16f);
	drawCuboid(1, 1, 1);
	glPopMatrix();
}

// Draws the middle waist section WITH the vertical lines/grooves from the sketch.
void drawWaistWithVerticalLines()
{
	// The waist connects the chest (ends at Y=0.25) and the lower waist (starts at Y=-0.05)
	float waist_top_y = 0.25f;
	float waist_bottom_y = -0.05f;

	// Define the radius at the top and bottom of this waist section
	float top_radius = 0.25f;  // Matches the bottom of the chest
	float bottom_radius = 0.18f; // Matches the top of the lower waist/hips

	int sides = 24; // Use more sides for a smoother curve and better line definition
	int num_lines = 6; // We want 6 vertical lines as seen in some interpretations

	glBegin(GL_QUADS);
	for (int i = 0; i < sides; ++i)
	{
		float angle1 = (float)i / (float)sides * 2.0f * 3.14159f;
		float angle2 = (float)(i + 1) / (float)sides * 2.0f * 3.14159f;

		// Determine if this segment is a "line" or a "panel"
		// We will make every few segments a recessed "line"
		bool isLineSegment = (i % (sides / num_lines) == 0);

		if (isLineSegment) {
			glColor3f(0.8f, 0.6f, 0.0f); // Darker gold for the recessed "line"
		}
		else {
			glColor3f(1.0f, 0.84f, 0.0f); // Standard golden yellow for the panels
		}

		// Calculate the four vertices of this vertical panel
		// Top-left
		float v1x = top_radius * cos(angle1);
		float v1z = top_radius * sin(angle1);
		// Top-right
		float v2x = top_radius * cos(angle2);
		float v2z = top_radius * sin(angle2);
		// Bottom-right
		float v3x = bottom_radius * cos(angle2);
		float v3z = bottom_radius * sin(angle2);
		// Bottom-left
		float v4x = bottom_radius * cos(angle1);
		float v4z = bottom_radius * sin(angle1);

		// Calculate the normal for this panel. It points outwards from the center.
		// We can approximate it by taking the average angle.
		float avg_angle = (angle1 + angle2) / 2.0f;
		glNormal3f(cos(avg_angle), 0.0f, sin(avg_angle));

		// Draw the quad for this panel
		glVertex3f(v1x, waist_top_y, v1z);
		glVertex3f(v2x, waist_top_y, v2z);
		glVertex3f(v3x, waist_bottom_y, v3z);
		glVertex3f(v4x, waist_bottom_y, v4z);
	}
	glEnd();
}

// Draws the belt and buckle as a separate object, placed over the hips.
void drawBeltAndBuckle()
{
	// The belt sits on top of the lower waist/hip section.
	// Let's place it around Y = -0.7f
	float belt_y = -0.7f;
	float belt_radius_inner = 0.34f; // Should be slightly larger than the hip radius at that Y
	float belt_radius_outer = 0.37f;
	float belt_height = 0.08f;
	int sides = 20;

	// --- Belt Strap ---
	glColor3f(0.8f, 0.6f, 0.2f); // Darker gold for the belt strap
	glPushMatrix();
	glTranslatef(0.0f, belt_y, 0.0f);

	glBegin(GL_QUAD_STRIP);
	glNormal3f(0.0f, 1.0f, 0.0f); // Top face normal
	for (int i = 0; i <= sides; ++i) {
		float angle = (float)i / (float)sides * 2.0f * 3.14159f;
		glVertex3f(belt_radius_outer * cos(angle), belt_height / 2.0f, belt_radius_outer * sin(angle));
		glVertex3f(belt_radius_inner * cos(angle), belt_height / 2.0f, belt_radius_inner * sin(angle));
	}
	glEnd();

	glBegin(GL_QUAD_STRIP);
	glNormal3f(0.0f, -1.0f, 0.0f); // Bottom face normal
	for (int i = 0; i <= sides; ++i) {
		float angle = (float)i / (float)sides * 2.0f * 3.14159f;
		glVertex3f(belt_radius_outer * cos(angle), -belt_height / 2.0f, belt_radius_outer * sin(angle));
		glVertex3f(belt_radius_inner * cos(angle), -belt_height / 2.0f, belt_radius_inner * sin(angle));
	}
	glEnd();

	glBegin(GL_QUAD_STRIP);
	for (int i = 0; i <= sides; ++i) {
		float angle = (float)i / (float)sides * 2.0f * 3.14159f;
		glNormal3f(cos(angle), 0.0f, sin(angle)); // Outer face normal
		glVertex3f(belt_radius_outer * cos(angle), belt_height / 2.0f, belt_radius_outer * sin(angle));
		glVertex3f(belt_radius_outer * cos(angle), -belt_height / 2.0f, belt_radius_outer * sin(angle));
	}
	glEnd();
	glPopMatrix();


	// --- Buckle ---
	// This is the more detailed buckle. We are keeping this one.
	glColor3f(0.9f, 0.9f, 0.9f); // Lighter color for the buckle gem
	float b_d = 0.05f; // Buckle depth
	glPushMatrix();
	// Position buckle in front of the belt
	glTranslatef(0.0f, belt_y, belt_radius_outer);

	glBegin(GL_TRIANGLES);
	// Front face
	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, -0.06f, b_d);
	glVertex3f(0.08f, 0.08f, b_d);
	glVertex3f(-0.08f, 0.08f, b_d);
	glEnd();

	glBegin(GL_QUADS);
	// Slanted side faces of the triangular prism
	glNormal3f(0.8f, 0.6f, 0.0f); // Right-top face normal
	glVertex3f(0.08f, 0.08f, b_d); glVertex3f(0.0f, -0.06f, b_d);
	glVertex3f(0.0f, -0.06f, -b_d); glVertex3f(0.08f, 0.08f, -b_d);

	glNormal3f(-0.8f, 0.6f, 0.0f); // Left-top face normal
	glVertex3f(-0.08f, 0.08f, b_d); glVertex3f(-0.08f, 0.08f, -b_d);
	glVertex3f(0.0f, -0.06f, -b_d); glVertex3f(0.0f, -0.06f, b_d);

	glNormal3f(0.0f, -1.0f, 0.0f); // Bottom face normal
	glVertex3f(0.08f, 0.08f, b_d); glVertex3f(-0.08f, 0.08f, b_d);
	glVertex3f(-0.08f, 0.08f, -b_d); glVertex3f(0.08f, 0.08f, -b_d);
	glEnd();

	glPopMatrix();
}

// Draws the folded collar around the neck, as seen in the sketch.
void drawFoldedCollar()
{
	glColor3f(0.8f, 0.6f, 0.0f); // Darker gold for the collar

	glPushMatrix();
	// Position the collar slightly above the neck, starting from its base.
	// Neck top is at Y=0.95f. Collar starts around Y=0.85f and goes up.
	glTranslatef(0.0f, 0.85f, 0.0f);

	float base_radius = 0.18f; // Radius at the base of the collar (around neck)
	float top_radius = 0.25f;  // Wider radius at the top/outer edge of the collar
	float collar_height = 0.15f; // Vertical extent of the collar
	float collar_thickness = 0.02f; // How thick the collar material is
	float front_overlap_z = 0.05f; // How much the front points overlap

	// --- Left side of the folded collar ---
	glBegin(GL_QUADS);
	// Inner face (closer to neck)
	glNormal3f(0.0f, 0.0f, -1.0f); // Normal points inwards (or slightly up/back)
	glVertex3f(-base_radius, 0.0f, 0.0f);
	glVertex3f(-base_radius * 0.9f, collar_height, 0.05f); // Pointing slightly up and forward
	glVertex3f(0.0f, collar_height * 0.8f, base_radius + front_overlap_z); // Front point
	glVertex3f(0.0f, 0.0f, base_radius * 0.8f);

	// Outer face (top side of the fold)
	glNormal3f(-0.5f, 0.5f, 0.5f); // Normal points up and out
	glVertex3f(-base_radius - collar_thickness, 0.0f, 0.0f); // Base
	glVertex3f(-top_radius, collar_height, 0.1f); // Outer top edge
	glVertex3f(0.0f, collar_height * 0.9f, top_radius + front_overlap_z); // Outer front point
	glVertex3f(0.0f, 0.0f, base_radius * 0.8f); // Connects to inner base

	// Side face (connecting inner and outer) - simplified
	glNormal3f(-1.0f, 0.0f, 0.0f);
	glVertex3f(-base_radius, 0.0f, 0.0f);
	glVertex3f(-base_radius - collar_thickness, 0.0f, 0.0f);
	glVertex3f(-top_radius, collar_height, 0.1f);
	glVertex3f(-base_radius * 0.9f, collar_height, 0.05f);

	glEnd();

	// --- Right side of the folded collar ---
	// (Mirror the left side by negating X coordinates and adjusting normals)
	glBegin(GL_QUADS);
	// Inner face
	glNormal3f(0.0f, 0.0f, -1.0f); // Normal points inwards
	glVertex3f(base_radius, 0.0f, 0.0f);
	glVertex3f(base_radius * 0.9f, collar_height, 0.05f);
	glVertex3f(0.0f, collar_height * 0.8f, base_radius + front_overlap_z); // Front point
	glVertex3f(0.0f, 0.0f, base_radius * 0.8f);

	// Outer face
	glNormal3f(0.5f, 0.5f, 0.5f); // Normal points up and out
	glVertex3f(base_radius + collar_thickness, 0.0f, 0.0f); // Base
	glVertex3f(top_radius, collar_height, 0.1f); // Outer top edge
	glVertex3f(0.0f, collar_height * 0.9f, top_radius + front_overlap_z); // Outer front point
	glVertex3f(0.0f, 0.0f, base_radius * 0.8f); // Connects to inner base

	// Side face
	glNormal3f(1.0f, 0.0f, 0.0f);
	glVertex3f(base_radius, 0.0f, 0.0f);
	glVertex3f(base_radius + collar_thickness, 0.0f, 0.0f);
	glVertex3f(top_radius, collar_height, 0.1f);
	glVertex3f(base_radius * 0.9f, collar_height, 0.05f);
	glEnd();

	glPopMatrix();

	glColor3f(1.0f, 0.84f, 0.0f); // Reset color
}

// Draws the armor collar (gorget) with a layered design, as seen in the sketch.
// This function replaces both the neck and the old collar functions.
void drawArmorCollar()
{
	// --- Main, Lower Collar Plate ---
	// This is the wider ring that rests on the chest.
	glColor3f(0.8f, 0.6f, 0.0f); // Darker gold for the main collar

	// Define the 2D cross-section profile of the lower ring.
	// It's a wide, relatively flat shape.
	// {X-radius, Y-height}
	float lower_collar_profile[][2] = {
		{0.20f, 0.85f}, // Inner edge, bottom
		{0.38f, 0.88f}, // Outer edge, slightly higher
		{0.38f, 0.84f}, // Outer edge, bottom (creates thickness)
		{0.20f, 0.82f}  // Inner edge, bottom (creates thickness)
	};
	int lower_collar_points = sizeof(lower_collar_profile) / sizeof(lower_collar_profile[0]);

	// Use the lathe function to create the smooth ring.
	drawLathedObject(lower_collar_profile, lower_collar_points, 24);


	// --- Inner, Upper Collar Ring ---
	// This is the smaller, thinner ring that sits on top, closer to the neck.
	glColor3f(0.9f, 0.7f, 0.1f); // Lighter gold for the inner ring

	// Define the profile for the smaller, upper ring.
	float upper_collar_profile[][2] = {
		{0.18f, 0.88f}, // Inner edge, bottom
		{0.24f, 0.90f}, // Outer edge, slightly higher
		{0.24f, 0.87f}, // Outer edge, bottom
		{0.18f, 0.86f}  // Inner edge, bottom
	};
	int upper_collar_points = sizeof(upper_collar_profile) / sizeof(upper_collar_profile[0]);

	drawLathedObject(upper_collar_profile, upper_collar_points, 20);

	// --- Simple Neck Cylinder inside the collar ---
	// A simple, short neck piece to fill the gap.
	glColor3f(1.0f, 0.84f, 0.0f); // Standard body color
	float neck_profile[][2] = {
		{0.17f, 0.88f}, // Base of the visible neck
		{0.17f, 0.95f}  // Top of the visible neck
	};
	drawLathedObject(neck_profile, 2, 16);
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
}

// -- Main Display Function --
void display()
{
	// --- Setup Code (unchanged) ---
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

	// --- Simplified and Corrected Drawing Calls ---
	drawSmoothChest();
	drawWaistWithVerticalLines();
	drawSmoothWaistLower();
	drawBeltAndBuckle();

	drawArmorCollar(); // REPLACES the old neck and collar functions

	drawSmoothArms();
	drawAngularParts();

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