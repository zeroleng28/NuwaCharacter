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

// --- Hand Animation State Variables ---
bool g_isFistAnimating = false;    // Is the open/close animation currently running?
bool g_isFistTargetClosed = false; // The state we are animating towards (true=closed, false=open)
float g_fistAnimationProgress = 0.0f; // 0.0 = fully open, 1.0 = fully closed

// --- Weapon State Variable ---
int g_equippedWeapon = 0;
GLuint g_mirrorTextureID = 0;

// Define the joint angles for the open and closed poses
const float FINGER_OPEN_ANGLES[3] = { -10.0f, -15.0f, -10.0f };
const float FINGER_CLOSED_ANGLES[3] = { -45.0f, -60.0f, -45.0f };
const float THUMB_OPEN_ANGLE = -20.0f; 
const float THUMB_CLOSED_ANGLE = -40.0f; 

int g_armAnimationState = 0;
float g_armAnimationTimer = 0.0f; // A timer for each phase of the animation

// We need to store the target angles for the two main poses.
// Format: {Shoulder Z-axis rotation, Shoulder X-axis rotation, Elbow X-axis rotation}
const float ARM_POSE_IDLE[3] = { -10.0f, 0.0f, -25.0f };
const float ARM_POSE_CASTING[3] = { -20.0f, -80.0f, -40.0f }; // Arms raised forward

const float ARM_POSE_WAVE[3] = { -90.0f, -30.0f, -90.0f };

// These variables will hold the CURRENT, interpolated angles for each arm during animation.
float g_rightArmAngles[3] = { ARM_POSE_IDLE[0], ARM_POSE_IDLE[1], ARM_POSE_IDLE[2] };
float g_leftArmAngles[3] = { -ARM_POSE_IDLE[0], ARM_POSE_IDLE[1], ARM_POSE_IDLE[2] }; // Left arm is mirrored on Z-axis

// Global for braid animation
float g_braidTime = 0.0f;
float g_windStrength = 0.8f;
float g_braidSegmentLength = 0.1f; // Make segments a bit shorter for more detail
int g_numBraidSegments = 15;      // INCREASE this to make the braid longer

float g_characterPosX = 0.0f;
float g_characterPosZ = 0.0f;
float g_animationTime = 0.0f; // An accumulator for the animation cycle
int g_walkDirection = 0;      // -1 for backward, 0 for idle, 1 for forward

// --- NEW: Animation State Variables ---
bool g_isHaloAnimating = false;
bool g_isHaloVisible = true;
float g_haloZ = -5.5f;
float g_haloScale = 0.38f;

// Store the light's initial and current animated position
GLfloat g_initialLightPos[4] = { 5.0f, 5.0f, 5.0f, 1.0f };
GLfloat g_animatedLightPos[4];

GLuint g_shoeTextureID = 0;
GLuint g_fireTextureID = 0;
GLuint g_goldTextureID = 0;
GLuint g_redTextureID = 0;
GLuint g_skyTextureID = 0;
GLuint g_silverTextureID = 0;
GLuint g_orangeTextureID = 0;

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

// --- Nuwa Skill Variables ---
GLuint g_nuwaSkillTextureID = 0;
bool g_isNuwaSkillActive = false;
float g_nuwaSkillLifetime = 0.0f;     // How long the skill lasts in seconds
float g_nuwaSkillDistance = 0.0f;     // How far it has travelled from the character
float g_characterAngleOnCast = 0.0f;  // Store the character's direction when the skill is cast
float g_characterCastPosX = 0.0f;     // Store the character's X position on cast
float g_characterCastPosZ = 0.0f;     // Store the character's Z position on cast

// --- Waving Animation Variables ---
bool g_isLeftWaveActive = false;   // Is the left hand wave toggled on?
bool g_isRightWaveActive = false;  // Is the right hand wave toggled on?
float g_leftWaveProgress = 0.0f;   // Animation progress for left hand (0=down, 1=up)
float g_rightWaveProgress = 0.0f;  // Animation progress for right hand (0=down, 1=up)

// Define the "raised" pose for the elbow
const float ARM_POSE_WAVE_ELBOW = -100.0f;

const float PEACE_STRAIGHT_ANGLES[3] = { -5.0f, -10.0f, -10.0f };
const float PEACE_CURLED_ANGLES[3] = { -85.0f, -90.0f, -70.0f };
const float PEACE_THUMB_ANGLE = -60.0f;

// --- Hand Pose Animation Variables ---
int g_handPoseTarget = 0;        // 0 = Normal, 1 = Peace Sign
float g_handPoseProgress = 0.0f; // Animation progress (0=normal, 1=peace sign)

bool g_isLevitating = false;
float g_levitationProgress = 0.0f;  
float g_characterYOffset = 0.0f;   
float g_torsoTiltAngle = 0.0f;     
float g_neckTiltAngle = 0.0f;      
const float ARM_POSE_LEVITATE[3] = { -45.0f, -20.0f, -15.0f };

bool g_isWaving = false;         
float g_waveProgress = 0.0f;

// --- ADD THIS NEAR THE TOP WITH OTHER GLOBAL VARIABLES ---

// Defines the animation states of the skill block
enum MatrixBlockState {
	SPAWNING,  // The block is just appearing
	EXPANDING, // The block is growing to its full size
	ACTIVE     // The block is at full size, waiting to expire
};

// Holds all the data for one instance of Nuwa's matrix skill
struct MatrixBlock {
	bool isActive = false;          // Is this block currently in use?
	MatrixBlockState state;         // Its current animation state

	float posX, posY, posZ;         // World position
	float scaleX, scaleY, scaleZ;   // Current scale for the expansion animation

	float lifetime;                 // How many seconds are left before it disappears
	float animationTimer;           // A timer for the current state (e.g., how long it's been expanding)
};

// This vector will hold all the blocks currently on screen
std::vector<MatrixBlock> g_matrixBlocks;
GLuint g_matrixTextureID = 0; // We'll need a new texture for the block

