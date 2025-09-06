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

// This function draws the torso with 3D volume and proper normals for lighting.
void drawTorso3D()
{
    // Set colour to a golden yellow
    glColor3f(1.0f, 0.84f, 0.0f);

    float depth = 0.4f; // How thick the torso is
    float d = depth / 2.0f;

    // --- Upper Torso (Chest) ---
    glBegin(GL_QUADS);
    // Front Face
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-0.4f, 0.8f, d);
    glVertex3f(0.4f, 0.8f, d);
    glVertex3f(0.2f, 0.0f, d);
    glVertex3f(-0.2f, 0.0f, d);

    // Back Face
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(-0.4f, 0.8f, -d);
    glVertex3f(-0.2f, 0.0f, -d);
    glVertex3f(0.2f, 0.0f, -d);
    glVertex3f(0.4f, 0.8f, -d);

    // Top Face
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-0.4f, 0.8f, -d);
    glVertex3f(0.4f, 0.8f, -d);
    glVertex3f(0.4f, 0.8f, d);
    glVertex3f(-0.4f, 0.8f, d);

    // Right Side Face
    glNormal3f(1.0f, 0.0f, 0.0f); // Approximate normal
    glVertex3f(0.4f, 0.8f, d);
    glVertex3f(0.4f, 0.8f, -d);
    glVertex3f(0.2f, 0.0f, -d);
    glVertex3f(0.2f, 0.0f, d);

    // Left Side Face
    glNormal3f(-1.0f, 0.0f, 0.0f); // Approximate normal
    glVertex3f(-0.4f, 0.8f, d);
    glVertex3f(-0.2f, 0.0f, d);
    glVertex3f(-0.2f, 0.0f, -d);
    glVertex3f(-0.4f, 0.8f, -d);
    glEnd();

    // --- Lower Torso (Hips) ---
    glBegin(GL_QUADS);
    // Front Face
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-0.2f, 0.0f, d);
    glVertex3f(0.2f, 0.0f, d);
    glVertex3f(0.5f, -0.8f, d);
    glVertex3f(-0.5f, -0.8f, d);

    // Back Face
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(-0.2f, 0.0f, -d);
    glVertex3f(-0.5f, -0.8f, -d);
    glVertex3f(0.5f, -0.8f, -d);
    glVertex3f(0.2f, 0.0f, -d);

    // Bottom Face
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(-0.5f, -0.8f, -d);
    glVertex3f(0.5f, -0.8f, -d);
    glVertex3f(0.5f, -0.8f, d);
    glVertex3f(-0.5f, -0.8f, d);

    // Right Side Face
    glNormal3f(1.0f, 0.0f, 0.0f); // Approximate normal
    glVertex3f(0.2f, 0.0f, d);
    glVertex3f(0.2f, 0.0f, -d);
    glVertex3f(0.5f, -0.8f, -d);
    glVertex3f(0.5f, -0.8f, d);

    // Left Side Face
    glNormal3f(-1.0f, 0.0f, 0.0f); // Approximate normal
    glVertex3f(-0.2f, 0.0f, d);
    glVertex3f(-0.5f, -0.8f, d);
    glVertex3f(-0.5f, -0.8f, -d);
    glVertex3f(-0.2f, 0.0f, -d);
    glEnd();
}

// Draws the belt and buckle as 3D shapes.
void drawBelt3D() {
    // Belt Strap
    glColor3f(0.8f, 0.6f, 0.2f); // Darker gold for the belt
    glPushMatrix();
    // Scale a cuboid to be wide, short, and thin, like a belt.
    glTranslatef(0.0f, 0.0f, 0.0f);
    glScalef(2.4f, 0.5f, 2.2f); // x, y, z scaling
    drawCuboid(0.2f, 0.2f, 0.2f);
    glPopMatrix();

    // Buckle
    glColor3f(0.9f, 0.9f, 0.9f); // A lighter colour for the buckle
    glPushMatrix();
    // Position the buckle in front of the belt.
    glTranslatef(0.0f, 0.0f, 0.25f);
    glScalef(0.8f, 0.8f, 0.5f);
    drawCuboid(0.2f, 0.2f, 0.2f);
    glPopMatrix();
}

// Draws the neck using a 3D cuboid.
void drawNeck3D() {
    glColor3f(1.0f, 0.84f, 0.0f); // Gold colour

    glPushMatrix();
    // Position the neck on top of the torso.
    glTranslatef(0.0f, 0.9f, 0.0f);
    // Scale a standard cuboid to be the right shape for a neck.
    glScalef(0.2f, 0.2f, 0.2f);
    drawCuboid(1.0f, 1.0f, 1.0f); // Draw a 1x1x1 cuboid to be scaled.
    glPopMatrix();
}