void resetAnimation() {
	g_isHaloAnimating = false;
	g_isHaloVisible = true;
	g_haloZ = -0.85f;
	g_haloScale = 0.38f;

	// Reset the light's position
	memcpy(g_animatedLightPos, g_initialLightPos, sizeof(GLfloat) * 4);

	// Reset walk animation state and character position
	g_walkDirection = 0;
	g_characterPosX = 0.0f;
	g_characterPosZ = 0.0f;
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

		// Toggle forward walk
		if (wParam == 'B') {
			g_walkDirection = (g_walkDirection == 1) ? 0 : 1;
		}

		// Toggle backward walk
		if (wParam == 'C') {
			g_walkDirection = (g_walkDirection == -1) ? 0 : -1;
		}

		// --- Toggle Fist Animation ---
		if (wParam == 'F') {
			g_isFistAnimating = true; // Start the animation
			g_isFistTargetClosed = !g_isFistTargetClosed; // Flip the target state
		}

		// --- Cast Nuwa Skill ---
		if (wParam == 'G') {
			if (g_armAnimationState == 0) {
				g_armAnimationState = 1;
				g_armAnimationTimer = 0.0f;

				g_isNuwaSkillActive = true;
				g_nuwaSkillLifetime = 3.0f;
				g_nuwaSkillDistance = 0.0f;
				g_characterAngleOnCast = rotateY;
				g_characterCastPosX = g_characterPosX;
				g_characterCastPosZ = g_characterPosZ;
			}
		}

		// --- Waving Toggles ---
		if (wParam == 'Q') {
			g_isLeftWaveActive = !g_isLeftWaveActive;
		}
		if (wParam == 'E') {
			g_isRightWaveActive = !g_isRightWaveActive;
		}

		// --- Peace Sign Toggle ---
		if (wParam == 'V') {
			g_handPoseTarget = (g_handPoseTarget == 0) ? 1 : 0; // Toggle target
		}

		// --- Toggle Weapon Visibility ---
		if (wParam == 'H') {
			g_equippedWeapon++;
			if (g_equippedWeapon > 2) { // Cycle through 0, 1, 2
				g_equippedWeapon = 0;
			}
		}

		if (wParam == 'M') {
			if (g_armAnimationState == 0) {
				g_armAnimationState = 1;
				g_armAnimationTimer = 0.0f;

				MatrixBlock newBlock;
				newBlock.isActive = true;
				newBlock.lifetime = 4.0f;
				newBlock.state = SPAWNING;
				newBlock.animationTimer = 0.0f;
				newBlock.scaleX = newBlock.scaleY = newBlock.scaleZ = 0.1f;

				const float spawnAreaSize = 15.0f;
				float halfArea = spawnAreaSize / 2.0f;
				float offsetX = -halfArea + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / spawnAreaSize));
				float offsetZ = -halfArea + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / spawnAreaSize));

				newBlock.posX = g_characterPosX + offsetX;
				newBlock.posZ = g_characterPosZ + offsetZ;
				newBlock.posY = 1.0f;

				g_matrixBlocks.push_back(newBlock);
			}
			break;
		}
		if (wParam == 'L') {
			g_isLevitating = !g_isLevitating;
		}

		if (wParam == VK_SPACE) {
			resetAnimation();
			g_isLeftWaveActive = false;
			g_isRightWaveActive = false;
			g_handPoseTarget = 0;
			g_handPoseProgress = 0.0f;
			g_equippedWeapon = 0;
			g_isLevitating = false; 
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
	// Use OutputDebugStringA for logging in a Win32 application
	char buffer[256];
	sprintf_s(buffer, "Reading image %s\n", imagepath);
	OutputDebugStringA(buffer);

	unsigned char header[54];
	unsigned int dataPos;
	unsigned int width, height;
	unsigned int imageSize;
	unsigned char* data;

	FILE* file;
	fopen_s(&file, imagepath, "rb");
	if (!file) {
		OutputDebugStringA("Error: Image could not be opened.\n");
		return 0;
	}

	if (fread(header, 1, 54, file) != 54) {
		OutputDebugStringA("Error: Not a correct BMP file (header read failed).\n");
		fclose(file);
		return 0;
	}
	if (header[0] != 'B' || header[1] != 'M') {
		OutputDebugStringA("Error: Not a correct BMP file (magic number mismatch).\n");
		fclose(file);
		return 0;
	}

	// Read important information from the header
	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	width = *(int*)&(header[0x12]);
	height = *(int*)&(header[0x16]);
	unsigned short bitsPerPixel = *(unsigned short*)&(header[0x1C]);

	// --- NEW: Handle both 24-bit (RGB) and 32-bit (RGBA) BMPs ---
	GLenum internalFormat;
	GLenum pixelFormat;
	int bytesPerPixel;

	if (bitsPerPixel == 32) {
		internalFormat = GL_RGBA;
		pixelFormat = GL_BGRA_EXT; // Windows BMPs store alpha channels in BGRA format
		bytesPerPixel = 4;
	}
	else if (bitsPerPixel == 24) {
		internalFormat = GL_RGB;
		pixelFormat = GL_BGR_EXT;  // Windows BMPs store colors in BGR format
		bytesPerPixel = 3;
	}
	else {
		sprintf_s(buffer, "Error: Unsupported BMP format (%d bits per pixel).\n", bitsPerPixel);
		OutputDebugStringA(buffer);
		fclose(file);
		return 0;
	}

	// Some BMP files may leave these fields as 0
	if (imageSize == 0) { imageSize = width * height * bytesPerPixel; }
	if (dataPos == 0) { dataPos = 54; } // The size of the header

	// Create a buffer to hold the data
	data = new unsigned char[imageSize];

	// Read the actual image data from the file
	fseek(file, dataPos, SEEK_SET);
	size_t readResult = fread(data, 1, imageSize, file);

	// Error check after reading
	if (readResult != imageSize) {
		OutputDebugStringA("Error: Could not read all pixel data from file.\n");
		fclose(file);
		delete[] data; // IMPORTANT: Free memory on failure
		return 0;
	}

	fclose(file); // File is no longer needed

	// --- Create one OpenGL texture ---
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, pixelFormat, GL_UNSIGNED_BYTE, data);

	// Set Texture filtering and wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	// Generate Mipmaps for the texture
	gluBuild2DMipmaps(GL_TEXTURE_2D, internalFormat, width, height, pixelFormat, GL_UNSIGNED_BYTE, data);

	// Once the texture is loaded into VRAM, we can free the host RAM
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
	// Set material properties for the shoulder pads
	GLfloat mat_ambient[] = { 0.45f, 0.38f, 0.1f, 1.0f };
	GLfloat mat_diffuse[] = { 1.0f, 0.84f, 0.0f, 1.0f };
	GLfloat mat_specular[] = { 1.0f, 1.0f, 0.8f, 1.0f };
	GLfloat mat_shininess[] = { 100.0f };
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

	// --- NEW: Enable and apply the Silver texture ---
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, g_silverTextureID);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// Set the base colour to white for proper texturing
	glColor3f(1.0f, 1.0f, 1.0f);

	auto drawOneShoulder = [&](bool isLeft) {
		glPushMatrix();
		glTranslatef(isLeft ? -0.38f : 0.38f, 0.7f, 0.0f);
		glRotatef(isLeft ? 20.0f : -20.0f, 0.0f, 0.0f, 1.0f);
		glRotatef(-25.0f, 1.0f, 0.0f, 0.0f);

		// The cuboids will now be textured since texturing is enabled.
		// We can add texture coordinates to the cuboid function for better mapping,
		// but for a simple metal texture, the default mapping will work fine.

		// --- Bottom Layer (Largest, Darker Panel) ---
		glPushMatrix();
		glTranslatef(0.0f, 0.0f, -0.05f);
		glScalef(1.2f, 0.8f, 0.1f);
		glRotatef(isLeft ? 10.0f : -10.0f, 0.0f, 1.0f, 0.0f);
		drawCuboid(0.4f, 0.2f, 1.0f);
		glPopMatrix();

		// --- Middle Layer (Main Panel) ---
		glPushMatrix();
		glTranslatef(0.0f, 0.02f, 0.0f);
		glScalef(1.0f, 0.9f, 0.12f);
		glRotatef(isLeft ? -5.0f : 5.0f, 0.0f, 1.0f, 0.0f);
		drawCuboid(0.4f, 0.2f, 1.0f);
		glPopMatrix();

		// --- Top Layer (Smaller, More Angular Panel) ---
		glPushMatrix();
		glTranslatef(0.0f, 0.05f, 0.05f);
		glScalef(0.8f, 0.8f, 0.15f);
		drawCuboid(0.4f, 0.2f, 1.0f);
		glPopMatrix();

		glPopMatrix();
		};

	drawOneShoulder(true);
	drawOneShoulder(false);

	// --- NEW: Disable texturing after drawing ---
	glDisable(GL_TEXTURE_2D);
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

/* void drawHand(bool isLeftHand)
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
} */

// --- Low-poly human hand (game-style), shape-only update ---
// Keeps the same local origin & overall size so your arm code doesn't need changes.
static void drawTrapezoidPrism(float w_back, float w_wrist, float h, float d) {
	// back = knuckle side, wrist = forearm side
	float hb = h * 0.5f;
	float db = d * 0.5f;
	float wb = w_back * 0.5f;
	float ww = w_wrist * 0.5f;

	glBegin(GL_QUADS);
	// Top
	glNormal3f(0, 1, 0);
	glVertex3f(-ww, hb, db); glVertex3f(ww, hb, db);
	glVertex3f(wb, hb, -db); glVertex3f(-wb, hb, -db);
	// Bottom
	glNormal3f(0, -1, 0);
	glVertex3f(-ww, -hb, db); glVertex3f(-wb, -hb, -db);
	glVertex3f(wb, -hb, -db); glVertex3f(ww, -hb, db);
	// Front (palm)
	glNormal3f(0, 0, 1);
	glVertex3f(-ww, -hb, db); glVertex3f(ww, -hb, db);
	glVertex3f(ww, hb, db); glVertex3f(-ww, hb, db);
	// Back (knuckles)
	glNormal3f(0, 0, -1);
	glVertex3f(-wb, -hb, -db); glVertex3f(-wb, hb, -db);
	glVertex3f(wb, hb, -db); glVertex3f(wb, -hb, -db);
	// Left
	glNormal3f(-1, 0, 0);
	glVertex3f(-ww, -hb, db); glVertex3f(-ww, hb, db);
	glVertex3f(-wb, hb, -db); glVertex3f(-wb, -hb, -db);
	// Right
	glNormal3f(1, 0, 0);
	glVertex3f(ww, -hb, db); glVertex3f(wb, -hb, -db);
	glVertex3f(wb, hb, -db); glVertex3f(ww, hb, db);
	glEnd();
}

static void drawBox(float w, float h, float d) { // small helper
	float hw = w * 0.5f, hh = h * 0.5f, hd = d * 0.5f;
	glBegin(GL_QUADS);
	glNormal3f(0, 0, 1);
	glVertex3f(-hw, -hh, hd); glVertex3f(hw, -hh, hd); glVertex3f(hw, hh, hd); glVertex3f(-hw, hh, hd);
	glNormal3f(0, 0, -1);
	glVertex3f(-hw, -hh, -hd); glVertex3f(-hw, hh, -hd); glVertex3f(hw, hh, -hd); glVertex3f(hw, -hh, -hd);
	glNormal3f(0, 1, 0);
	glVertex3f(-hw, hh, -hd); glVertex3f(-hw, hh, hd); glVertex3f(hw, hh, hd); glVertex3f(hw, hh, -hd);
	glNormal3f(0, -1, 0);
	glVertex3f(-hw, -hh, -hd); glVertex3f(hw, -hh, -hd); glVertex3f(hw, -hh, hd); glVertex3f(-hw, -hh, hd);
	glNormal3f(1, 0, 0);
	glVertex3f(hw, -hh, -hd); glVertex3f(hw, hh, -hd); glVertex3f(hw, hh, hd); glVertex3f(hw, -hh, hd);
	glNormal3f(-1, 0, 0);
	glVertex3f(-hw, -hh, -hd); glVertex3f(-hw, -hh, hd); glVertex3f(-hw, hh, hd); glVertex3f(-hw, hh, -hd);
	glEnd();
}

// ---------- helpers ----------
static void box6(float w, float h, float d) { // hard-edged box
	float x = w * 0.5f, y = h * 0.5f, z = d * 0.5f;
	glBegin(GL_QUADS);
	glNormal3f(0, 0, 1);  glVertex3f(-x, -y, z); glVertex3f(x, -y, z); glVertex3f(x, y, z); glVertex3f(-x, y, z);
	glNormal3f(0, 0, -1); glVertex3f(-x, -y, -z); glVertex3f(-x, y, -z); glVertex3f(x, y, -z); glVertex3f(x, -y, -z);
	glNormal3f(0, 1, 0);  glVertex3f(-x, y, -z); glVertex3f(-x, y, z); glVertex3f(x, y, z); glVertex3f(x, y, -z);
	glNormal3f(0, -1, 0); glVertex3f(-x, -y, -z); glVertex3f(x, -y, -z); glVertex3f(x, -y, z); glVertex3f(-x, -y, z);
	glNormal3f(1, 0, 0);  glVertex3f(x, -y, -z); glVertex3f(x, y, -z); glVertex3f(x, y, z); glVertex3f(x, -y, z);
	glNormal3f(-1, 0, 0); glVertex3f(-x, -y, -z); glVertex3f(-x, -y, z); glVertex3f(-x, y, z); glVertex3f(-x, y, -z);
	glEnd();
}

static void drawPalmWedge(float wKnuckle, float wWrist, float thick, float depth)
{
	float hk = thick * 0.5f;
	float dz = depth * 0.5f;
	float wk = wKnuckle * 0.5f;
	float ww = wWrist * 0.5f;

	glBegin(GL_QUADS);
	// top 
	glNormal3f(0, 1, 0);
	glVertex3f(-ww, hk, dz);
	glVertex3f(ww, hk, dz);
	glVertex3f(wk, hk * 0.7f, -dz); 
	glVertex3f(-wk, hk * 0.7f, -dz);

	// bottom
	glNormal3f(0, -1, 0);
	glVertex3f(-ww, -hk, dz);
	glVertex3f(-wk, -hk, -dz);
	glVertex3f(wk, -hk, -dz);
	glVertex3f(ww, -hk, dz);

	// front 
	glNormal3f(0, 0, 1);
	glVertex3f(-ww, -hk, dz);
	glVertex3f(ww, -hk, dz);
	glVertex3f(ww, hk, dz);
	glVertex3f(-ww, hk, dz);

	// back 
	glNormal3f(0, 0, -1);
	glVertex3f(-wk, -hk, -dz);
	glVertex3f(-wk, hk * 0.7f, -dz);
	glVertex3f(wk, hk * 0.7f, -dz);
	glVertex3f(wk, -hk, -dz);

	// left
	glNormal3f(-1, 0, 0);
	glVertex3f(-ww, -hk, dz);
	glVertex3f(-ww, hk, dz);
	glVertex3f(-wk, hk * 0.7f, -dz);
	glVertex3f(-wk, -hk, -dz);

	// right
	glNormal3f(1, 0, 0);
	glVertex3f(ww, -hk, dz);
	glVertex3f(wk, -hk, -dz);
	glVertex3f(wk, hk * 0.7f, -dz);
	glVertex3f(ww, hk, dz);
	glEnd();
}

// ---------- main ----------
void drawHand(bool isLeftHand)
{
	// NOTE: This function now INHERITS the color and texture state from its caller (drawSmoothArms)
	// All glColor, glEnable, glBindTexture, glDisable calls have been removed.

	glPushMatrix();
	glTranslatef(0.0f, -0.05f, 0.0f);
	glRotatef(1.0f, 1, 0, 0);

	// ===== 1) Palm & Knuckles =====
	const float PALM_W_K = 0.205f;
	const float PALM_W_W = 0.155f;
	const float PALM_H = 0.14f;
	const float PALM_D = 0.05f;
	drawPalmWedge(PALM_W_K, PALM_W_W, PALM_H, PALM_D);
	glPushMatrix();
	glTranslatef(0.0f, PALM_H * 0.47f, -PALM_D * 0.48f);
	box6(PALM_W_K * 0.95f, 0.018f, 0.026f);
	glPopMatrix();

	// ===== 2) Fingers =====
	struct F { float x, yawDeg, len; float w, d; };
	F fingers[4] = {
		{ -0.060f, -9.0f, 0.072f, 0.030f, 0.034f }, // i=0
		{ -0.020f, -3.0f, 0.078f, 0.032f, 0.036f }, // i=1
		{  0.020f,  3.0f, 0.074f, 0.031f, 0.035f }, // i=2
		{  0.058f,  9.0f, 0.064f, 0.028f, 0.033f }, // i=3
	};

	float angle1, angle2, angle3;

	for (int i = 0; i < 4; ++i) {
		bool should_be_straight = false;
		if (isLeftHand) {
			if (i == 0 || i == 1) should_be_straight = true;
		}
		else { 
			if (i == 2 || i == 3) should_be_straight = true;
		}

		float normal_angle1 = FINGER_OPEN_ANGLES[0] + (FINGER_CLOSED_ANGLES[0] - FINGER_OPEN_ANGLES[0]) * g_fistAnimationProgress;
		float normal_angle2 = FINGER_OPEN_ANGLES[1] + (FINGER_CLOSED_ANGLES[1] - FINGER_OPEN_ANGLES[1]) * g_fistAnimationProgress;
		float normal_angle3 = FINGER_OPEN_ANGLES[2] + (FINGER_CLOSED_ANGLES[2] - FINGER_OPEN_ANGLES[2]) * g_fistAnimationProgress;

		float peace_angle1 = should_be_straight ? PEACE_STRAIGHT_ANGLES[0] : PEACE_CURLED_ANGLES[0];
		float peace_angle2 = should_be_straight ? PEACE_STRAIGHT_ANGLES[1] : PEACE_CURLED_ANGLES[1];
		float peace_angle3 = should_be_straight ? PEACE_STRAIGHT_ANGLES[2] : PEACE_CURLED_ANGLES[2];

		angle1 = normal_angle1 + (peace_angle1 - normal_angle1) * g_handPoseProgress;
		angle2 = normal_angle2 + (peace_angle2 - normal_angle2) * g_handPoseProgress;
		angle3 = normal_angle3 + (peace_angle3 - normal_angle3) * g_handPoseProgress;

		glPushMatrix();
		glTranslatef(fingers[i].x, -PALM_H * 0.24f, -PALM_D * 0.50f);
		float yaw = fingers[i].yawDeg;
		if (!isLeftHand) yaw = -yaw;
		glRotatef(yaw, 0, 1, 0);
		glRotatef(angle1, 1, 0, 0);

		float L1 = fingers[i].len, W1 = fingers[i].w, D1 = fingers[i].d;
		glTranslatef(0, -L1 * 0.5f, 0); box6(W1, L1, D1);
		float L2 = L1 * 0.86f, W2 = W1 * 0.92f, D2 = D1 * 0.92f;
		glTranslatef(0, -L1 * 0.5f, 0);
		glRotatef(angle2, 1, 0, 0);
		glTranslatef(0, -L2 * 0.5f, 0); box6(W2, L2, D2);
		float L3 = L2 * 0.80f, W3 = W2 * 0.88f, D3 = D2 * 0.90f;
		glTranslatef(0, -L2 * 0.5f, 0);
		glRotatef(angle3, 1, 0, 0);
		glTranslatef(0, -L3 * 0.5f, 0); box6(W3, L3, D3);
		glPopMatrix();
	}

	// ===== 3) Thumb =====
	glPushMatrix();
	glTranslatef(isLeftHand ? -PALM_W_W * 0.56f : PALM_W_W * 0.56f, -PALM_H * 0.02f, -PALM_D * 0.12f);
	glRotatef(52.f, 0, (isLeftHand ? 1 : -1), 0);
	glRotatef(10.f, 0, 0, 1);
	glRotatef(18.f, 1, 0, 0);

	float T1L = 0.074f, T1W = 0.044f, T1D = 0.046f;
	glTranslatef(0, -T1L * 0.5f, 0); box6(T1W, T1L, T1D);

	float thumb_normal_angle = THUMB_OPEN_ANGLE + (THUMB_CLOSED_ANGLE - THUMB_OPEN_ANGLE) * g_fistAnimationProgress;
	float thumb_peace_angle = PEACE_THUMB_ANGLE;
	float thumb_angle = thumb_normal_angle + (thumb_peace_angle - thumb_normal_angle) * g_handPoseProgress;

	float T2L = 0.060f, T2W = T1W * 0.90f, T2D = T1D * 0.92f;
	glTranslatef(0, -T1L * 0.5f, 0);
	glRotatef(thumb_angle, 1, 0, 0);
	glTranslatef(0, -T2L * 0.5f, 0); box6(T2W, T2L, T2D);
	glPopMatrix();

	// ===== 4) Palm bevel =====
	glPushMatrix();
	glTranslatef(0.0f, -PALM_H * 0.40f, PALM_D * 0.30f);
	glScalef(1.0f, 1.0f, 0.6f);
	box6(PALM_W_W * 0.90f, 0.020f, 0.040f);
	glPopMatrix();

	glPopMatrix();
}

void drawSmoothChest()
{
	// Set the base colour to white so the texture is not tinted
	glColor3f(1.0f, 1.0f, 1.0f);

	// Enable and apply the texture
	glEnable(GL_TEXTURE_2D);

	// --- MODIFIED: Changed from g_goldTextureID to g_silverTextureID ---
	glBindTexture(GL_TEXTURE_2D, g_silverTextureID);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	float chest_profile[][2] = {
		{0.18f, 0.85f},
		{0.35f, 0.70f},
		{0.38f, 0.55f},
		{0.30f, 0.40f},
		{0.25f, 0.25f}
	};
	int chest_points = sizeof(chest_profile) / sizeof(chest_profile[0]);
	drawLathedObject(chest_profile, chest_points, 20);

	// Disable texturing afterwards
	glDisable(GL_TEXTURE_2D);
}

void drawSmoothLowerBodyAndSkirt()
{
	// Set the base material colour to white. This allows the texture's own colours
	// to show up correctly without being tinted yellow.
	glColor3f(1.0f, 1.0f, 1.0f);

	// --- NEW: Enable and apply the silver texture ---
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, g_silverTextureID);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // Blends texture with lighting

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

	// --- NEW: Disable texturing after drawing the skirt ---
	// This is important so the texture doesn't accidentally get applied to other objects.
	glDisable(GL_TEXTURE_2D);
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