// Draws the shoulder pads as 3D prisms.
void drawShoulderPads3D() {
    glColor3f(0.9f, 0.7f, 0.1f);
    float depth = 0.2f;
    float d = depth / 2.0f;

    // --- Left Shoulder Pad ---
    glPushMatrix();
    glTranslatef(-0.55f, 0.85f, 0.0f); // Position on the left shoulder.

    glBegin(GL_QUADS);
    // Define the 8 vertices of the prism shape
    float v[8][3] = {
        {-0.25f, 0.15f, d}, {0.25f, 0.15f, d}, {0.2f, -0.1f, d}, {-0.2f, -0.1f, d},
        {-0.25f, 0.15f, -d}, {0.25f, 0.15f, -d}, {0.2f, -0.1f, -d}, {-0.2f, -0.1f, -d}
    };

    // Front Face
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3fv(v[0]); glVertex3fv(v[1]); glVertex3fv(v[2]); glVertex3fv(v[3]);
    // Back Face
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3fv(v[4]); glVertex3fv(v[7]); glVertex3fv(v[6]); glVertex3fv(v[5]);
    // Top Face
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3fv(v[0]); glVertex3fv(v[4]); glVertex3fv(v[5]); glVertex3fv(v[1]);
    // Bottom Face
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3fv(v[3]); glVertex3fv(v[2]); glVertex3fv(v[6]); glVertex3fv(v[7]);
    // Right Face
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3fv(v[1]); glVertex3fv(v[5]); glVertex3fv(v[6]); glVertex3fv(v[2]);
    // Left Face
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3fv(v[0]); glVertex3fv(v[3]); glVertex3fv(v[7]); glVertex3fv(v[4]);
    glEnd();
    glPopMatrix();

    // --- Right Shoulder Pad ---
    glPushMatrix();
    glTranslatef(0.55f, 0.85f, 0.0f); // Position on the right shoulder.
    // (We can use the same shape definition)
    glBegin(GL_QUADS);
    float v2[8][3] = {
        {-0.25f, 0.15f, d}, {0.25f, 0.15f, d}, {0.2f, -0.1f, d}, {-0.2f, -0.1f, d},
        {-0.25f, 0.15f, -d}, {0.25f, 0.15f, -d}, {0.2f, -0.1f, -d}, {-0.2f, -0.1f, -d}
    };
    // Faces drawn in the same order with the same normals
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3fv(v2[0]); glVertex3fv(v2[1]); glVertex3fv(v2[2]); glVertex3fv(v2[3]);
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3fv(v2[4]); glVertex3fv(v2[7]); glVertex3fv(v2[6]); glVertex3fv(v2[5]);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3fv(v2[0]); glVertex3fv(v2[4]); glVertex3fv(v2[5]); glVertex3fv(v2[1]);
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3fv(v2[3]); glVertex3fv(v2[2]); glVertex3fv(v2[6]); glVertex3fv(v2[7]);
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3fv(v2[1]); glVertex3fv(v2[5]); glVertex3fv(v2[6]); glVertex3fv(v2[2]);
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3fv(v2[0]); glVertex3fv(v2[3]); glVertex3fv(v2[7]); glVertex3fv(v2[4]);
    glEnd();
    glPopMatrix();
}

// Draws the arms and hands
void drawArms()
{
    glColor3f(1.0f, 0.84f, 0.0f); // Gold colour

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

// Draws the hip guards as 3D shapes.
void drawHipGuards3D() {
    glColor3f(0.8f, 0.6f, 0.2f); // Darker gold

    // Left Hip Guard
    glPushMatrix();
    glTranslatef(-0.5f, -0.6f, 0.0f); // Position
    glRotatef(20.0f, 0.0f, 0.0f, 1.0f); // Rotate
    glScalef(0.4f, 0.4f, 0.2f); // Scale to size
    drawCuboid(1.0f, 1.0f, 1.0f);
    glPopMatrix();

    // Right Hip Guard
    glPushMatrix();
    glTranslatef(0.5f, -0.6f, 0.0f); // Position
    glRotatef(-20.0f, 0.0f, 0.0f, 1.0f); // Rotate
    glScalef(0.4f, 0.4f, 0.2f); // Scale to size
    drawCuboid(1.0f, 1.0f, 1.0f);
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