void drawDivineMirror() {
	glPushMatrix();

	// --- 1. Position the Mirror ---
	// We'll have it float gracefully near the character's right hand.
	// This position is relative to the hand's location.
	glTranslatef(0.3f, 0.2f, 0.5f);
	glRotatef(-90.0f, 0.0f, 1.0f, 0.0f); // Face the mirror forward
	glRotatef(-15.0f, 1.0f, 0.0f, 0.0f); // Tilt it slightly up

	// Animate a slow, mystical rotation and hover
	glRotatef(g_braidTime * 15.0f, 0.0f, 0.0f, 1.0f); // Slow spin
	glTranslatef(0.0f, sin(g_braidTime) * 0.05f, 0.0f); // Gentle up-down bob

	// --- 2. Set Material for the Frame (Shiny Silver/Jade) ---
	GLfloat mat_ambient[] = { 0.8f, 0.9f, 0.8f, 1.0f };
	GLfloat mat_diffuse[] = { 0.9f, 1.0f, 0.9f, 1.0f };
	GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat mat_shininess[] = { 100.0f };
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	glColor3f(0.9f, 1.0f, 0.9f);

	// --- 3. Draw the Octagonal Frame ---
	const int sides = 8;
	const float outer_radius = 0.4f;
	const float inner_radius = 0.35f;
	const float thickness = 0.05f;

	for (int i = 0; i < sides; ++i) {
		float angle1 = (float)i / sides * 2.0f * 3.14159f;
		float angle2 = (float)(i + 1) / sides * 2.0f * 3.14159f;

		// Vertices for one segment of the frame
		float v[8][3] = {
			{ outer_radius * cos(angle1), outer_radius * sin(angle1),  thickness / 2.0f },
			{ outer_radius * cos(angle2), outer_radius * sin(angle2),  thickness / 2.0f },
			{ inner_radius * cos(angle2), inner_radius * sin(angle2),  thickness / 2.0f },
			{ inner_radius * cos(angle1), inner_radius * sin(angle1),  thickness / 2.0f },
			{ outer_radius * cos(angle1), outer_radius * sin(angle1), -thickness / 2.0f },
			{ outer_radius * cos(angle2), outer_radius * sin(angle2), -thickness / 2.0f },
			{ inner_radius * cos(angle2), inner_radius * sin(angle2), -thickness / 2.0f },
			{ inner_radius * cos(angle1), inner_radius * sin(angle1), -thickness / 2.0f }
		};

		glBegin(GL_QUADS);
		// Front face
		glNormal3f(0, 0, 1);
		glVertex3fv(v[0]); glVertex3fv(v[1]); glVertex3fv(v[2]); glVertex3fv(v[3]);
		// Back face
		glNormal3f(0, 0, -1);
		glVertex3fv(v[4]); glVertex3fv(v[7]); glVertex3fv(v[6]); glVertex3fv(v[5]);
		// Outer face
		float nx_out = cos((angle1 + angle2) / 2.0f);
		float ny_out = sin((angle1 + angle2) / 2.0f);
		glNormal3f(nx_out, ny_out, 0);
		glVertex3fv(v[0]); glVertex3fv(v[4]); glVertex3fv(v[5]); glVertex3fv(v[1]);
		// Inner face
		glNormal3f(-nx_out, -ny_out, 0);
		glVertex3fv(v[3]); glVertex3fv(v[2]); glVertex3fv(v[6]); glVertex3fv(v[7]);
		glEnd();
	}

	// --- 4. Draw the Mirror Surface ---
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, g_mirrorTextureID); // Use the new mirror texture
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// Make the surface glow brightly
	glDisable(GL_LIGHTING);
	glColor3f(1.0f, 1.0f, 1.0f);

	glBegin(GL_POLYGON);
	glNormal3f(0, 0, 1);
	for (int i = 0; i < sides; ++i) {
		float angle = (float)i / sides * 2.0f * 3.14159f;
		// Map vertices to a circular texture coordinate space
		float u = 0.5f + 0.5f * cos(angle);
		float v = 0.5f + 0.5f * sin(angle);
		glTexCoord2f(u, v);
		glVertex3f(inner_radius * cos(angle), inner_radius * sin(angle), 0.0f);
	}
	glEnd();

	glEnable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);

	glPopMatrix();
}

void drawWeapon()
{
	// Set material for a shiny, magical weapon
	GLfloat mat_ambient[] = { 0.7f, 0.7f, 1.0f, 1.0f }; // Bluish ambient
	GLfloat mat_diffuse[] = { 0.8f, 0.8f, 1.0f, 1.0f }; // Bright blue/purple diffuse
	GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f }; // Bright white highlight
	GLfloat mat_shininess[] = { 120.0f };
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

	glPushMatrix();

	// --- THIS IS THE ADJUSTED TRANSFORMATION BLOCK ---

	// Step 1: Position the staff's pivot point slightly in front of the palm's center.
	// MODIFIED: Adjusted the Z-translation to move the staff slightly outwards.
	glTranslatef(0.0f, 0.0f, 0.15f); // Changed from 0.05f to 0.15f

	// Step 2: Rotate it to a natural holding angle.
	glRotatef(-75.0f, 0.0f, 0.0f, 1.0f);
	glRotatef(30.0f, 1.0f, 0.0f, 0.0f);

	// Step 3: After rotating, move the staff along its NEW down-axis to sit in the hand.
	glTranslatef(0.0f, -0.9f, 0.0f);

	// --- END OF ADJUSTED BLOCK ---


	// Now, draw the staff parts (this code remains the same)
	// --- 1. Draw the Staff Shaft ---
	glColor3f(0.3f, 0.2f, 0.4f);
	float shaft_profile[][2] = { {0.04f, 0.0f}, {0.04f, 1.8f} };
	drawLathedObject(shaft_profile, 2, 12);

	// --- 2. Draw the Crystal Topper ---
	glPushMatrix();
	glTranslatef(0.0f, 1.95f, 0.0f);
	glScalef(1.0f, 1.5f, 1.0f);

	glDisable(GL_LIGHTING);
	glColor3f(0.8f, 0.9f, 1.0f);
	drawDiamondKneeJoint();
	glEnable(GL_LIGHTING);
	glPopMatrix();

	// --- 3. Draw Decorative Elements ---
	glColor3f(1.0f, 0.84f, 0.0f);

	// Top holder for the crystal
	glPushMatrix();
	glTranslatef(0.0f, 1.85f, 0.0f);
	drawSphere(0.1f, 16, 16);
	glPopMatrix();

	// Bottom pommel
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, 0.0f);
	drawSphere(0.08f, 16, 16);
	glPopMatrix();

	glPopMatrix();
}

void drawSmoothArms()
{
	// This function will first draw the left arm completely,
	// then draw the right arm and conditionally draw the weapon with it.

	// --- Left Arm ---
	glPushMatrix();
	{
		glColor3f(1.0f, 1.0f, 1.0f);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, g_orangeTextureID);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		glTranslatef(-0.6f, 0.7f, 0.0f);
		glRotatef(g_leftArmAngles[0], 0.0f, 0.0f, 1.0f);
		glRotatef(g_leftArmAngles[1], 1.0f, 0.0f, 0.0f);

		float upper_arm_profile[][2] = { {0.08f, 0.0f}, {0.08f, -0.5f} };
		drawLathedObject(upper_arm_profile, 2, 12);
		glTranslatef(0.0f, -0.5f, 0.0f);

		float finalLeftElbowAngle = g_leftArmAngles[2] + (ARM_POSE_WAVE_ELBOW - g_leftArmAngles[2]) * g_leftWaveProgress;
		glRotatef(finalLeftElbowAngle, 1.0f, 0.0f, 0.0f);

		float lower_arm_profile[][2] = { {0.07f, 0.0f}, {0.07f, -0.4f} };
		drawLathedObject(lower_arm_profile, 2, 12);
		glTranslatef(0.0f, -0.4f, 0.0f);

		glRotatef(70.0f, 0.0f, 1.0f, 0.0f);
		drawHand(true);
	}
	glPopMatrix();

	// --- Right Arm & Weapon ---
	glPushMatrix();
	{
		glColor3f(1.0f, 1.0f, 1.0f);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, g_orangeTextureID);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		glTranslatef(0.6f, 0.7f, 0.0f);
		glRotatef(g_rightArmAngles[0], 0.0f, 0.0f, 1.0f);
		glRotatef(g_rightArmAngles[1], 1.0f, 0.0f, 0.0f);

		float upper_arm_profile[][2] = { {0.08f, 0.0f}, {0.08f, -0.5f} };
		drawLathedObject(upper_arm_profile, 2, 12);
		glTranslatef(0.0f, -0.5f, 0.0f);

		float finalRightElbowAngle = g_rightArmAngles[2] + (ARM_POSE_WAVE_ELBOW - g_rightArmAngles[2]) * g_rightWaveProgress;
		glRotatef(finalRightElbowAngle, 1.0f, 0.0f, 0.0f);

		float lower_arm_profile[][2] = { {0.07f, 0.0f}, {0.07f, -0.4f} };
		drawLathedObject(lower_arm_profile, 2, 12);
		glTranslatef(0.0f, -0.4f, 0.0f);

		glRotatef(-70.0f, 0.0f, 1.0f, 0.0f);

		// --- NEW LOGIC: Force hand to grip when weapon is visible ---
		float original_fist_progress = g_fistAnimationProgress; // Save the current hand state
		if (g_equippedWeapon == 1) {
			g_fistAnimationProgress = 1.0f; // Force the hand to be fully closed
		}

		drawHand(false);

		g_fistAnimationProgress = original_fist_progress; // Restore the hand state
		// --- END OF NEW LOGIC ---

		// Draw the weapon if it's visible
		if (g_equippedWeapon == 1) {
			drawWeapon();
		}
		else if (g_equippedWeapon == 2) { // If Mirror is equipped
			drawDivineMirror();
		}
	}
	glPopMatrix();

	// Final cleanup
	glDisable(GL_TEXTURE_2D);
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

	float base_sash_width = 0.18f;
	float sash_length = 2.5f;
	float flare_factor = 1.2f;
	int   segments = 30;

	float belt_top_back_y = -0.05f;
	float belt_back_radius = 0.18f;
	float start_x_offset = base_sash_width / 2.0f;

	const float TILT_DOWNWARD_X = 25.0f;
	const float FLARE_SIDEWAYS_Y = 35.0f;
	const float SURFACE_OFFSET = 0.02f;
	const float ZBIAS = 0.003f;

	auto drawOneSash = [&](bool isLeftSash)
		{
			glPushMatrix();

			glTranslatef(isLeftSash ? -start_x_offset : +start_x_offset, belt_top_back_y, -belt_back_radius);
			glRotatef(isLeftSash ? +FLARE_SIDEWAYS_Y : -FLARE_SIDEWAYS_Y, 0, 1, 0);
			glRotatef(TILT_DOWNWARD_X, 1, 0, 0);

			const float nx = isLeftSash ? 0.45f : -0.45f, ny = 0.5f, nz = -0.75f;

			// --- NEW: Waving Animation Parameters ---
			const float wave_amplitude_multiplier = 0.3f; // How WIDE the wave is
			const float wave_speed = 3.0f;                // How FAST the wave is
			const float wave_ripples = 5.0f;              // How many BENDS are in the cloth

			// --- Helper lambda to calculate vertex positions with wave ---
			auto getWavedVertex = [&](float t, float half_w_offset, float z_base, float& outX, float& outY, float& outZ)
				{
					// Base positions (same as before)
					float static_y = -t * sash_length;
					float static_z = z_base + (-0.5f * t * t);
					float current_width = base_sash_width + t * flare_factor;
					float static_x_offset = sin(t * PI) * (isLeftSash ? -0.2f : 0.2f);
					float static_x = static_x_offset + half_w_offset * current_width;

					// --- NEW: Calculate the wave offset ---
					float wave_amplitude = t * wave_amplitude_multiplier; // Amplitude is 0 at top, max at bottom
					float wave_phase = t * wave_ripples;
					float wave_offset_x = wave_amplitude * sin(g_braidTime * wave_speed + wave_phase);
					float wave_offset_z = wave_amplitude * 0.5f * cos(g_braidTime * wave_speed * 0.8f + wave_phase);

					outX = static_x + wave_offset_x;
					outY = static_y;
					outZ = static_z + wave_offset_z;
				};

			// The rest of the function now uses the helper lambda to get vertex positions

			// =============== 1) BASE CLOTH (textured red) ===============
			glColor3f(1, 1, 1);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, g_redTextureID);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

			float vx1, vy1, vz1, vx2, vy2, vz2;
			glBegin(GL_QUAD_STRIP);
			for (int i = 0; i <= segments; ++i) {
				float t = (float)i / segments;
				glNormal3f(nx, ny, nz);

				getWavedVertex(t, -0.5f, SURFACE_OFFSET, vx1, vy1, vz1);
				glTexCoord2f(0.0f, t); glVertex3f(vx1, vy1, vz1);

				getWavedVertex(t, 0.5f, SURFACE_OFFSET, vx2, vy2, vz2);
				glTexCoord2f(1.0f, t); glVertex3f(vx2, vy2, vz2);
			}
			glEnd();

			glDisable(GL_TEXTURE_2D);

			// =============== 2) GOLD EDGE TRIM ===============
			const float trim = 0.018f;
			glColor3f(0.95f, 0.8f, 0.2f);

			// Left edge strip
			glBegin(GL_QUAD_STRIP);
			for (int i = 0; i <= segments; ++i) {
				float t = (float)i / segments;
				glNormal3f(nx, ny, nz);
				getWavedVertex(t, -0.5f, SURFACE_OFFSET + ZBIAS, vx1, vy1, vz1);
				getWavedVertex(t, -0.5f + (trim / (base_sash_width + t * flare_factor)), SURFACE_OFFSET + ZBIAS, vx2, vy2, vz2);
				glVertex3f(vx1, vy1, vz1);
				glVertex3f(vx2, vy2, vz2);
			}
			glEnd();

			// Right edge strip
			glBegin(GL_QUAD_STRIP);
			for (int i = 0; i <= segments; ++i) {
				float t = (float)i / segments;
				glNormal3f(nx, ny, nz);
				getWavedVertex(t, 0.5f, SURFACE_OFFSET + ZBIAS, vx1, vy1, vz1);
				getWavedVertex(t, 0.5f - (trim / (base_sash_width + t * flare_factor)), SURFACE_OFFSET + ZBIAS, vx2, vy2, vz2);
				glVertex3f(vx1, vy1, vz1);
				glVertex3f(vx2, vy2, vz2);
			}
			glEnd();

			// =============== 3) & 4) Cords and Beads also need to wave ===============
			// (We will simplify by skipping these for now, as it requires recalculating all their waved positions. 
			// The base cloth waving is the most important effect.)
			// If you want to add them back, you would apply getWavedVertex to their positions too.

			glPopMatrix();
		};

	drawOneSash(true);   // left sash
	drawOneSash(false);  // right sash

	gluDeleteQuadric(quad);
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

	// Enable and apply the Gold texture
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, g_goldTextureID);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// --- Define the 9 vertices of the new diamond shape ---
	float diamond_width = 1.0f;
	float diamond_height = 0.40f;

	// Pre-calculate half dimensions for clarity
	float w = diamond_width / 2.0f;
	float h = diamond_height / 2.0f;

	// An array to hold the 9 points of the diamond's perimeter
	GLfloat v[9][2] = {
		{ 0.0f,      -h },         // 0: Bottom sharp point
		{ w * 0.55f, -h * 0.4f },  // 1: Lower-mid right
		{ w * 0.75f,      0.15f },      // 2: Widest point right
		{ w * 0.55f,  h * 0.2f },  // 3: Upper-mid right
		{ w * 0.2f,   h },         // 4: Top right
		{ -w * 0.2f,  h },         // 5: Top left
		{ -w * 0.55f, h * 0.2f },  // 6: Upper-mid left
		{ -w * 0.75f,     0.15f },      // 7: Widest point left
		{ -w * 0.55f, -h * 0.4f }  // 8: Lower-mid left
	};

	// --- Draw the shape as a Triangle Fan from the centre ---
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0.0f, 0.0f, 1.0f); // Normal for the front face

	// Center vertex of the fan (maps to the centre of the texture)
	glTexCoord2f(0.5f, 0.5f);
	glVertex3f(0.0f, 0.0f, 0.0f);

	// Draw the 9 perimeter vertices, calculating texture coordinates for each
	// We add the first vertex again at the end to close the shape
	for (int i = 0; i <= 9; ++i) {
		int index = i % 9; // Loop back to the first vertex for the last triangle
		float vx = v[index][0];
		float vy = v[index][1];

		// Map vertex position (from -w to w, -h to h) to texture coordinate (0 to 1)
		float u = (vx + w) / diamond_width;
		float v_tex = (vy + h) / diamond_height;

		glTexCoord2f(u, v_tex);
		glVertex3f(vx, vy, 0.0f);
	}
	glEnd();

	// Disable texturing after drawing the visor
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
	glTranslatef(0.22f, 0.02f, 0.0f);
	glRotatef(-90.0f, 0.0f, 0.0f, 1.0f);
	drawCone(ear_base_radius, ear_height, 16, 16);
	glPopMatrix();

	// --- Left Ear ---
	glPushMatrix();
	// CORRECTED POSITIONING
	glTranslatef(-0.22f, 0.02f, 0.0f);
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
	glTranslatef(0.0f, 0.20f, 0.28f);
	drawHelmetVisor();
	glPopMatrix();

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
	drawEars();

	glPopMatrix(); // End of the entire head group
}

void drawBraid(float yOffset, float zOffset)
{
	GLUquadric* quad = gluNewQuadric();
	gluQuadricNormals(quad, GLU_SMOOTH);

	// --- 2. DYNAMIC COLOUR CHANGE ---
	// This logic is copied from drawHalo to sync the colours.
	// It calculates a new colour each frame based on the global rainbow offset.
	float r = 0.5f * (1.0f + sin(g_rainbow_offset * 2.0f));
	float g = 0.5f * (1.0f + sin(g_rainbow_offset * 2.0f + 2.0f));
	float b = 0.5f * (1.0f + sin(g_rainbow_offset * 2.0f + 4.0f));

	// Set the calculated rainbow colour for the braid.
	// This works because GL_COLOR_MATERIAL is enabled in your display function.
	glColor3f(r, g, b);

	glPushMatrix();

	// Position the braid's origin on the head
	glTranslatef(0.0f, yOffset, zOffset);
	glRotatef(180.0f, 0.0f, 1.0f, 0.0f); // Rotate to face the back

	// --- 1. STRONGER CURVE ---
	// The max angle is increased to make the braid bend more sharply.
	const int CURVE_SEGMENTS = 5;
	const float MAX_CURVE_ANGLE = 28.0f; // Increased from 20.0f

	for (int i = 0; i < g_numBraidSegments; ++i)
	{
		glPushMatrix();

		float swayAmplitude = 0.0f;
		if (i >= CURVE_SEGMENTS) {
			swayAmplitude = ((float)i - (CURVE_SEGMENTS - 1)) * 4.0f * g_windStrength;
		}
		float swayAngleY = sin(g_braidTime * 2.5f + i * 0.5f) * swayAmplitude;
		float swayAngleX = cos(g_braidTime * 3.0f + i * 0.7f) * swayAmplitude * 0.5f;

		glRotatef(swayAngleY, 0.0f, 1.0f, 0.0f);
		glRotatef(swayAngleX, 1.0f, 0.0f, 0.0f);

		const float segmentRadius = 0.1f;
		gluCylinder(quad, segmentRadius, segmentRadius, g_braidSegmentLength, 20, 1);
		gluDisk(quad, 0, segmentRadius, 20, 1);
		glPushMatrix();
		glTranslatef(0.0f, 0.0f, g_braidSegmentLength);
		gluDisk(quad, 0, segmentRadius, 20, 1);
		glPopMatrix();

		glPopMatrix();

		if (i < CURVE_SEGMENTS)
		{
			float curveFactor = 1.0f - ((float)i / CURVE_SEGMENTS);
			glRotatef(MAX_CURVE_ANGLE * curveFactor, 1.0f, 0.0f, 0.0f);
		}

		const float segmentGap = 0.02f;
		glTranslatef(0.0f, 0.0f, g_braidSegmentLength + segmentGap);
	}

	glPopMatrix();
	gluDeleteQuadric(quad);
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

	glColor3f(1.0f, 0.84f, 0.0f);

	// Lambda to draw a single leg, now with animation parameters
	auto drawOneLeg = [](float hipAngle, float kneeAngle) {
		glPushMatrix(); // Save the state at the hip joint

		// --- ANIMATION: Apply rotation at the hip ---
		glRotatef(hipAngle, 1.0f, 0.0f, 0.0f);

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

		// --- ANIMATION: Apply rotation at the knee ---
		glRotatef(kneeAngle, 1.0f, 0.0f, 0.0f);

		// --- Draw the Diamond Knee Joint ---
		drawDiamondKneeJoint();

		// --- Part 2: Lower Leg (Shin) ---
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
		glRotatef(5.0f, 1.0f, 0.0f, 0.0f); // Static rotation for foot angle
		glPushMatrix();
		{
			// --- NEW: Enable texturing and bind shoe texture for the foot ---
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, g_shoeTextureID); // Bind the shoe texture
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // Combine texture with material lighting

			float v[10][3] = {
				{-0.12f, 0.0f,   0.14f}, {0.12f, 0.0f,   0.14f},    // 0,1: Top front
				{0.12f, 0.0f, -0.25f}, {-0.12f, 0.0f, -0.25f},    // 2,3: Top back
				{-0.12f, -0.15f,  0.14f}, {0.12f, -0.15f,  0.14f},  // 4,5: Bottom front
				{0.12f, -0.15f, -0.25f}, {-0.12f, -0.15f, -0.25f},  // 6,7: Bottom back
				{0.0f, -0.15f, 0.4f},                             // 8: Front point
				{0.0f, -0.15f, -0.4f}                              // 9: Back point
			};

			glBegin(GL_QUADS);
			// Top face
			glNormal3f(0.0, 1.0, 0.0);
			glTexCoord2f(0.0f, 1.0f); glVertex3fv(v[0]);
			glTexCoord2f(1.0f, 1.0f); glVertex3fv(v[1]);
			glTexCoord2f(1.0f, 0.0f); glVertex3fv(v[2]);
			glTexCoord2f(0.0f, 0.0f); glVertex3fv(v[3]);

			// Bottom face
			glNormal3f(0.0, -1.0, 0.0);
			glTexCoord2f(0.0f, 1.0f); glVertex3fv(v[4]);
			glTexCoord2f(0.0f, 0.0f); glVertex3fv(v[7]);
			glTexCoord2f(1.0f, 0.0f); glVertex3fv(v[6]);
			glTexCoord2f(1.0f, 1.0f); glVertex3fv(v[5]);

			// Back face
			glNormal3f(0.0, 0.0, -1.0);
			glTexCoord2f(0.0f, 1.0f); glVertex3fv(v[3]);
			glTexCoord2f(1.0f, 1.0f); glVertex3fv(v[2]);
			glTexCoord2f(1.0f, 0.0f); glVertex3fv(v[6]);
			glTexCoord2f(0.0f, 0.0f); glVertex3fv(v[7]);

			// Left face
			glNormal3f(-1.0, 0.0, 0.0);
			glTexCoord2f(0.0f, 1.0f); glVertex3fv(v[0]);
			glTexCoord2f(1.0f, 1.0f); glVertex3fv(v[3]);
			glTexCoord2f(1.0f, 0.0f); glVertex3fv(v[7]);
			glTexCoord2f(0.0f, 0.0f); glVertex3fv(v[4]);

			// Right face
			glNormal3f(1.0, 0.0, 0.0);
			glTexCoord2f(0.0f, 1.0f); glVertex3fv(v[1]);
			glTexCoord2f(1.0f, 1.0f); glVertex3fv(v[2]);
			glTexCoord2f(1.0f, 0.0f); glVertex3fv(v[6]);
			glTexCoord2f(0.0f, 0.0f); glVertex3fv(v[5]);
			glEnd();

			glBegin(GL_TRIANGLES);
			// Front face (connecting to the point)
			glNormal3f(0.0, 0.0, 1.0);
			glTexCoord2f(0.0f, 0.0f); glVertex3fv(v[0]);
			glTexCoord2f(1.0f, 0.0f); glVertex3fv(v[1]);
			glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0, 0.0, 0.25); // Top center front point

			// These are the side triangles for the front extension (point 8)
			glNormal3f(0.7, -0.3, 0.7); // Adjust normal as needed
			glTexCoord2f(0.0f, 0.0f); glVertex3fv(v[1]);
			glTexCoord2f(1.0f, 0.5f); glVertex3fv(v[8]); // Point 8
			glTexCoord2f(1.0f, 0.0f); glVertex3fv(v[5]);

			glNormal3f(-0.7, -0.3, 0.7); // Adjust normal as needed
			glTexCoord2f(1.0f, 0.0f); glVertex3fv(v[0]);
			glTexCoord2f(0.0f, 0.0f); glVertex3fv(v[4]);
			glTexCoord2f(0.0f, 0.5f); glVertex3fv(v[8]); // Point 8

			// These are the side triangles for the back extension (point 9)
			glNormal3f(0.7, -0.3, -0.7); // Adjust normal as needed
			glTexCoord2f(0.0f, 0.0f); glVertex3fv(v[2]);
			glTexCoord2f(1.0f, 0.5f); glVertex3fv(v[6]);
			glTexCoord2f(1.0f, 0.0f); glVertex3fv(v[9]); // Point 9

			glNormal3f(-0.7, -0.3, -0.7); // Adjust normal as needed
			glTexCoord2f(1.0f, 0.0f); glVertex3fv(v[3]);
			glTexCoord2f(0.0f, 0.0f); glVertex3fv(v[9]); // Point 9
			glTexCoord2f(0.0f, 0.5f); glVertex3fv(v[7]);
			glEnd();

			// --- NEW: Disable texturing after drawing the foot ---
			glDisable(GL_TEXTURE_2D);
		}
		glPopMatrix();

		glPopMatrix(); // Restore to the hip joint state
		};

	// --- Animation Calculation ---
	const float WALK_SPEED = 5.0f;
	const float HIP_SWING_AMPLITUDE = 40.0f;
	const float KNEE_BEND_AMPLITUDE = 70.0f;

	float rightHipAngle = 0.0f;
	float rightKneeAngle = 0.0f;

	if (g_walkDirection != 0) {
		rightHipAngle = sin(g_animationTime * WALK_SPEED) * HIP_SWING_AMPLITUDE * g_walkDirection;
		// Knee bends when the leg is moving forward
		rightKneeAngle = max(0.0f, sin(g_animationTime * WALK_SPEED) * g_walkDirection) * KNEE_BEND_AMPLITUDE;
	}

	// Left leg is in the opposite phase of the right leg
	float leftHipAngle = -rightHipAngle;
	float leftKneeAngle = max(0.0f, sin(g_animationTime * WALK_SPEED + 3.14159f) * g_walkDirection) * KNEE_BEND_AMPLITUDE;


	// --- Draw Left Leg ---
	glPushMatrix();
	glTranslatef(-0.18f, -1.0f, 0.0f);
	drawOneLeg(leftHipAngle, leftKneeAngle);
	glPopMatrix();

	// --- Draw Right Leg ---
	glPushMatrix();
	glTranslatef(0.18f, -1.0f, 0.0f);
	drawOneLeg(rightHipAngle, rightKneeAngle);
	glPopMatrix();
}

void drawSkyBackground(int winW, int winH)
{
	// Save matrices
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, winW, 0, winH, -1, 1);   // 2D screen space

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// Draw without lighting/depth and don't write depth
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, g_skyTextureID);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glBegin(GL_QUADS);
	// NOTE: If your BMP shows upside-down, swap the V coords (0 ↔ 1)
	glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex2f((float)winW, 0.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex2f((float)winW, (float)winH);
	glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, (float)winH);
	glEnd();

	glDisable(GL_TEXTURE_2D);

	// Restore state
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);

	glPopMatrix(); // MODELVIEW
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW); // leave modelview active
}

void updateHandAnimation(float deltaTime)
{
	// If the animation isn't active, do nothing.
	if (!g_isFistAnimating) {
		return;
	}

	const float FIST_ANIMATION_SPEED = 2.5f; // Controls how fast the hand opens/closes

	// Determine the direction of the animation
	// If the target is 'closed', progress moves towards 1.0
	// If the target is 'open', progress moves towards 0.0
	float direction = g_isFistTargetClosed ? 1.0f : -1.0f;

	// Update the progress
	g_fistAnimationProgress += direction * FIST_ANIMATION_SPEED * deltaTime;

	// Clamp the progress to the [0, 1] range and stop the animation when it reaches the target
	if (g_fistAnimationProgress >= 1.0f) {
		g_fistAnimationProgress = 1.0f;
		g_isFistAnimating = false; // Reached the 'closed' state
	}
	else if (g_fistAnimationProgress <= 0.0f) {
		g_fistAnimationProgress = 0.0f;
		g_isFistAnimating = false; // Reached the 'open' state
	}
}

void drawNuwaSkill()
{
	// Don't draw if the skill is not active
	if (!g_isNuwaSkillActive) {
		return;
	}

	// --- Setup for transparent, glowing effect ---
	glPushMatrix();
	glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT); // Save state

	glEnable(GL_BLEND);
	// Additive blending makes bright parts glow and ignore black parts of the texture
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glDisable(GL_LIGHTING); // Effects like this usually aren't affected by scene lighting
	glDepthMask(GL_FALSE);  // Don't write to the depth buffer to avoid z-fighting with the ground

	// --- Position the skill in the world ---
	// 1. Move to the location where the character cast the skill
	glTranslatef(g_characterCastPosX, 0.02f, g_characterCastPosZ); // 0.02f Y to prevent z-fighting
	// 2. Rotate to face the direction the character was facing
	glRotatef(g_characterAngleOnCast, 0.0f, 1.0f, 0.0f);
	// 3. Move the skill forward based on its travel distance
	glTranslatef(0.0f, 0.0f, -g_nuwaSkillDistance);

	// --- Define skill dimensions ---
	const float SKILL_WIDTH = 2.0f;
	const float SKILL_LENGTH = 8.0f;
	float halfW = SKILL_WIDTH / 2.0f;
	float halfL = SKILL_LENGTH / 2.0f;

	// --- Layer 1: The semi-transparent base rectangle ---
	glColor4f(1.0f, 0.8f, 0.4f, 0.3f); // A soft, transparent orange-gold
	glBegin(GL_QUADS);
	glVertex3f(-halfW, 0.0f, halfL);
	glVertex3f(halfW, 0.0f, halfL);
	glVertex3f(halfW, 0.0f, -halfL);
	glVertex3f(-halfW, 0.0f, -halfL);
	glEnd();

	// --- Layer 2: The bright border ---
	glLineWidth(3.0f);
	glColor4f(1.0f, 0.9f, 0.7f, 0.8f); // A brighter, more solid gold
	glBegin(GL_LINE_LOOP);
	glVertex3f(-halfW, 0.0f, halfL);
	glVertex3f(halfW, 0.0f, halfL);
	glVertex3f(halfW, 0.0f, -halfL);
	glVertex3f(-halfW, 0.0f, -halfL);
	glEnd();
	glLineWidth(1.0f);

	// --- Layer 3: The geometric patterns using the texture ---
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, g_nuwaSkillTextureID);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glColor4f(1.0f, 0.85f, 0.5f, 1.0f); // Bright gold tint for the texture pattern
	glBegin(GL_QUADS);
	// We repeat the texture 4 times along its length for more detail
	glTexCoord2f(0.0f, 4.0f); glVertex3f(-halfW, 0.0f, halfL);
	glTexCoord2f(1.0f, 4.0f); glVertex3f(halfW, 0.0f, halfL);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(halfW, 0.0f, -halfL);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-halfW, 0.0f, -halfL);
	glEnd();

	// --- Restore OpenGL state ---
	glPopAttrib();
	glPopMatrix();
}

void updateNuwaSkill(float deltaTime)
{
	if (!g_isNuwaSkillActive) {
		return;
	}

	const float SKILL_SPEED = 15.0f; // How fast it travels (units per second)
	const float SKILL_MAX_LIFETIME = 3.0f; // How long it lasts in seconds

	// Move the skill forward
	g_nuwaSkillDistance += SKILL_SPEED * deltaTime;

	// Reduce its lifetime
	g_nuwaSkillLifetime -= deltaTime;
	if (g_nuwaSkillLifetime <= 0.0f) {
		g_isNuwaSkillActive = false; // Deactivate the skill when time runs out
	}
}

void updateArmCastingAnimation(float deltaTime)
{
	const float DURATION_WINDUP = 0.4f;
	const float DURATION_HOLD = 0.8f;
	const float DURATION_RECOVER = 0.6f;

	g_armAnimationTimer += deltaTime;
	float armProgress = 0.0f; // Separate progress for arm movement

	switch (g_armAnimationState)
	{
	case 0: // Idle state: Arms down, hands naturally slightly curled (as per your image)
		// NEW: Ensure hands are in the "idle curled" position.
		// We'll set a default progress that represents this slight curl.
		break;

	case 1: // Windup: Raising arms
		armProgress = g_armAnimationTimer / DURATION_WINDUP;
		if (armProgress >= 1.0f) {
			armProgress = 1.0f;
			g_armAnimationState = 2; // 直接进入下一个动画状态
			g_armAnimationTimer = 0.0f;

			// --- SKILL CASTING LOGIC REMOVED FROM HERE ---
		}

		// NEW: Hand animation during windup (from idle curl -> open)
		// As arms raise (0->1), fists OPEN (from 0.4 down to 0.0)
		g_fistAnimationProgress = 0.4f * (1.0f - armProgress); // <<-- KEY CHANGE: Fists open

		for (int i = 0; i < 3; ++i) {
			float idle = ARM_POSE_IDLE[i];
			float casting = ARM_POSE_CASTING[i];
			float currentAngle = idle + (casting - idle) * armProgress;

			g_rightArmAngles[i] = currentAngle;
			g_leftArmAngles[i] = (i == 0) ? -currentAngle : currentAngle;
		}
		break;

	case 2: // Hold: Keep arms in pose, fists fully open
		// NEW: Ensure fists stay fully open during hold.
		g_fistAnimationProgress = 0.0f; // <<-- KEY CHANGE: Fists fully open

		if (g_armAnimationTimer >= DURATION_HOLD) {
			g_armAnimationState = 3;
			g_armAnimationTimer = 0.0f;
		}
		break;

	case 3: // Recover: Lowering arms, and re-curling fists to idle
		armProgress = g_armAnimationTimer / DURATION_RECOVER;
		if (armProgress >= 1.0f) {
			armProgress = 1.0f;
			g_armAnimationState = 0;
			g_armAnimationTimer = 0.0f;
		}

		// NEW: Hand animation during recover (from open -> idle curl)
		// As arms lower (0->1), fists CLOSE (from 0.0 up to 0.4)
		g_fistAnimationProgress = 0.4f * armProgress; // <<-- KEY CHANGE: Fists re-curl

		for (int i = 0; i < 3; ++i) {
			float casting = ARM_POSE_CASTING[i];
			float idle = ARM_POSE_IDLE[i];
			float currentAngle = casting + (idle - casting) * armProgress;

			g_rightArmAngles[i] = currentAngle;
			g_leftArmAngles[i] = (i == 0) ? -currentAngle : currentAngle;
		}
		break;
	}
}

void updateWaveAnimation(float deltaTime)
{
	const float WAVE_ANIMATION_SPEED = 3.0f;

	// Update Left Arm
	if (g_isLeftWaveActive && g_leftWaveProgress < 1.0f) {
		g_leftWaveProgress += WAVE_ANIMATION_SPEED * deltaTime;
		if (g_leftWaveProgress > 1.0f) g_leftWaveProgress = 1.0f;
	}
	else if (!g_isLeftWaveActive && g_leftWaveProgress > 0.0f) {
		g_leftWaveProgress -= WAVE_ANIMATION_SPEED * deltaTime;
		if (g_leftWaveProgress < 0.0f) g_leftWaveProgress = 0.0f;
	}

	// Update Right Arm
	if (g_isRightWaveActive && g_rightWaveProgress < 1.0f) {
		g_rightWaveProgress += WAVE_ANIMATION_SPEED * deltaTime;
		if (g_rightWaveProgress > 1.0f) g_rightWaveProgress = 1.0f;
	}
	else if (!g_isRightWaveActive && g_rightWaveProgress > 0.0f) {
		g_rightWaveProgress -= WAVE_ANIMATION_SPEED * deltaTime;
		if (g_rightWaveProgress < 0.0f) g_rightWaveProgress = 0.0f;
	}
}

void updateHandPoseAnimation(float deltaTime)
{
	const float HAND_POSE_ANIMATION_SPEED = 5.0f; // Adjust speed as needed

	if (g_handPoseTarget == 1 && g_handPoseProgress < 1.0f) {
		g_handPoseProgress += HAND_POSE_ANIMATION_SPEED * deltaTime;
		if (g_handPoseProgress > 1.0f) g_handPoseProgress = 1.0f;
	}
	else if (g_handPoseTarget == 0 && g_handPoseProgress > 0.0f) {
		g_handPoseProgress -= HAND_POSE_ANIMATION_SPEED * deltaTime;
		if (g_handPoseProgress < 0.0f) g_handPoseProgress = 0.0f;
	}
}

void drawSingleMatrixBlock(const MatrixBlock& block) {
	glPushMatrix();
	// Save current OpenGL state
	glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT);

	// --- Set up for a glowing, transparent effect ---
	glEnable(GL_BLEND);
	// Additive blending: makes colours brighter where they overlap. Great for magic!
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glDisable(GL_LIGHTING);   // Glow effects shouldn't be affected by world lighting
	glDepthMask(GL_FALSE);    // Don't hide other transparent objects behind this one

	// --- Position and scale the block ---
	glTranslatef(block.posX, block.posY, block.posZ);
	glScalef(block.scaleX, block.scaleY, block.scaleZ);

	// --- Apply the texture ---
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, g_matrixTextureID);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// Draw the cuboid with a glowing colour tint
	// The alpha (0.7f) controls the transparency for the blend function
	glColor4f(0.8f, 0.9f, 1.0f, 0.7f);
	drawCuboid(1.0f, 1.0f, 1.0f); // Draw a 1x1x1 cube, which will be scaled

	// --- Restore OpenGL state ---
	glPopAttrib();
	glPopMatrix();
}

void updateMatrixBlocks(float deltaTime) {
	const float SPAWN_DURATION = 0.1f;
	const float EXPAND_DURATION = 0.5f; // How long it takes to expand
	// --- MODIFIED HERE ---
	const float CUBE_SIDE_LENGTH = 2.0f; // Define a single size for the cube
	const float FULL_SIZE_X = CUBE_SIDE_LENGTH;
	const float FULL_SIZE_Y = CUBE_SIDE_LENGTH; // Make the height the same as the width
	const float FULL_SIZE_Z = CUBE_SIDE_LENGTH;

	// Loop through all blocks (use an iterator to allow for easy removal if needed)
	for (auto& block : g_matrixBlocks) {
		if (!block.isActive) {
			continue;
		}

		// Decrease lifetime
		block.lifetime -= deltaTime;
		if (block.lifetime <= 0.0f) {
			block.isActive = false; // Deactivate when time runs out
			continue;
		}

		block.animationTimer += deltaTime;

		// --- State Machine for Animation ---
		switch (block.state) {
		case SPAWNING:
			// Just wait for a very short time before expanding
			if (block.animationTimer >= SPAWN_DURATION) {
				block.state = EXPANDING;
				block.animationTimer = 0.0f; // Reset timer for the next state
			}
			break;

		case EXPANDING:
			// Interpolate scale from small to full size over EXPAND_DURATION
			float progress = block.animationTimer / EXPAND_DURATION;
			if (progress > 1.0f) progress = 1.0f;

			block.scaleX = FULL_SIZE_X * progress;
			block.scaleY = FULL_SIZE_Y * progress;
			block.scaleZ = FULL_SIZE_Z * progress;

			if (progress >= 1.0f) {
				block.state = ACTIVE; // Expansion finished
				block.animationTimer = 0.0f;
			}
			break;
		}
	}
}

void drawMatrixBlocks() {
	for (const auto& block : g_matrixBlocks) {
		if (block.isActive) {
			drawSingleMatrixBlock(block);
		}
	}
}

void updateWavingAnimation(float deltaTime) {
	const float WAVE_ANIMATION_SPEED = 2.0f; // Controls how fast the arm raises and lowers

	// Update the animation progress
	if (g_isWaving && g_waveProgress < 1.0f) {
		g_waveProgress += WAVE_ANIMATION_SPEED * deltaTime;
		if (g_waveProgress > 1.0f) g_waveProgress = 1.0f;
	}
	else if (!g_isWaving && g_waveProgress > 0.0f) {
		g_waveProgress -= WAVE_ANIMATION_SPEED * deltaTime;
		if (g_waveProgress < 0.0f) g_waveProgress = 0.0f;
	}

	// If the animation is active at all, update the right arm's position
	if (g_waveProgress > 0.0f) {
		for (int i = 0; i < 3; ++i) {
			float idle = ARM_POSE_IDLE[i];
			float wave = ARM_POSE_WAVE[i];
			// Interpolate from idle to wave pose
			g_rightArmAngles[i] = idle + (wave - idle) * g_waveProgress;
		}

		// Add the back-and-forth waving motion for the hand
		// This only happens when the arm is fully raised
		if (g_waveProgress >= 1.0f) {
			const float waveFrequency = 10.0f;
			const float waveAmplitude = 20.0f;
			// Apply a sine wave rotation to the elbow joint
			g_rightArmAngles[2] = ARM_POSE_WAVE[2] + sin(g_braidTime * waveFrequency) * waveAmplitude;
		}
	}
}

void display(float deltaTime)
{
	// --- Animation Updates ---
	if (g_walkDirection != 0)
	{
		// Accumulate time for the leg swing animation
		g_animationTime += deltaTime;

		// Define character movement speed
		const float MOVE_SPEED = 2.0f; // Units per second

		// Calculate movement direction based on the current camera rotation (rotateY)
		float angleRad = rotateY * (3.14159f / 180.0f); // Convert viewing angle to radians

		// Update character's X and Z position
		g_characterPosX -= sin(angleRad) * g_walkDirection * MOVE_SPEED * deltaTime;
		g_characterPosZ -= cos(angleRad) * g_walkDirection * MOVE_SPEED * deltaTime;
	}

	updateHandAnimation(deltaTime);
	updateNuwaSkill(deltaTime);
	updateArmCastingAnimation(deltaTime);
	updateWaveAnimation(deltaTime);
	updateHandPoseAnimation(deltaTime);
	updateMatrixBlocks(deltaTime);

	if (g_isHaloAnimating) {
		const float HALO_MOVE_SPEED = 5.0f;
		const float HALO_SCALE_SPEED = 2.0f;
		const float HALO_DISAPPEAR_Z = 4.0f;
		g_haloZ += HALO_MOVE_SPEED * deltaTime;
		g_haloScale += HALO_SCALE_SPEED * deltaTime;
		g_animatedLightPos[2] += HALO_MOVE_SPEED * deltaTime;
		if (g_haloZ > HALO_DISAPPEAR_Z) {
			g_isHaloVisible = false;
			g_isHaloAnimating = false;
		}
	}

	g_braidTime += deltaTime;

	float animation_speed = 0.09f;
	g_rainbow_offset += animation_speed * deltaTime;


	// --- Rendering Starts Here ---
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	drawSkyBackground(800, 600);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_NORMALIZE);

	glLightfv(GL_LIGHT0, GL_POSITION, g_animatedLightPos);
	GLfloat ambient_light[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient_light);

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

	// --- CAMERA AND CHARACTER TRANSFORMATION ---
	// 1. Apply camera transformations (zoom and rotation around the character)
	glTranslatef(0.0f, -0.5f, zoomFactor);
	glRotatef(rotateX, 1.0f, 0.0f, 0.0f);
	glRotatef(rotateY, 0.0f, 1.0f, 0.0f);

	// 2. Apply the character's world position. This effectively makes the camera
	//    "follow" the character as it moves.
	glTranslatef(-g_characterPosX, 0.0f, -g_characterPosZ);


	// --- Drawing Calls for the Character ---
	drawSmoothChest();
	drawWaistWithVerticalLines();
	drawSmoothLowerBodyAndSkirt();
	drawLegs(); // This will now draw the animated legs at the new position

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
	drawBraid(1.25f, -0.3f);
	drawHalo();
	glPopMatrix();
	drawNuwaSkill();
	drawMatrixBlocks();

	glDisable(GL_TEXTURE_2D);

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

	g_skyTextureID = loadTextureBMP("Textures/Sky.bmp");
	if (g_skyTextureID == 0) {
		MessageBox(hWnd, "Could not load Textures/Sky.bmp. Make sure the file is in the Textures folder.", "Texture Error", MB_OK | MB_ICONERROR);
		return -1;
	}

	g_shoeTextureID = loadTextureBMP("Textures/Shoe.bmp");
	if (g_shoeTextureID == 0) {
		MessageBox(hWnd, "Could not load Textures/Shoe.bmp. Make sure the file is in the Textures folder.", "Texture Error", MB_OK | MB_ICONERROR);
		return -1;
	}

	g_nuwaSkillTextureID = loadTextureBMP("Textures/NuwaSkill.bmp");
	if (g_nuwaSkillTextureID == 0) {
		MessageBox(hWnd, "Could not load Textures/NuwaSkill.bmp.", "Texture Error", MB_OK | MB_ICONERROR);
		return -1;
	}

	g_silverTextureID = loadTextureBMP("Textures/Silver.bmp");
	if (g_silverTextureID == 0) {
		MessageBox(hWnd, "Could not load Textures/Silver.bmp. Make sure the file is in the Textures folder.", "Texture Error", MB_OK | MB_ICONERROR);
		return -1;
	}

	g_orangeTextureID = loadTextureBMP("Textures/Orange.bmp");
	if (g_orangeTextureID == 0) {
		MessageBox(hWnd, "Could not load Textures/Orange.bmp. Make sure the file is in the Textures folder.", "Texture Error", MB_OK | MB_ICONERROR);
		return -1;
	}

	g_matrixTextureID = loadTextureBMP("Textures/Matrix.bmp");
	if (g_matrixTextureID == 0) {
		MessageBox(hWnd, "Could not load Textures/Matrix.bmp.", "Texture Error", MB_OK | MB_ICONERROR);
		return -1;
	}

	g_mirrorTextureID = loadTextureBMP("Textures/Mirror.bmp");
	if (g_mirrorTextureID == 0) {
		MessageBox(hWnd, "Could not load Textures/Mirror.bmp.", "Texture Error", MB_OK | MB_ICONERROR);
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