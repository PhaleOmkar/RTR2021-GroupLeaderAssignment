// Header File Declaration
#include "../headers/OpenGLFiles.h"
#include "../headers/Resources.h"

#include "../headers/MyShapes.h"

// starfield //
struct header
{
	unsigned char identifier[12];
	unsigned int endianness;
	unsigned int gltype;
	unsigned int gltypesize;
	unsigned int glformat;
	unsigned int glinternalformat;
	unsigned int glbaseinternalformat;
	unsigned int pixelwidth;
	unsigned int pixelheight;
	unsigned int pixeldepth;
	unsigned int arrayelements;
	unsigned int faces;
	unsigned int miplevels;
	unsigned int keypairbytes;
};

enum
{
	NUMBER_OF_STARS = 5000
};

struct
{
	GLint time;

	GLint model_view_matrix;
	GLint projection_matrix;
} starfield_uniforms_t;

// Global Function Declaration
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Global Macro Definitions
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define NUMBER_OF_TEXTURES 15 // With Pluto and Charon - 17

#define STACK_SIZE 20
#define STACK_TYPE mat4

// Pragma Declaration
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "MyShapes.lib")

using namespace vmath;

enum
{
	OUP_ATTRIBUTE_POSITION = 0,
	OUP_ATTRIBUTE_COLOR,
	OUP_ATTRIBUTE_NORMAL,
	OUP_ATTRIBUTE_TEXTURE
};

enum
{
	SUN = 0,
	MERCURY = 1,
	VENUS = 2,
	EARTH = 3,
	MOON = 4,
	MARS = 5,
	PHOBOS = 6,
	JUPITER = 7,
	EUROPA = 8,
	SATURN = 9,
	TITAN = 10,
	URANUS = 11,
	ARIEL = 12,
	NEPTUNE = 13,
	TRITON = 14,
	// PLUTO  = 15, // UNUSED
	// CHARON  = 16 // UNUSED
};

// Global Variable Declaration
DWORD dwStyle;
WINDOWPLACEMENT wpPrev = {sizeof(WINDOWPLACEMENT)};

HWND ghwnd = NULL;
HDC ghdc = NULL;
HGLRC ghrc = NULL;

FILE *gpFile;

bool gbFullScreen = false;
bool gbActiveWindow = false;
bool gbUpdateRendering = true;

GLuint windowWidth;
GLuint windowHeight;

// mouse based camera //
vec3 camera_pos(0.0f, 0.0f, 490.0f);
vec3 camera_front(0.0f, 0.0f, 0.0f);
vec3 camera_up(0.0f, 1.0f, 0.0f);

float camera_speed = 0.15f;
float sensitivity = 0.1f;

bool first_mouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float last_x = 1920.0f / 2.0;
float last_y = 1080.0f / 2.0f;
float fov = 45.0f;

// starfield //
GLuint vertexShaderObjectStarfield;
GLuint fragmentShaderObjectStarfield;
GLuint shaderProgramObjectStarfield;

GLuint starfield_vao;
GLuint starfield_vbo;
GLuint star_texture;
GLuint textureSamplerUniformKTX;
GLfloat timeValue = 1.0;

GLfloat rotationAngle;
GLfloat ANGLE_INCREMENT_VALUE = 0.05f;	// For Intel 0.1f
GLfloat TIME_INCREMENT_VALUE = 0.00025f; // For Intel 0.00025f

// SPHERE //
GLuint vertexShaderObjectSphere;
GLuint fragmentShaderObjectSphere;
GLuint shaderProgramObjectSphere;

GLuint vao_sphere;
GLuint vbo_element_sphere;
GLuint vbo_position_sphere;
GLuint vbo_normal_sphere;
GLuint vbo_texture_sphere;

GLuint modelViewMatrixUniformPlanet;
GLuint viewMatrixUniformPlanet;
GLuint projectionMatrixUniformPlanet;
GLuint textureSamplerUniformPlanets;

std::vector<float> param_sphere_vertices;
std::vector<float> param_sphere_normals;
std::vector<float> param_sphere_texCoords;
std::vector<unsigned int> param_sphere_indices;
std::vector<unsigned int> param_sphere_line_indices;

GLuint gluiTextureImage[NUMBER_OF_TEXTURES];

// ELLIPTICAL ORBITS //
GLuint gVertexShaderObject;
GLuint gFragmentShaderObject;
GLuint gShaderProgramObject;

GLuint mvpMatrixUniform;

GLuint vao_ellipse;
GLuint vbo_position_ellipse;

GLfloat mercury_value = 0.0f;
GLfloat venus_value = 0.0f;
GLfloat earth_value = 0.0f;
GLfloat mars_value = 0.0f;
GLfloat jupiter_value = 0.0f;
GLfloat saturn_value = 0.0f;
GLfloat uranus_value = 0.0f;
GLfloat neptune_value = 0.0f;
GLfloat POSITION_INCREMENT_VALUE = 0.7f; // For NVIDIA 1.0f; // For Intel 0.25f

// GLfloat mercury_value = 272.0f;
// GLfloat venus_value = 348.0f;
// GLfloat earth_value = 440.0f;
// GLfloat mars_value = 520.0f;
// GLfloat jupiter_value = 630.0f;
// GLfloat saturn_value = 750.0f;
// GLfloat uranus_value = 850.0f;
// GLfloat neptune_value = 950.0f;
// GLfloat POSITION_INCREMENT_VALUE = 0.15f;

bool gbAllPlanetsReached = false;
GLfloat ellipseVerticesPath[6284][3];
int position_index[14];

// STACK //
int stack_top = -1;
STACK_TYPE stack_array[STACK_SIZE];

mat4 perspectiveProjectionMatrix;

// WinMain() Defintion
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	// Local Funtion Declaration
	void ToggleFullScreen(void);
	void Initialize(void);
	void Display(void);
	void Update(void);

	// Local Variable Declaration
	WNDCLASSEX wndclassex;
	HWND hwnd;
	MSG message;
	TCHAR szAppName[] = TEXT("RTR2021");
	RECT rect;
	BOOL bResult;
	int iCenterX = 0;
	int iCenterY = 0;
	bool bDone = false;

	// Code
	// Error Checking Of 'fopen_s()'
	if (fopen_s(&gpFile, "CodeExecutionLog.txt", "w") != 0)
	{
		MessageBox(NULL,
				   TEXT("Cannot open desired file"),
				   TEXT("Message::fopen_s() failed"),
				   MB_ICONERROR);
		exit(0);
	}

	// Getting System Parameters Info
	bResult = SystemParametersInfo(
		SPI_GETWORKAREA,
		0,
		&rect,
		0);

	// Error Checking :: bResult
	if (bResult == TRUE)
	{
		iCenterX = ((int)(rect.left + rect.right) - (int)(WINDOW_WIDTH)) / 2;
		iCenterY = ((int)(rect.top + rect.bottom) - (int)(WINDOW_HEIGHT)) / 2;
	}

	// Initialization Of WNDCLASSEX
	wndclassex.cbSize = sizeof(WNDCLASSEX);
	wndclassex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclassex.cbClsExtra = 0;
	wndclassex.cbWndExtra = 0;
	wndclassex.lpfnWndProc = WndProc;
	wndclassex.hInstance = hInstance;
	wndclassex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(DEFAULT_LARGE_ICON));
	wndclassex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclassex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclassex.lpszClassName = szAppName;
	wndclassex.lpszMenuName = NULL;
	wndclassex.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(DEFAULT_LARGE_ICON));

	// Register Above Class
	RegisterClassEx(&wndclassex);

	// Create Window
	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
						  szAppName,
						  TEXT("Omkar Phale (RTR2021)"),
						  WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
						  iCenterX,
						  iCenterY,
						  (int)WINDOW_WIDTH,
						  (int)WINDOW_HEIGHT,
						  NULL,
						  NULL,
						  hInstance,
						  NULL);

	// Copying Window Handle To Global Handle
	ghwnd = hwnd;

	// Initialize() Called
	Initialize();

	// Show Window
	ShowWindow(hwnd, iCmdShow);

	// Default Fullscreen
	ToggleFullScreen();

	// Setting ForeGroundWindow and Focus
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	// Game Loop
	while (bDone == false)
	{
		if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
		{
			if (message.message == WM_QUIT)
			{
				bDone = true;
			}

			else
			{
				TranslateMessage(&message);
				DispatchMessage(&message);
			}
		}

		else
		{
			if (gbActiveWindow == true)
			{
				if (gbUpdateRendering == true)
				{
					// Update() :: OpenGL rendering should be called
					Update();
				}

				// Display() :: OpenGL rendering should be called
				Display();
			}
		}
	}

	return ((int)message.wParam);
}

// WndProc() Definition
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	// Local Function Declaration
	void ToggleFullScreen(void);
	void Resize(int, int);
	void Uninitialize(void);
	void MouseMove(double xPosition, double yPosition);

	// Code
	switch (iMessage)
	{
	case WM_SETFOCUS:
		gbActiveWindow = true;
		break;

	case WM_KILLFOCUS:
		gbActiveWindow = false;
		break;

	case WM_ERASEBKGND:
		return (0);

	case WM_SIZE:
		windowWidth = LOWORD(lParam);
		windowHeight = HIWORD(lParam);

		Resize(LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			DestroyWindow(hwnd);
			break;

		case VK_UP:
			TIME_INCREMENT_VALUE += 0.001;
			break;

		case VK_DOWN:
			TIME_INCREMENT_VALUE -= 0.001;
			break;

		case 0x46:
		case 0x66:
			ToggleFullScreen();
			break;

		case 0x50:
		case 0x80:
			gbUpdateRendering = gbUpdateRendering ^ 1;
			break;

		default:
			break;
		}
		break;

	case WM_CHAR:
		switch (wParam)
		{
		default:
			break;
		}
		break;

	case WM_MOUSEMOVE:
		MouseMove(LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;

	case WM_DESTROY:
		Uninitialize();
		PostQuitMessage(0);
		break;
	}

	return (DefWindowProc(hwnd, iMessage, wParam, lParam));
}

// ToggleFullScreen() Definition
void ToggleFullScreen(void)
{
	// Local Variable Declarations
	MONITORINFO monitorinfo = {sizeof(MONITORINFO)};

	// Code
	if (gbFullScreen == false)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);

		if (dwStyle & WS_OVERLAPPEDWINDOW)
		{
			if (GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &monitorinfo))
			{
				SetWindowLong(ghwnd, GWL_STYLE, (dwStyle & ~WS_OVERLAPPEDWINDOW));
				SetWindowPos(ghwnd,
							 HWND_TOP,
							 monitorinfo.rcMonitor.left,
							 monitorinfo.rcMonitor.top,
							 (monitorinfo.rcMonitor.right - monitorinfo.rcMonitor.left),
							 (monitorinfo.rcMonitor.bottom - monitorinfo.rcMonitor.top),
							 SWP_NOZORDER | SWP_FRAMECHANGED);
			}
		}

		// ShowCursor(FALSE);
		gbFullScreen = true;
	}

	else
	{
		SetWindowLong(ghwnd, GWL_STYLE, (dwStyle | WS_OVERLAPPEDWINDOW));
		SetWindowPlacement(ghwnd, &wpPrev);
		SetWindowPos(ghwnd,
					 HWND_TOP,
					 0,
					 0,
					 0,
					 0,
					 SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);
		// ShowCursor(TRUE);
		gbFullScreen = false;
	}
}

// MouseMove() Definition
void MouseMove(double x_position, double y_position)
{
	// Code
	if (first_mouse)
	{
		last_x = x_position;
		last_y = y_position;

		first_mouse = false;
	}

	float xoffset = x_position - last_x;
	float yoffset = last_y - y_position;

	last_x = x_position;
	last_y = y_position;

	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 90.0f)
	{
		pitch = 90.0f;
	}

	if (pitch < -90.0f)
	{
		pitch = -90.0f;
	}

	static vec3 front;
	front[0] = cos(vmath::radians(yaw)) * cos(vmath::radians(pitch));
	front[1] = sin(vmath::radians(pitch));
	front[2] = sin(vmath::radians(yaw)) * cos(vmath::radians(pitch));

	camera_front = vmath::normalize(front);
}

// Initialize() Definition
void Initialize(void)
{
	// Local Function Declaration
	void initialize_starfield(void);
	void initialize_solar_system(void);

	void Resize(int, int);
	void SphereRendering(const float radius);
	bool LoadGLTexture(GLuint * gluiTexture, TCHAR szResourceID[]);
	void PrintingOpenGLRelatedSpecificationLog(void);

	// Local Variable Declaration
	PIXELFORMATDESCRIPTOR pixelformatdescriptor;
	int iPixelFormatIndex;

	// Code
	ghdc = GetDC(ghwnd);
	ZeroMemory(&pixelformatdescriptor, sizeof(PIXELFORMATDESCRIPTOR));

	// Initialization of PIXELFORMATDESCRIPTOR
	pixelformatdescriptor.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pixelformatdescriptor.nVersion = 1;
	pixelformatdescriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pixelformatdescriptor.iPixelType = PFD_TYPE_RGBA;
	pixelformatdescriptor.cColorBits = 32;
	pixelformatdescriptor.cRedBits = 8;
	pixelformatdescriptor.cGreenBits = 8;
	pixelformatdescriptor.cBlueBits = 8;
	pixelformatdescriptor.cAlphaBits = 8;
	pixelformatdescriptor.cDepthBits = 32;

	iPixelFormatIndex = ChoosePixelFormat(ghdc, &pixelformatdescriptor);

	if (iPixelFormatIndex == 0)
	{
		fprintf(gpFile, "ChoosePixelFormat() failed...\n");
		DestroyWindow(ghwnd);
	}

	if (SetPixelFormat(ghdc, iPixelFormatIndex, &pixelformatdescriptor) == FALSE)
	{
		fprintf(gpFile, "SetPixelFormat() failed...\n");
		DestroyWindow(ghwnd);
	}

	ghrc = wglCreateContext(ghdc);
	if (ghrc == NULL)
	{
		fprintf(gpFile, "wglCreateContext() failed...\n");
		DestroyWindow(ghwnd);
	}

	if (wglMakeCurrent(ghdc, ghrc) == FALSE)
	{
		fprintf(gpFile, "wglMakeCurrent() failed...\n");
		DestroyWindow(ghwnd);
	}

	// Printing OpenGL Related Specification Log
	PrintingOpenGLRelatedSpecificationLog();

	initialize_starfield();
	initialize_solar_system();

	// Local Variable Declaration
	GLint iInfoLogLength = 0;
	GLint iShaderCompiledStatus = 0;
	GLint iShaderLinkerStatus = 0;
	GLchar *szInfoLogBuffer = NULL;

	/////////////////////////////////////////////////////
	// Initialize Of Elliptical Path Started From Here //
	/////////////////////////////////////////////////////
	// Vertex Shader
	// Creating Shader
	gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
	const GLchar *pglcVertexShaderSourceCode =
		"#version 430 core										\n"
		"														\n"
		"in vec4 vPosition;										\n"
		"														\n"
		"uniform mat4 u_mvpMatrix;								\n"
		"														\n"
		"void main()											\n"
		"{														\n"
		"	gl_Position = u_mvpMatrix * vPosition;				\n"
		"}														\n";

	glShaderSource(gVertexShaderObject, 1, (const GLchar **)&pglcVertexShaderSourceCode, NULL);

	// Compiling Shader
	glCompileShader(gVertexShaderObject);

	// Error Checking
	glGetShaderiv(gVertexShaderObject, GL_COMPILE_STATUS, &iShaderCompiledStatus);
	if (iShaderCompiledStatus == GL_FALSE)
	{
		glGetShaderiv(gVertexShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLogBuffer = (GLchar *)malloc(iInfoLogLength * sizeof(GLchar));
			if (szInfoLogBuffer != NULL)
			{
				GLsizei written;

				glGetShaderInfoLog(gVertexShaderObject, iInfoLogLength, &written, szInfoLogBuffer);

				fprintf(gpFile, "\n[Vertex Shader Compilation Error Log : %s]\n", szInfoLogBuffer);
				free(szInfoLogBuffer);
				DestroyWindow(ghwnd);
			}
		}
	}

	// Fragment Shader
	// Creating Shader
	gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	const GLchar *pglcFragmentShaderSourceCode =
		"#version 430 core										\n"
		"														\n"
		"out vec4 FragmentColor;								\n"
		"														\n"
		"void main()											\n"
		"{														\n"
		"	vec4 color = vec4(0.4, 0.4, 0.4, 1.0);				\n"
		"	FragmentColor = color;								\n"
		"}														\n";

	glShaderSource(gFragmentShaderObject, 1, (const GLchar **)&pglcFragmentShaderSourceCode, NULL);

	// Compiling Shader
	glCompileShader(gFragmentShaderObject);

	// Error Checking
	glGetShaderiv(gFragmentShaderObject, GL_COMPILE_STATUS, &iShaderCompiledStatus);
	if (iShaderCompiledStatus == GL_FALSE)
	{
		glGetShaderiv(gFragmentShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLogBuffer = (GLchar *)malloc(iInfoLogLength * sizeof(GLchar));
			if (szInfoLogBuffer != NULL)
			{
				GLsizei written;

				glGetShaderInfoLog(gFragmentShaderObject, iInfoLogLength, &written, szInfoLogBuffer);

				fprintf(gpFile, "\n[Fragment Shader Compilation Error Log : %s]\n", szInfoLogBuffer);
				free(szInfoLogBuffer);
				DestroyWindow(ghwnd);
			}
		}
	}

	// Shader Program - Create Shader Program
	gShaderProgramObject = glCreateProgram();

	glAttachShader(gShaderProgramObject, gVertexShaderObject);	 //Attach Vertex Shader To Shader Program
	glAttachShader(gShaderProgramObject, gFragmentShaderObject); //Attach Fragment Shader To Shader Program

	// Bind Vertex Shader Position Attribute
	glBindAttribLocation(gShaderProgramObject, OUP_ATTRIBUTE_POSITION, "vPosition");

	// Link Shader Program
	glLinkProgram(gShaderProgramObject);

	// Error Checking
	glGetProgramiv(gShaderProgramObject, GL_LINK_STATUS, &iShaderLinkerStatus);
	if (iShaderLinkerStatus == GL_FALSE)
	{
		glGetShaderiv(gShaderProgramObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLogBuffer = (GLchar *)malloc(iInfoLogLength * sizeof(GLchar));
			if (szInfoLogBuffer != NULL)
			{
				GLsizei written;

				glGetShaderInfoLog(gShaderProgramObject, iInfoLogLength, &written, szInfoLogBuffer);

				fprintf(gpFile, "\n[Shader Program Linking Error Log : %s]\n", szInfoLogBuffer);
				free(szInfoLogBuffer);
				DestroyWindow(ghwnd);
			}
		}
	}

	// Get Uniform Location
	mvpMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_mvpMatrix");

	glGenVertexArrays(1, &vao_ellipse);
	glBindVertexArray(vao_ellipse);
	// vbo_position_ellipse
	glGenBuffers(1, &vbo_position_ellipse);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_position_ellipse);
	glBufferData(GL_ARRAY_BUFFER, NULL, NULL, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(OUP_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(OUP_ATTRIBUTE_POSITION);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	//////////////////////////////////////////////////
	// Initialize Of Elliptical Path Completed Here //
	//////////////////////////////////////////////////

	// Depth Settings
	glClearDepth(1.0f);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	// glEnable(GL_CULL_FACE);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glBlendFunc(GL_ONE, NULL);
	glEnable(GL_BLEND);

	// Set Clear Color
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	perspectiveProjectionMatrix = mat4::identity();

	Resize(WINDOW_WIDTH, WINDOW_HEIGHT); // Warm Up Call To Resize()
}

// PrintingOpenGLRelatedSpecificationLog() Definition
void PrintingOpenGLRelatedSpecificationLog(void)
{
	// Local Variable Declaration
	int iIndex;
	char *pcDayOfTheWeek = NULL;

	SYSTEMTIME stSystemTime;
	GLenum glew_error;
	GLint numberOfExtensions;

	// Code
	glew_error = glewInit();
	if (glew_error != GLEW_OK)
	{
		DestroyWindow(ghwnd);
	}

	// OpenGL Related Specification Log
	ZeroMemory(&stSystemTime, sizeof(stSystemTime));

	GetLocalTime(&stSystemTime);
	switch (stSystemTime.wDayOfWeek)
	{
	case 0:
		pcDayOfTheWeek = "Sunday";
		break;

	case 1:
		pcDayOfTheWeek = "Monday";
		break;

	case 2:
		pcDayOfTheWeek = "Tuesday";
		break;

	case 3:
		pcDayOfTheWeek = "Wednesday";
		break;

	case 4:
		pcDayOfTheWeek = "Thursday";
		break;

	case 5:
		pcDayOfTheWeek = "Friday";
		break;

	case 6:
		pcDayOfTheWeek = "Saturday";
		break;

	default:
		break;
	}

	fprintf(gpFile, "[================= OpenGL Related Specification Log : [%s - %2.2d/%2.2d/%4d, %2.2d:%2.2d:%2.2d:%2.3d] =================]\n",
			pcDayOfTheWeek, stSystemTime.wDay, stSystemTime.wMonth, stSystemTime.wYear, stSystemTime.wHour, stSystemTime.wMinute, stSystemTime.wSecond, stSystemTime.wMilliseconds);
	fprintf(gpFile, " ## OpenGL Vendor   : %s\n", glGetString(GL_VENDOR));
	fprintf(gpFile, " ## OpenGL Renderer : %s\n", glGetString(GL_RENDERER));
	fprintf(gpFile, " ## OpenGL Version  : %s\n", glGetString(GL_VERSION));
	fprintf(gpFile, " ## GLSL Version    : %s\n\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	// OpenGL Enable Extensions
	glGetIntegerv(GL_NUM_EXTENSIONS, &numberOfExtensions);

	fprintf(gpFile, " ## Number of GLSL extensions = %d\n", numberOfExtensions);
	for (iIndex = 0; iIndex < numberOfExtensions; iIndex++)
	{
		fprintf(gpFile, " %3.3d] %s\n", (iIndex + 1), glGetStringi(GL_EXTENSIONS, iIndex));
	}

	fprintf(gpFile, "[====================================== End Of OpenGL Log ======================================]\n\n");
}

// LoadGLTexture() Definition
bool LoadGLTexture(GLuint *gluiTexture, TCHAR szResourceID[])
{
	// Local Variable Declaration
	bool bResult = false;
	HBITMAP hBitmap = NULL;
	BITMAP bmp;

	// Code
	hBitmap = (HBITMAP)LoadImage(GetModuleHandle(NULL), szResourceID, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	if (hBitmap)
	{
		bResult = true;
		GetObject(hBitmap, sizeof(BITMAP), &bmp);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glGenTextures(1, gluiTexture);
		glBindTexture(GL_TEXTURE_2D, *gluiTexture);

		// Setting Texture Parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		// Actually Pushing The Data To Memory With The Help Of Graphic Driver
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bmp.bmWidth, bmp.bmHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, bmp.bmBits);
		glGenerateMipmap(GL_TEXTURE_2D); // gluBuild2DMipmaps(GL_TEXTURE_2D, 3, bmp.bmWidth, bmp.bmHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, bmp.bmBits);

		DeleteObject(hBitmap);
	}

	return (bResult);
}

// Resize() Definition
void Resize(int iWidth, int iHeight)
{
	// Code
	if (iHeight == 0)
	{
		iHeight = 1;
	}

	glViewport(0, 0, (GLsizei)iWidth, (GLsizei)iHeight);

	perspectiveProjectionMatrix = perspective(45.0f,
											  (GLfloat)iWidth / (GLfloat)iHeight,
											  0.1f,
											  1000.0f);
}

// Display() Definition
void Display(void)
{
	// Local Function Declaration
	void display_solar_system(void);

	// Code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	display_solar_system();

	SwapBuffers(ghdc);
}

// Update() Definition
void Update(void)
{
	// Local Function Declaration
	void update_starfield(void);
	void update_solar_system(void);

	// Local Variable Declaration
	static bool bIsBGMPlaying = false;

	// Code
	if (bIsBGMPlaying == false)
	{
		PlaySound(MAKEINTRESOURCE(SEA_OF_TIME_BGM_WAVE), NULL, SND_ASYNC | SND_RESOURCE);
		bIsBGMPlaying = true;
	}

	rotationAngle = rotationAngle + ANGLE_INCREMENT_VALUE;

	update_starfield();
	update_solar_system();
}

// Unintialize() Definition
void Uninitialize(void)
{
	// Local Function Declaration
	void uninitialize_starfield(void);
	void uninitialize_solar_system(void);

	// Code
	if (gbFullScreen == true)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);

		SetWindowLong(ghwnd, GWL_STYLE, (dwStyle | WS_OVERLAPPEDWINDOW));
		SetWindowPlacement(ghwnd, &wpPrev);
		SetWindowPos(ghwnd,
					 HWND_TOP,
					 0,
					 0,
					 0,
					 0,
					 SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);

		ShowCursor(TRUE);
	}

	if (wglGetCurrentContext() == ghrc)
	{
		wglMakeCurrent(NULL, NULL);
	}

	if (ghrc)
	{
		wglDeleteContext(ghrc);
		ghrc = NULL;
	}

	if (ghdc)
	{
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	uninitialize_starfield();
	uninitialize_solar_system();

	// Local Variable Declaration
	GLsizei ShaderCount;
	GLsizei index;
	GLuint *pShaders = NULL;

	if (vao_ellipse)
	{
		glDeleteBuffers(1, &vao_ellipse);
		vao_ellipse = 0;
	}

	if (vbo_position_ellipse)
	{
		glDeleteBuffers(1, &vbo_position_ellipse);
		vbo_position_ellipse = 0;
	}

	if (gShaderProgramObject)
	{
		//Safe Shader Cleaup
		glUseProgram(gShaderProgramObject);

		glGetProgramiv(gShaderProgramObject, GL_ATTACHED_SHADERS, &ShaderCount);
		pShaders = (GLuint *)malloc(sizeof(GLuint) * ShaderCount);
		//TODO : Error Checking

		glGetAttachedShaders(gShaderProgramObject, ShaderCount, &ShaderCount, pShaders);
		for (index = 0; index < ShaderCount; index++)
		{
			glDetachShader(gShaderProgramObject, pShaders[index]);
			glDeleteShader(pShaders[index]);
			pShaders[index] = 0;
		}

		free(pShaders);

		glDeleteShader(gShaderProgramObject);
		gShaderProgramObject = 0;

		glUseProgram(0);
	}

	// Closing 'gpFile'
	if (gpFile)
	{
		fclose(gpFile);
		gpFile = NULL;
	}
}

// initialize_solar_system() Definition
void initialize_solar_system(void)
{
	//Local Variable Declaration
	GLint iInfoLogLength = 0;
	GLint iShaderCompiledStatus = 0;
	GLchar *szInfoLogBuffer = NULL;

	//Vertex Shader -  For Sphere - Vertices + Texture + Lights (Per Fragment)
	//Creating Shader
	vertexShaderObjectSphere = glCreateShader(GL_VERTEX_SHADER);
	const GLchar *pglcVertexShaderSourceCode =
		"#version 430 core																						        \n"
		"																										        \n"
		"in vec4 vPosition;																						        \n"
		"in vec3 vNormal;																						        \n"
		"in vec2 vTexCoord;																						        \n"
		"																										        \n"
		"uniform mat4 u_model_view_matrix;																			    \n"
		"uniform mat4 u_projection_matrix;																		        \n"
		"out vec2 out_vTexCoord;																				        \n"
		"																										        \n"
		"void main(void)																						        \n"
		"{																										        \n"
		"	out_vTexCoord = vTexCoord;																			        \n"
		"																										        \n"
		"	gl_Position = u_projection_matrix * u_model_view_matrix * vPosition;						        		\n"
		"}																										        \n";

	glShaderSource(vertexShaderObjectSphere, 1, (const GLchar **)&pglcVertexShaderSourceCode, NULL);

	//Compiling Shader
	glCompileShader(vertexShaderObjectSphere);

	//Error Checking
	glGetShaderiv(vertexShaderObjectSphere, GL_COMPILE_STATUS, &iShaderCompiledStatus);
	if (iShaderCompiledStatus == GL_FALSE)
	{
		glGetShaderiv(vertexShaderObjectSphere, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLogBuffer = (GLchar *)malloc(iInfoLogLength * sizeof(GLchar));
			if (szInfoLogBuffer != NULL)
			{
				GLsizei written;

				glGetShaderInfoLog(vertexShaderObjectSphere, iInfoLogLength, &written, szInfoLogBuffer);

				fprintf(gpFile, "%s : \n[Vertex Shader Compilation Error Log : %s]\n", __FILE__, szInfoLogBuffer);
				free(szInfoLogBuffer);
				DestroyWindow(ghwnd);
			}
		}
	}

	iInfoLogLength = 0;
	iShaderCompiledStatus = 0;
	szInfoLogBuffer = NULL;

	//Fragment Shader -  For Sphere - Vertices + Texture + Lights (Per Fragment)
	//Creating Shader
	fragmentShaderObjectSphere = glCreateShader(GL_FRAGMENT_SHADER);

	const GLchar *pglcFragmentShaderSourceCode =
		"#version 430 core																												\n"
		"																																\n"
		"uniform sampler2D u_Texture_Sampler;																							\n"
		"																																\n"
		"in vec2 out_vTexCoord;																											\n"
		"out vec4 FragmentColor;																										\n"
		"																																\n"
		"void main(void)																												\n"
		"{																																\n"
		"   vec4 color;                                                                                                                 \n"
		"   color = texture(u_Texture_Sampler, out_vTexCoord);								                                        	\n"
		"																																\n"
		"	FragmentColor = color;	                        								                                            \n"
		"}																																\n";

	glShaderSource(fragmentShaderObjectSphere, 1, (const GLchar **)&pglcFragmentShaderSourceCode, NULL);

	//Compiling Shader
	glCompileShader(fragmentShaderObjectSphere);

	//Error Checking
	glGetShaderiv(fragmentShaderObjectSphere, GL_COMPILE_STATUS, &iShaderCompiledStatus);
	if (iShaderCompiledStatus == GL_FALSE)
	{
		glGetShaderiv(fragmentShaderObjectSphere, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLogBuffer = (GLchar *)malloc(iInfoLogLength * sizeof(GLchar));
			if (szInfoLogBuffer != NULL)
			{
				GLsizei written;

				glGetShaderInfoLog(fragmentShaderObjectSphere, iInfoLogLength, &written, szInfoLogBuffer);

				fprintf(gpFile, "\n[Fragment Shader Compilation Error Log : %s]\n", szInfoLogBuffer);
				free(szInfoLogBuffer);
				DestroyWindow(ghwnd);
			}
		}
	}

	iInfoLogLength = 0;
	GLint iShaderLinkerStatus = 0;
	szInfoLogBuffer = NULL;

	//Shader Program -  For Sphere - Vertices + Texture + Lights (Per Fragment)
	//Create Shader Program
	shaderProgramObjectSphere = glCreateProgram();

	glAttachShader(shaderProgramObjectSphere, vertexShaderObjectSphere);   //Attach Vertex Shader To Shader Program
	glAttachShader(shaderProgramObjectSphere, fragmentShaderObjectSphere); //Attach Fragment Shader To Shader Program

	//Bind Vertex Shader Position Attribute
	glBindAttribLocation(shaderProgramObjectSphere, OUP_ATTRIBUTE_POSITION, "vPosition");
	glBindAttribLocation(shaderProgramObjectSphere, OUP_ATTRIBUTE_TEXTURE, "vTexCoord");

	//Link Shader Program
	glLinkProgram(shaderProgramObjectSphere);

	//Error Checking
	glGetProgramiv(shaderProgramObjectSphere, GL_LINK_STATUS, &iShaderLinkerStatus);
	if (iShaderLinkerStatus == GL_FALSE)
	{
		glGetShaderiv(shaderProgramObjectSphere, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLogBuffer = (GLchar *)malloc(iInfoLogLength * sizeof(GLchar));
			if (szInfoLogBuffer != NULL)
			{
				GLsizei written;

				glGetShaderInfoLog(shaderProgramObjectSphere, iInfoLogLength, &written, szInfoLogBuffer);

				fprintf(gpFile, "\n[Shader Program Linking Error Log : %s]\n", szInfoLogBuffer);
				free(szInfoLogBuffer);
				DestroyWindow(ghwnd);
			}
		}
	}

	//Get Uniform Location
	// vs
	modelViewMatrixUniformPlanet = glGetUniformLocation(shaderProgramObjectSphere, "u_model_view_matrix");
	projectionMatrixUniformPlanet = glGetUniformLocation(shaderProgramObjectSphere, "u_projection_matrix");

	// fs
	textureSamplerUniformPlanets = glGetUniformLocation(shaderProgramObjectSphere, "u_Texture_Sampler");

	SphereRendering(25.0f, param_sphere_vertices,
					param_sphere_normals,
					param_sphere_texCoords,
					param_sphere_indices,
					param_sphere_line_indices);

	GLfloat glfRadius = 0.5f;
	GLfloat glfPoint;
	int count = 0;

	for (glfPoint = 0.0f; glfPoint < 2.0f * M_PI; glfPoint += 0.001f)
	{
		ellipseVerticesPath[count][0] = glfRadius * cos(glfPoint);
		ellipseVerticesPath[count][1] = (glfRadius / 2) * sin(glfPoint);
		ellipseVerticesPath[count][2] = 0.0f;

		//printf("%f, ", ellipseVerticesPath[count][0]);
		//printf("%f, ", ellipseVerticesPath[count][1]);
		//printf("%f\n", ellipseVerticesPath[count][2]);

		count++;
	}
	// vao_sphere
	glGenVertexArrays(1, &vao_sphere);
	glBindVertexArray(vao_sphere);
	// vbo_position_sphere
	glGenBuffers(1, &vbo_position_sphere);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_position_sphere);
	glBufferData(GL_ARRAY_BUFFER, param_sphere_vertices.size() * sizeof(float), param_sphere_vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(OUP_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(OUP_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/*// vbo_normal_sphere
	glGenBuffers(1, &vbo_normal_sphere);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_normal_sphere);
	glBufferData(GL_ARRAY_BUFFER, param_sphere_normals.size() * sizeof(float), param_sphere_normals.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(OUP_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(OUP_ATTRIBUTE_NORMAL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);*/

	// vbo_texture_sphere
	glGenBuffers(1, &vbo_texture_sphere);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_texture_sphere);
	glBufferData(GL_ARRAY_BUFFER, param_sphere_texCoords.size() * sizeof(float), param_sphere_texCoords.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(OUP_ATTRIBUTE_TEXTURE, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(OUP_ATTRIBUTE_TEXTURE);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// vbo_element_sphere
	glGenBuffers(1, &vbo_element_sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_element_sphere);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, param_sphere_indices.size() * sizeof(unsigned int), param_sphere_indices.data(), GL_STATIC_DRAW);
	// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Loading Texture //
	// SUN //
	LoadGLTexture(&gluiTextureImage[SUN], MAKEINTRESOURCE(SUN_BITMAP));
	if (gluiTextureImage[SUN] == 0)
	{
		fprintf(gpFile, "[%s - %d] - error>> LoadGLTexture(0_sun.bmp) failed... gluiTextureImage[SUN ] = %d]\n", __FILE__, __LINE__, gluiTextureImage[SUN]);
	}
	else
	{
		fprintf(gpFile, "[%s - %d] - info>> LoadGLTexture(0_sun.bmp) successful... gluiTextureImage[SUN ] = %d]\n", __FILE__, __LINE__, gluiTextureImage[SUN]);
	}

	// MERCURY //
	LoadGLTexture(&gluiTextureImage[MERCURY], MAKEINTRESOURCE(MERCURY_BITMAP));
	if (gluiTextureImage[MERCURY] == 0)
	{
		fprintf(gpFile, "[%s - %d] - error>> LoadGLTexture(1_mercury.bmp) failed... gluiTextureImage[MERCURY ] = %d]\n", __FILE__, __LINE__, gluiTextureImage[MERCURY]);
	}
	else
	{
		fprintf(gpFile, "[%s - %d] - info>> LoadGLTexture(1_mercury.bmp) successful... gluiTextureImage[MERCURY ] = %d]\n", __FILE__, __LINE__, gluiTextureImage[MERCURY]);
	}

	// VENUS //
	LoadGLTexture(&gluiTextureImage[VENUS], MAKEINTRESOURCE(VENUS_BITMAP));
	if (gluiTextureImage[VENUS] == 0)
	{
		fprintf(gpFile, "[%s - %d] - error>> LoadGLTexture(2_venus.bmp) failed... gluiTextureImage[VENUS ] = %d]\n", __FILE__, __LINE__, gluiTextureImage[VENUS]);
	}
	else
	{
		fprintf(gpFile, "[%s - %d] - info>> LoadGLTexture(2_venus.bmp) successful... gluiTextureImage[VENUS ] = %d]\n", __FILE__, __LINE__, gluiTextureImage[VENUS]);
	}

	// EARTH //
	LoadGLTexture(&gluiTextureImage[EARTH], MAKEINTRESOURCE(EARTH_BITMAP));
	if (gluiTextureImage[EARTH] == 0)
	{
		fprintf(gpFile, "[%s - %d] - error>> LoadGLTexture(3.0_earth.bmp) failed... gluiTextureImage[EARTH ] = %d]\n", __FILE__, __LINE__, gluiTextureImage[EARTH]);
	}
	else
	{
		fprintf(gpFile, "[%s - %d] - info>> LoadGLTexture(3.0_earth.bmp) successful... gluiTextureImage[EARTH ] = %d]\n", __FILE__, __LINE__, gluiTextureImage[EARTH]);
	}

	// MOON //
	LoadGLTexture(&gluiTextureImage[MOON], MAKEINTRESOURCE(MOON_BITMAP));
	if (gluiTextureImage[MOON] == 0)
	{
		fprintf(gpFile, "[%s - %d] - error>> LoadGLTexture(3.1_moon.bmp) failed... gluiTextureImage[MOON ] = %d]\n", __FILE__, __LINE__, gluiTextureImage[MOON]);
	}
	else
	{
		fprintf(gpFile, "[%s - %d] - info>> LoadGLTexture(3.1_moon.bmp) successful... gluiTextureImage[MOON ] = %d]\n", __FILE__, __LINE__, gluiTextureImage[MOON]);
	}

	// MARS //
	LoadGLTexture(&gluiTextureImage[MARS], MAKEINTRESOURCE(MARS_BITMAP));
	if (gluiTextureImage[MARS] == 0)
	{
		fprintf(gpFile, "[%s - %d] - error>> LoadGLTexture(4.0_mars.bmp) failed... gluiTextureImage[MARS ] = %d]\n", __FILE__, __LINE__, gluiTextureImage[MARS]);
	}
	else
	{
		fprintf(gpFile, "[%s - %d] - info>> LoadGLTexture(4.0_mars.bmp) successful... gluiTextureImage[MARS ] = %d]\n", __FILE__, __LINE__, gluiTextureImage[MARS]);
	}

	// PHOBOS //
	LoadGLTexture(&gluiTextureImage[PHOBOS], MAKEINTRESOURCE(PHOBOS_BITMAP));
	if (gluiTextureImage[PHOBOS] == 0)
	{
		fprintf(gpFile, "[%s - %d] - error>> LoadGLTexture(4.1_phobos.bmp) failed... gluiTextureImage[PHOBOS ] = %d]\n", __FILE__, __LINE__, gluiTextureImage[PHOBOS]);
	}
	else
	{
		fprintf(gpFile, "[%s - %d] - info>> LoadGLTexture(4.1_phobos.bmp) successful... gluiTextureImage[PHOBOS ] = %d]\n", __FILE__, __LINE__, gluiTextureImage[PHOBOS]);
	}

	// JUPITER //
	LoadGLTexture(&gluiTextureImage[JUPITER], MAKEINTRESOURCE(JUPITER_BITMAP));
	if (gluiTextureImage[JUPITER] == 0)
	{
		fprintf(gpFile, "[%s - %d] - error>> LoadGLTexture(5.0_jupiter.bmp) failed... gluiTextureImage[JUPITER ] = %d]\n", __FILE__, __LINE__, gluiTextureImage[JUPITER]);
	}
	else
	{
		fprintf(gpFile, "[%s - %d] - info>> LoadGLTexture(5.0_jupiter.bmp) successful... gluiTextureImage[JUPITER ] = %d]\n", __FILE__, __LINE__, gluiTextureImage[JUPITER]);
	}

	// EUROPA //
	LoadGLTexture(&gluiTextureImage[EUROPA], MAKEINTRESOURCE(EUROPA_BITMAP));
	if (gluiTextureImage[EUROPA] == 0)
	{
		fprintf(gpFile, "[%s - %d] - error>> LoadGLTexture(5.1_europa.bmp) failed... gluiTextureImage[EUROPA ] = %d]\n", __FILE__, __LINE__, gluiTextureImage[EUROPA]);
	}
	else
	{
		fprintf(gpFile, "[%s - %d] - info>> LoadGLTexture(5.1_europa.bmp) successful... gluiTextureImage[EUROPA ] = %d]\n", __FILE__, __LINE__, gluiTextureImage[EUROPA]);
	}

	// URANUS //
	LoadGLTexture(&gluiTextureImage[URANUS], MAKEINTRESOURCE(URANUS_BITMAP));
	if (gluiTextureImage[URANUS] == 0)
	{
		fprintf(gpFile, "[%s - %d] - error>> LoadGLTexture(7.0_uranus.bmp) failed... gluiTextureImage[URANUS ] = %d]\n", __FILE__, __LINE__, gluiTextureImage[URANUS]);
	}
	else
	{
		fprintf(gpFile, "[%s - %d] - info>> LoadGLTexture(7.0_uranus.bmp) successful... gluiTextureImage[URANUS ] = %d]\n", __FILE__, __LINE__, gluiTextureImage[URANUS]);
	}

	// SATURN //
	LoadGLTexture(&gluiTextureImage[SATURN], MAKEINTRESOURCE(SATURN_BITMAP));
	if (gluiTextureImage[SATURN] == 0)
	{
		fprintf(gpFile, "[%s - %d] - error>> LoadGLTexture(6.0_saturn.bmp) failed... gluiTextureImage[SATURN ] = %d]\n", __FILE__, __LINE__, gluiTextureImage[SATURN]);
	}
	else
	{
		fprintf(gpFile, "[%s - %d] - info>> LoadGLTexture(6.0_saturn.bmp) successful... gluiTextureImage[SATURN ] = %d]\n", __FILE__, __LINE__, gluiTextureImage[SATURN]);
	}

	// TITAN //
	LoadGLTexture(&gluiTextureImage[TITAN], MAKEINTRESOURCE(TITAN_BITMAP));
	if (gluiTextureImage[TITAN] == 0)
	{
		fprintf(gpFile, "[%s - %d] - error>> LoadGLTexture(6.1_titan.bmp) failed... gluiTextureImage[TITAN ] = %d]\n", __FILE__, __LINE__, gluiTextureImage[TITAN]);
	}
	else
	{
		fprintf(gpFile, "[%s - %d] - info>> LoadGLTexture(6.1_titan.bmp) successful... gluiTextureImage[TITAN ] = %d]\n", __FILE__, __LINE__, gluiTextureImage[TITAN]);
	}

	// ARIEL //
	LoadGLTexture(&gluiTextureImage[ARIEL], MAKEINTRESOURCE(ARIEL_BITMAP));
	if (gluiTextureImage[ARIEL] == 0)
	{
		fprintf(gpFile, "[%s - %d] - error>> LoadGLTexture(7.1_ariel.bmp) failed... gluiTextureImage[ARIEL ] = %d]\n", __FILE__, __LINE__, gluiTextureImage[ARIEL]);
	}
	else
	{
		fprintf(gpFile, "[%s - %d] - info>> LoadGLTexture(7.1_ariel.bmp) successful... gluiTextureImage[ARIEL ] = %d]\n", __FILE__, __LINE__, gluiTextureImage[ARIEL]);
	}

	// NEPTUNE //
	LoadGLTexture(&gluiTextureImage[NEPTUNE], MAKEINTRESOURCE(NEPTUNE_BITMAP));
	if (gluiTextureImage[NEPTUNE] == 0)
	{
		fprintf(gpFile, "[%s - %d] - error>> LoadGLTexture(8.0_neptune.bmp) failed... gluiTextureImage[NEPTUNE ] = %d]\n", __FILE__, __LINE__, gluiTextureImage[NEPTUNE]);
	}
	else
	{
		fprintf(gpFile, "[%s - %d] - info>> LoadGLTexture(8.0_neptune.bmp) successful... gluiTextureImage[NEPTUNE ] = %d]\n", __FILE__, __LINE__, gluiTextureImage[NEPTUNE]);
	}

	// TRITON //
	LoadGLTexture(&gluiTextureImage[TRITON], MAKEINTRESOURCE(TRITON_BITMAP));
	if (gluiTextureImage[TRITON] == 0)
	{
		fprintf(gpFile, "[%s - %d] - error>> LoadGLTexture(8.1_triton.bmp) failed... gluiTextureImage[TRITON ] = %d]\n", __FILE__, __LINE__, gluiTextureImage[TRITON]);
	}
	else
	{
		fprintf(gpFile, "[%s - %d] - info>> LoadGLTexture(8.1_triton.bmp) successful... gluiTextureImage[TRITON ] = %d]\n", __FILE__, __LINE__, gluiTextureImage[TRITON]);
	}

	/*
	// PLUTO //
	LoadGLTexture(&gluiTextureImage[PLUTO], MAKEINTRESOURCE(PLUTO_BITMAP));
	if (gluiTextureImage[PLUTO] == 0)
	{
		fprintf(gpFile, "[%s - %d] - error>> LoadGLTexture(9.0_pluto.bmp) failed... gluiTextureImage[PLUTO ] = %d]\n", __FILE__, __LINE__, gluiTextureImage[PLUTO]);
	}
	else
	{
		fprintf(gpFile, "[%s - %d] - info>> LoadGLTexture(9.0_pluto.bmp) successful... gluiTextureImage[PLUTO ] = %d]\n", __FILE__, __LINE__, gluiTextureImage[PLUTO]);
	}

	// CHARON //
	LoadGLTexture(&gluiTextureImage[CHARON], MAKEINTRESOURCE(CHARON_BITMAP));
	if (gluiTextureImage[CHARON] == 0)
	{
		fprintf(gpFile, "[%s - %d] - error>> LoadGLTexture(9.1_charon.bmp) failed... gluiTextureImage[CHARON ] = %d]\n", __FILE__, __LINE__, gluiTextureImage[CHARON]);
	}
	else
	{
		fprintf(gpFile, "[%s - %d] - info>> LoadGLTexture(9.1_charon.bmp) successful... gluiTextureImage[CHARON ] = %d]\n", __FILE__, __LINE__, gluiTextureImage[CHARON]);
	}
	*/

	position_index[MERCURY] = 0;
	position_index[VENUS] = 700;
	position_index[EARTH] = 1400;
	position_index[MARS] = 2100;
	position_index[JUPITER] = 2800;
	position_index[SATURN] = 3500;
	position_index[URANUS] = 4200;
	position_index[NEPTUNE] = 4900;
}

// display_solar_system() Definition
void display_solar_system(void)
{
	// Local Function Declaration
	void pushOnStack(const STACK_TYPE value);
	void popFromStack(void);
	const STACK_TYPE topOfStack(void);

	void display_starfield(void);
	void display_elliptical_paths(void);

	// Code
	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	mat4 modelViewMatrixSolarSystem = mat4::identity();
	// modelViewMatrixSolarSystem = vmath::lookat(camera_pos,
	// 										   camera_pos + camera_front,
	// 										   camera_up);

	modelViewMatrixSolarSystem = modelViewMatrixSolarSystem * vmath::lookat(vec3(0.0f, 0.0f, 515.0f),
																			vec3(0.0f, 0.0f, 0.0f),
																			vec3(0.0f, 1.0f, 0.0f));

	// modelViewMatrixSolarSystem = modelViewMatrixSolarSystem * vmath::translate(0.0f, 0.0f, -515.0f);
	modelViewMatrixSolarSystem = modelViewMatrixSolarSystem * vmath::rotate(-3.0f, 0.0f, 0.0f, 1.0f);

	pushOnStack(modelViewMatrixSolarSystem); // Main ModelView Pushed
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// SUN //
	glUseProgram(shaderProgramObjectSphere);

	mat4 modelViewMatrixSun = topOfStack(); // Main ModelView Copied
	modelViewMatrixSun = modelViewMatrixSun * vmath::translate(0.0f, 0.0f, -195.0f);
	pushOnStack(modelViewMatrixSun); // ModelView Of Sun Pushed

	modelViewMatrixSun = modelViewMatrixSun * vmath::rotate(rotationAngle, 0.0f, 1.0f, 0.0f);
	modelViewMatrixSun = modelViewMatrixSun * vmath::rotate(90.0f, 1.0f, 0.0f, 0.0f);
	modelViewMatrixSun = modelViewMatrixSun * vmath::scale(2.2f, 2.2f, 2.2f);
	glUniformMatrix4fv(modelViewMatrixUniformPlanet, 1, GL_FALSE, modelViewMatrixSun);
	glUniformMatrix4fv(projectionMatrixUniformPlanet, 1, GL_FALSE, perspectiveProjectionMatrix);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gluiTextureImage[SUN]);
	glUniform1i(textureSamplerUniformPlanets, 1);

	glBindVertexArray(vao_sphere);
	glDrawElements(GL_TRIANGLES, param_sphere_indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// MERCURY //
	mat4 modelMatrixMercury = topOfStack(); // ModelView Of Sun Copied
	modelMatrixMercury = modelMatrixMercury * vmath::translate(ellipseVerticesPath[position_index[MERCURY]][0] * mercury_value, ellipseVerticesPath[position_index[MERCURY]][1] * mercury_value, 0.0f);
	modelMatrixMercury = modelMatrixMercury * vmath::scale(0.4f, 0.4f, 0.4f);
	modelMatrixMercury = modelMatrixMercury * vmath::rotate(rotationAngle * 2.0f, 0.0f, 1.0f, 0.0f);
	modelMatrixMercury = modelMatrixMercury * vmath::rotate(90.0f, 1.0f, 0.0f, 0.0f);

	glUniformMatrix4fv(modelViewMatrixUniformPlanet, 1, GL_FALSE, modelMatrixMercury);
	glUniformMatrix4fv(projectionMatrixUniformPlanet, 1, GL_FALSE, perspectiveProjectionMatrix);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gluiTextureImage[MERCURY]);
	glUniform1i(textureSamplerUniformPlanets, 2);

	glBindVertexArray(vao_sphere);
	glDrawElements(GL_TRIANGLES, param_sphere_indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// VENUS //
	mat4 modelMatrixVenus = topOfStack(); // ModelView Of Sun Copied
	// modelMatrixVenus = modelMatrixVenus * vmath::translate(120.0f, 0.0f, 0.0f);
	modelMatrixVenus = modelMatrixVenus * vmath::translate(ellipseVerticesPath[position_index[VENUS]][0] * venus_value, ellipseVerticesPath[position_index[VENUS]][1] * venus_value, 0.0f);
	modelMatrixVenus = modelMatrixVenus * vmath::scale(0.5f, 0.5f, 0.5f);
	modelMatrixVenus = modelMatrixVenus * vmath::rotate(rotationAngle * 2.0f, 0.0f, 1.0f, 0.0f);
	modelMatrixVenus = modelMatrixVenus * vmath::rotate(90.0f, 1.0f, 0.0f, 0.0f);

	glUniformMatrix4fv(modelViewMatrixUniformPlanet, 1, GL_FALSE, modelMatrixVenus);
	glUniformMatrix4fv(projectionMatrixUniformPlanet, 1, GL_FALSE, perspectiveProjectionMatrix);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, gluiTextureImage[VENUS]);
	glUniform1i(textureSamplerUniformPlanets, 3);

	glBindVertexArray(vao_sphere);
	glDrawElements(GL_TRIANGLES, param_sphere_indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// EARTH //
	mat4 modelMatrixEarth = topOfStack(); // ModelView Of Sun Copied
	// modelMatrixEarth = modelMatrixEarth * vmath::translate(150.0f, 0.0f, 0.0f);
	modelMatrixEarth = modelMatrixEarth * vmath::translate(ellipseVerticesPath[position_index[EARTH]][0] * earth_value, ellipseVerticesPath[position_index[EARTH]][1] * earth_value, 0.0f);
	modelMatrixEarth = modelMatrixEarth * vmath::scale(0.5f, 0.5f, 0.5f);
	modelMatrixEarth = modelMatrixEarth * vmath::rotate(rotationAngle * 2.0f, 0.0f, 1.0f, 0.0f);
	modelMatrixEarth = modelMatrixEarth * vmath::rotate(90.0f, 1.0f, 0.0f, 0.0f);
	pushOnStack(modelMatrixEarth); // ModelView Of Earth Pushed

	glUniformMatrix4fv(modelViewMatrixUniformPlanet, 1, GL_FALSE, modelMatrixEarth);
	glUniformMatrix4fv(projectionMatrixUniformPlanet, 1, GL_FALSE, perspectiveProjectionMatrix);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, gluiTextureImage[EARTH]);
	glUniform1i(textureSamplerUniformPlanets, 4);

	glBindVertexArray(vao_sphere);
	glDrawElements(GL_TRIANGLES, param_sphere_indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	// MOON //
	mat4 modelMatrixMoon = topOfStack(); // ModelView Of Earth Copied
	modelMatrixMoon = modelMatrixMoon * vmath::translate(30.0f, -15.0f, 0.0f);
	modelMatrixMoon = modelMatrixMoon * vmath::scale(0.2f, 0.2f, 0.2f);
	modelMatrixMoon = modelMatrixMoon * vmath::rotate(rotationAngle * 2.0f, 0.0f, 0.0f, 1.0f);
	modelMatrixMoon = modelMatrixMoon * vmath::rotate(90.0f, 1.0f, 0.0f, 0.0f);

	glUniformMatrix4fv(modelViewMatrixUniformPlanet, 1, GL_FALSE, modelMatrixMoon);
	glUniformMatrix4fv(projectionMatrixUniformPlanet, 1, GL_FALSE, perspectiveProjectionMatrix);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, gluiTextureImage[MOON]);
	glUniform1i(textureSamplerUniformPlanets, 4);

	glBindVertexArray(vao_sphere);
	glDrawElements(GL_TRIANGLES, param_sphere_indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	popFromStack(); // ModelView Of Earth Popped

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// MARS //
	mat4 modelMatrixMars = topOfStack(); // ModelView Of Sun Copied
	// modelMatrixMars = modelMatrixMars * vmath::translate(180.0f, 0.0f, 0.0f);
	modelMatrixMars = modelMatrixMars * vmath::translate(ellipseVerticesPath[position_index[MARS]][0] * mars_value, ellipseVerticesPath[position_index[MARS]][1] * mars_value, 0.0f);
	modelMatrixMars = modelMatrixMars * vmath::scale(0.4f, 0.4f, 0.4f);
	modelMatrixMars = modelMatrixMars * vmath::rotate(rotationAngle * 2.0f, 0.0f, 1.0f, 0.0f);
	modelMatrixMars = modelMatrixMars * vmath::rotate(90.0f, 1.0f, 0.0f, 0.0f);
	pushOnStack(modelMatrixMars); // ModelView Of Mars Pushed

	glUniformMatrix4fv(modelViewMatrixUniformPlanet, 1, GL_FALSE, modelMatrixMars);
	glUniformMatrix4fv(projectionMatrixUniformPlanet, 1, GL_FALSE, perspectiveProjectionMatrix);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, gluiTextureImage[MARS]);
	glUniform1i(textureSamplerUniformPlanets, 5);

	glBindVertexArray(vao_sphere);
	glDrawElements(GL_TRIANGLES, param_sphere_indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	// PHOBOS //
	mat4 modelMatrixPhobos = topOfStack(); // ModelView Of Mars Copied
	modelMatrixPhobos = modelMatrixPhobos * vmath::translate(30.0f, -15.0f, 0.0f);
	modelMatrixPhobos = modelMatrixPhobos * vmath::scale(0.215f, 0.215f, 0.215f);
	modelMatrixPhobos = modelMatrixPhobos * vmath::rotate(rotationAngle * 2.0f, 0.0f, 0.0f, 1.0f);
	modelMatrixPhobos = modelMatrixPhobos * vmath::rotate(90.0f, 1.0f, 0.0f, 0.0f);

	glUniformMatrix4fv(modelViewMatrixUniformPlanet, 1, GL_FALSE, modelMatrixPhobos);
	glUniformMatrix4fv(projectionMatrixUniformPlanet, 1, GL_FALSE, perspectiveProjectionMatrix);

	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, gluiTextureImage[PHOBOS]);
	glUniform1i(textureSamplerUniformPlanets, 6);

	glBindVertexArray(vao_sphere);
	glDrawElements(GL_TRIANGLES, param_sphere_indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	popFromStack(); // ModelView Of Mars Popped

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// JUPITER //
	mat4 modelMatrixJupiter = topOfStack(); // ModelView Of Sun Copied
	// modelMatrixJupiter = modelMatrixJupiter * vmath::translate(220.0f, 0.0f, 0.0f);
	modelMatrixJupiter = modelMatrixJupiter * vmath::translate(ellipseVerticesPath[position_index[JUPITER]][0] * jupiter_value, ellipseVerticesPath[position_index[JUPITER]][1] * jupiter_value, 0.0f);
	modelMatrixJupiter = modelMatrixJupiter * vmath::scale(0.75f, 0.75f, 0.75f);
	modelMatrixJupiter = modelMatrixJupiter * vmath::rotate(rotationAngle * 2.0f, 0.0f, 1.0f, 0.0f);
	modelMatrixJupiter = modelMatrixJupiter * vmath::rotate(90.0f, 0.0f, 0.0f, 1.0f);
	modelMatrixJupiter = modelMatrixJupiter * vmath::rotate(90.0f, 0.0f, 1.0f, 0.0f);
	pushOnStack(modelMatrixJupiter); // ModelView Of Jupiter Pushed

	glUniformMatrix4fv(modelViewMatrixUniformPlanet, 1, GL_FALSE, modelMatrixJupiter);
	glUniformMatrix4fv(projectionMatrixUniformPlanet, 1, GL_FALSE, perspectiveProjectionMatrix);

	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, gluiTextureImage[JUPITER]);
	glUniform1i(textureSamplerUniformPlanets, 7);

	glBindVertexArray(vao_sphere);
	glDrawElements(GL_TRIANGLES, param_sphere_indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	// EUROPA //
	mat4 modelMatrixEuropa = topOfStack(); // ModelView Of Jupiter Copied
	modelMatrixEuropa = modelMatrixEuropa * vmath::translate(30.0f, -15.0f, 0.0f);
	modelMatrixEuropa = modelMatrixEuropa * vmath::scale(0.2f, 0.2f, 0.2f);
	modelMatrixEuropa = modelMatrixEuropa * vmath::rotate(rotationAngle * 4.0f, 0.0f, 0.0f, 1.0f);
	modelMatrixEuropa = modelMatrixEuropa * vmath::rotate(90.0f, 0.0f, 1.0f, 0.0f);

	glUniformMatrix4fv(modelViewMatrixUniformPlanet, 1, GL_FALSE, modelMatrixEuropa);
	glUniformMatrix4fv(projectionMatrixUniformPlanet, 1, GL_FALSE, perspectiveProjectionMatrix);

	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, gluiTextureImage[EUROPA]);
	glUniform1i(textureSamplerUniformPlanets, 8);

	glBindVertexArray(vao_sphere);
	glDrawElements(GL_TRIANGLES, param_sphere_indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	popFromStack(); // ModelView Of Jupiter Popped

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// SATURN  9 10 //
	mat4 modelMatrixSaturn = topOfStack(); // ModelView Of Sun Copied
	// modelMatrixSaturn = modelMatrixSaturn * vmath::translate(270.0f, 0.0f, 0.0f);
	modelMatrixSaturn = modelMatrixSaturn * vmath::translate(ellipseVerticesPath[position_index[SATURN]][0] * saturn_value, ellipseVerticesPath[position_index[SATURN]][1] * saturn_value, 0.0f);
	modelMatrixSaturn = modelMatrixSaturn * vmath::scale(0.55f, 0.55f, 0.55f);
	modelMatrixSaturn = modelMatrixSaturn * vmath::rotate(rotationAngle * 2.0f, 0.0f, 1.0f, 0.0f);
	modelMatrixSaturn = modelMatrixSaturn * vmath::rotate(90.0f, 0.0f, 0.0f, 1.0f);
	modelMatrixSaturn = modelMatrixSaturn * vmath::rotate(90.0f, 0.0f, 1.0f, 0.0f);
	pushOnStack(modelMatrixSaturn); // ModelView Of Saturn Pushed

	glUniformMatrix4fv(modelViewMatrixUniformPlanet, 1, GL_FALSE, modelMatrixSaturn);
	glUniformMatrix4fv(projectionMatrixUniformPlanet, 1, GL_FALSE, perspectiveProjectionMatrix);

	glActiveTexture(GL_TEXTURE9);
	glBindTexture(GL_TEXTURE_2D, gluiTextureImage[SATURN]);
	glUniform1i(textureSamplerUniformPlanets, 9);

	glBindVertexArray(vao_sphere);
	glDrawElements(GL_TRIANGLES, param_sphere_indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	popFromStack();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// URANUS //
	mat4 modelMatrixUranus = topOfStack(); // ModelView Of Sun Copied
	// modelMatrixUranus = modelMatrixUranus * vmath::translate(310.0f, 0.0f, 0.0f);
	modelMatrixUranus = modelMatrixUranus * vmath::translate(ellipseVerticesPath[position_index[URANUS]][0] * uranus_value, ellipseVerticesPath[position_index[URANUS]][1] * uranus_value, 0.0f);
	modelMatrixUranus = modelMatrixUranus * vmath::scale(0.55f, 0.55f, 0.55f);
	modelMatrixUranus = modelMatrixUranus * vmath::rotate(rotationAngle * 2.0f, 0.0f, 1.0f, 0.0f);
	modelMatrixUranus = modelMatrixUranus * vmath::rotate(90.0f, 0.0f, 0.0f, 1.0f);
	modelMatrixUranus = modelMatrixUranus * vmath::rotate(90.0f, 0.0f, 1.0f, 0.0f);
	pushOnStack(modelMatrixUranus); // ModelView Of Uranus Pushed

	glUniformMatrix4fv(modelViewMatrixUniformPlanet, 1, GL_FALSE, modelMatrixUranus);
	glUniformMatrix4fv(projectionMatrixUniformPlanet, 1, GL_FALSE, perspectiveProjectionMatrix);

	glActiveTexture(GL_TEXTURE11);
	glBindTexture(GL_TEXTURE_2D, gluiTextureImage[URANUS]);
	glUniform1i(textureSamplerUniformPlanets, 11);

	glBindVertexArray(vao_sphere);
	glDrawElements(GL_TRIANGLES, param_sphere_indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	// ARIEL //
	mat4 modelMatrixAriel = topOfStack(); // ModelView Of Uranus Copied
	modelMatrixAriel = modelMatrixAriel * vmath::translate(10.0f, -40.0f, 0.0f);
	modelMatrixAriel = modelMatrixAriel * vmath::scale(0.35f, 0.35f, 0.35f);
	modelMatrixAriel = modelMatrixAriel * vmath::rotate(rotationAngle * 4.0f, 0.0f, 0.0f, 1.0f);
	modelMatrixAriel = modelMatrixAriel * vmath::rotate(90.0f, 1.0f, 0.0f, 0.0f);

	glUniformMatrix4fv(modelViewMatrixUniformPlanet, 1, GL_FALSE, modelMatrixAriel);
	glUniformMatrix4fv(projectionMatrixUniformPlanet, 1, GL_FALSE, perspectiveProjectionMatrix);

	glActiveTexture(GL_TEXTURE12);
	glBindTexture(GL_TEXTURE_2D, gluiTextureImage[ARIEL]);
	glUniform1i(textureSamplerUniformPlanets, 12);

	glBindVertexArray(vao_sphere);
	glDrawElements(GL_TRIANGLES, param_sphere_indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	popFromStack(); // ModelView Of Uranus Popped

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// NEPTUNE //
	mat4 modelMatrixNeptune = topOfStack(); // ModelView Of Sun Copied
	// modelMatrixNeptune = modelMatrixNeptune * vmath::translate(360.0f, 0.0f, 0.0f);
	modelMatrixNeptune = modelMatrixNeptune * vmath::translate(ellipseVerticesPath[position_index[NEPTUNE]][0] * neptune_value, ellipseVerticesPath[position_index[NEPTUNE]][1] * neptune_value, 0.0f);
	modelMatrixNeptune = modelMatrixNeptune * vmath::scale(0.55f, 0.55f, 0.55f);
	modelMatrixNeptune = modelMatrixNeptune * vmath::rotate(rotationAngle * 2.0f, 0.0f, 1.0f, 0.0f);
	modelMatrixNeptune = modelMatrixNeptune * vmath::rotate(90.0f, 0.0f, 0.0f, 1.0f);
	modelMatrixNeptune = modelMatrixNeptune * vmath::rotate(90.0f, 0.0f, 1.0f, 0.0f);
	pushOnStack(modelMatrixNeptune); // ModelView Of Neptune Pushed

	glUniformMatrix4fv(modelViewMatrixUniformPlanet, 1, GL_FALSE, modelMatrixNeptune);
	glUniformMatrix4fv(projectionMatrixUniformPlanet, 1, GL_FALSE, perspectiveProjectionMatrix);

	glActiveTexture(GL_TEXTURE13);
	glBindTexture(GL_TEXTURE_2D, gluiTextureImage[NEPTUNE]);
	glUniform1i(textureSamplerUniformPlanets, 13);

	glBindVertexArray(vao_sphere);
	glDrawElements(GL_TRIANGLES, param_sphere_indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	// TRITON //
	mat4 modelMatrixTriton = topOfStack(); // ModelView Of Neptune Copied
	modelMatrixTriton = modelMatrixTriton * vmath::translate(10.0f, -40.0f, 0.0f);
	modelMatrixTriton = modelMatrixTriton * vmath::scale(0.3f, 0.3f, 0.3f);
	modelMatrixTriton = modelMatrixTriton * vmath::rotate(rotationAngle * 4.0f, 0.0f, 0.0f, 1.0f);
	modelMatrixTriton = modelMatrixTriton * vmath::rotate(90.0f, 1.0f, 0.0f, 0.0f);

	glUniformMatrix4fv(modelViewMatrixUniformPlanet, 1, GL_FALSE, modelMatrixTriton);
	glUniformMatrix4fv(projectionMatrixUniformPlanet, 1, GL_FALSE, perspectiveProjectionMatrix);

	glActiveTexture(GL_TEXTURE14);
	glBindTexture(GL_TEXTURE_2D, gluiTextureImage[TRITON]);
	glUniform1i(textureSamplerUniformPlanets, 14);

	glBindVertexArray(vao_sphere);
	glDrawElements(GL_TRIANGLES, param_sphere_indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	popFromStack(); // ModelView Of Neptune Popped
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	if (gbAllPlanetsReached == true)
	{
		gbAllPlanetsReached = false;

		display_elliptical_paths();
	}

	popFromStack(); // ModelView Of Sun Popped

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	display_starfield();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	popFromStack(); // Main ModelView Popped

	glUseProgram(0);
}

// display_elliptical_paths() Definition
void display_elliptical_paths(void)
{
	// Local Variable Declaration
	float local_ellipseVertices[6284][3];

	// Local Function Declaration
	const STACK_TYPE topOfStack(void);

	// Code
	glUseProgram(gShaderProgramObject);
	mat4 modelViewMatrix = topOfStack();
	mat4 modelViewProjectionMatrix = mat4::identity();

	modelViewMatrix = modelViewMatrix * vmath::translate(0.0f, 0.0f, -10.0f);
	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;

	glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, modelViewProjectionMatrix);

	glBindVertexArray(vao_ellipse);

	// MERCURY //
	drawEllipticalPath(142.0f, local_ellipseVertices);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_position_ellipse);
	glBufferData(GL_ARRAY_BUFFER, sizeof(local_ellipseVertices), local_ellipseVertices, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(OUP_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(OUP_ATTRIBUTE_POSITION);

	// glDrawArrays(GL_POINTS, 0, 6284);
	glDrawArrays(GL_LINE_LOOP, 0, 6284);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// VENUS //
	drawEllipticalPath(178.0f, local_ellipseVertices);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_position_ellipse);
	glBufferData(GL_ARRAY_BUFFER, sizeof(local_ellipseVertices), local_ellipseVertices, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(OUP_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(OUP_ATTRIBUTE_POSITION);

	// glDrawArrays(GL_POINTS, 0, 6284);
	glDrawArrays(GL_LINE_LOOP, 0, 6284);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// EARTH //
	drawEllipticalPath(225.0f, local_ellipseVertices);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_position_ellipse);
	glBufferData(GL_ARRAY_BUFFER, sizeof(local_ellipseVertices), local_ellipseVertices, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(OUP_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(OUP_ATTRIBUTE_POSITION);

	// glDrawArrays(GL_POINTS, 0, 6284);
	glDrawArrays(GL_LINE_LOOP, 0, 6284);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// MARS //
	drawEllipticalPath(264.0f, local_ellipseVertices);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_position_ellipse);
	glBufferData(GL_ARRAY_BUFFER, sizeof(local_ellipseVertices), local_ellipseVertices, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(OUP_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(OUP_ATTRIBUTE_POSITION);

	// glDrawArrays(GL_POINTS, 0, 6284);
	glDrawArrays(GL_LINE_LOOP, 0, 6284);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// JUPITER //
	drawEllipticalPath(325.0f, local_ellipseVertices);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_position_ellipse);
	glBufferData(GL_ARRAY_BUFFER, sizeof(local_ellipseVertices), local_ellipseVertices, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(OUP_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(OUP_ATTRIBUTE_POSITION);

	// glDrawArrays(GL_POINTS, 0, 6284);
	glDrawArrays(GL_LINE_LOOP, 0, 6284);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// SATURN //
	drawEllipticalPath(380.0f, local_ellipseVertices);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_position_ellipse);
	glBufferData(GL_ARRAY_BUFFER, sizeof(local_ellipseVertices), local_ellipseVertices, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(OUP_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(OUP_ATTRIBUTE_POSITION);

	// glDrawArrays(GL_POINTS, 0, 6284);
	glDrawArrays(GL_LINE_LOOP, 0, 6284);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// URANUS //
	drawEllipticalPath(432.0f, local_ellipseVertices);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_position_ellipse);
	glBufferData(GL_ARRAY_BUFFER, sizeof(local_ellipseVertices), local_ellipseVertices, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(OUP_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(OUP_ATTRIBUTE_POSITION);

	// glDrawArrays(GL_POINTS, 0, 6284);
	glDrawArrays(GL_LINE_LOOP, 0, 6284);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// NEPTUNE //
	drawEllipticalPath(480.0f, local_ellipseVertices);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_position_ellipse);
	glBufferData(GL_ARRAY_BUFFER, sizeof(local_ellipseVertices), local_ellipseVertices, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(OUP_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(OUP_ATTRIBUTE_POSITION);

	// glDrawArrays(GL_POINTS, 0, 6284);
	glDrawArrays(GL_LINE_LOOP, 0, 6284);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glUseProgram(0);
}

// update_solar_system() Definition
void update_solar_system(void)
{
	// Code
	if (position_index[MERCURY] < 6284)
	{
		position_index[MERCURY] = position_index[MERCURY] + 1;
	}
	else
	{
		position_index[MERCURY] = 0;
	}

	if (position_index[VENUS] < 6284)
	{
		position_index[VENUS] = position_index[VENUS] + 1;
	}
	else
	{
		position_index[VENUS] = 0;
	}

	if (position_index[EARTH] < 6284)
	{
		position_index[EARTH] = position_index[EARTH] + 1;
	}
	else
	{
		position_index[EARTH] = 0;
	}

	if (position_index[MARS] < 6284)
	{
		position_index[MARS] = position_index[MARS] + 1;
	}
	else
	{
		position_index[MARS] = 0;
	}

	if (position_index[JUPITER] < 6284)
	{
		position_index[JUPITER] = position_index[JUPITER] + 1;
	}
	else
	{
		position_index[JUPITER] = 0;
	}

	if (position_index[SATURN] < 6284)
	{
		position_index[SATURN] = position_index[SATURN] + 1;
	}
	else
	{
		position_index[SATURN] = 0;
	}

	if (position_index[URANUS] < 6284)
	{
		position_index[URANUS] = position_index[URANUS] + 1;
	}
	else
	{
		position_index[URANUS] = 0;
	}

	if (position_index[NEPTUNE] < 6284)
	{
		position_index[NEPTUNE] = position_index[NEPTUNE] + 1;
	}
	else
	{
		position_index[NEPTUNE] = 0;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	if (neptune_value <= 950.0f)
	{
		neptune_value = neptune_value + POSITION_INCREMENT_VALUE;
	}
	else
	{
		if (uranus_value <= 850.0f)
		{
			uranus_value = uranus_value + POSITION_INCREMENT_VALUE;
		}
		else
		{
			{
				if (saturn_value <= 750.0f)
				{
					saturn_value = saturn_value + POSITION_INCREMENT_VALUE;
				}
				else
				{
					{
						if (jupiter_value <= 630.0f)
						{
							jupiter_value = jupiter_value + POSITION_INCREMENT_VALUE;
						}
						else
						{
							if (mars_value <= 520.0f)
							{
								mars_value = mars_value + POSITION_INCREMENT_VALUE;
							}
							else
							{
								if (earth_value <= 440.0f)
								{
									earth_value = earth_value + POSITION_INCREMENT_VALUE;
								}
								else
								{
									if (venus_value <= 348.0f)
									{
										venus_value = venus_value + POSITION_INCREMENT_VALUE;
									}
									else
									{
										if (mercury_value <= 272.0f)
										{
											mercury_value = mercury_value + POSITION_INCREMENT_VALUE;
										}
										else
										{
											gbAllPlanetsReached = true;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

// uninitialize_solar_system() Definition
void uninitialize_solar_system(void)
{
	// Local Variable Declaration
	GLsizei ShaderCount;
	GLsizei index;
	GLuint *pShaders = NULL;

	// Code
	if (vao_sphere)
	{
		glDeleteBuffers(1, &vao_sphere);
		vao_sphere = 0;
	}

	if (vbo_position_sphere)
	{
		glDeleteBuffers(1, &vbo_position_sphere);
		vbo_position_sphere = 0;
	}

	if (param_sphere_vertices.data() != NULL)
	{
		param_sphere_vertices.clear();
	}

	if (param_sphere_normals.data() != NULL)
	{
		param_sphere_normals.clear();
	}

	if (param_sphere_texCoords.data() != NULL)
	{
		param_sphere_texCoords.clear();
	}

	if (param_sphere_indices.data() != NULL)
	{
		param_sphere_indices.clear();
	}

	if (param_sphere_line_indices.data() != NULL)
	{
		param_sphere_line_indices.clear();
	}

	if (shaderProgramObjectSphere)
	{
		//Safe Shader Cleaup
		glUseProgram(shaderProgramObjectSphere);

		glGetProgramiv(shaderProgramObjectSphere, GL_ATTACHED_SHADERS, &ShaderCount);
		pShaders = (GLuint *)malloc(sizeof(GLuint) * ShaderCount);
		//TODO : Error Checking

		glGetAttachedShaders(shaderProgramObjectSphere, ShaderCount, &ShaderCount, pShaders);
		for (index = 0; index < ShaderCount; index++)
		{
			glDetachShader(shaderProgramObjectSphere, pShaders[index]);
			glDeleteShader(pShaders[index]);
			pShaders[index] = 0;
		}

		free(pShaders);

		glDeleteShader(shaderProgramObjectSphere);
		shaderProgramObjectSphere = 0;

		glUseProgram(0);
	}
}

// pushOnStack() Definition
void pushOnStack(const STACK_TYPE value)
{
	// Code
	// fprintf(gpFile, "info>> pushOnStack() started...\n");

	if (stack_top == (STACK_SIZE - 1))
	{
		// fprintf(gpFile, "error>> stack is full...\n");
	}
	else
	{
		if (stack_array[stack_top] != value || stack_top == -1)
		{
			stack_top += 1;
			stack_array[stack_top] = value;
			// fprintf(gpFile, "info>> pushOnStack() :: %d pushed on stack...\n", stack_top);
		}
	}

	// fprintf(gpFile, "info>> pushOnStack() finished...\n");
}

// popFromStack() Definition
void popFromStack(void)
{
	// Code
	if (stack_top == -1)
	{
		// fprintf(gpFile, "error>> stack is empty...\n");
	}
	else
	{
		if (stack_top > -1)
		{
			// fprintf(gpFile, "info>> popFromStack() :: %d is popped from stack...\n", stack_top);
			stack_top -= 1;
		}
	}
}

// topOfStack() Definition
const STACK_TYPE topOfStack(void)
{
	return (stack_array[stack_top]);
}

// initialize_starfield() Definition
void initialize_starfield(void)
{
	// Local Variable Declaration
	GLint iInfoLogLength = 0;
	GLint iShaderCompiledStatus = 0;
	GLint iShaderLinkerStatus = 0;
	GLchar *szInfoLogBuffer = NULL;

	// Local Function Declaration
	unsigned int loadKTXImage(const char *filename, unsigned int tex = 0);
	GLfloat random_float(void);

	// Vertex Shader - Creating Shader
	vertexShaderObjectStarfield = glCreateShader(GL_VERTEX_SHADER);
	const GLchar *pglcVertexShaderSourceCode =
		"#version 430 core									  			          			\n"
		"													  			          			\n"
		"in vec4 vPosition;									  			          			\n"
		"in vec4 vColor;                                      			          			\n"
		"													  			          			\n"
		"uniform float u_time;                                			          			\n"
		"uniform mat4 u_modelViewMatrix;								          			\n"
		"uniform mat4 u_projectionMatrix;					  			          			\n"
		"													  			          			\n"
		"out vec4 starColor;                             			          				\n"
		"													  			          			\n"
		"void main(void)									  			          			\n"
		"{													  			          			\n"
		"	vec4 newVertex = u_modelViewMatrix * vPosition;                      			\n"
		"                                                     			          			\n"
		"   newVertex.z += u_time;                           			          			\n"
		"   newVertex.z = fract(newVertex.z);                			          			\n"
		"                                                     			          			\n"
		"   float size = (8.0 * newVertex.z * newVertex.z); 			          			\n"
		"                                                     			          			\n"
		"   starColor = vColor;							  			          				\n"
		"                                                     			          			\n"
		"   newVertex.z = (199.9 * newVertex.z) - 5.0;    			          				\n"
		"   gl_Position = u_projectionMatrix * u_modelViewMatrix * newVertex;  				\n"
		"   gl_PointSize = size;                             			          			\n"
		"}                                                    			          			\n";

	glShaderSource(vertexShaderObjectStarfield, 1, (const GLchar **)&pglcVertexShaderSourceCode, NULL);

	// Compiling Shader
	glCompileShader(vertexShaderObjectStarfield);

	// Error Checking
	glGetShaderiv(vertexShaderObjectStarfield, GL_COMPILE_STATUS, &iShaderCompiledStatus);
	if (iShaderCompiledStatus == GL_FALSE)
	{
		glGetShaderiv(vertexShaderObjectStarfield, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLogBuffer = (GLchar *)malloc(iInfoLogLength * sizeof(GLchar));
			if (szInfoLogBuffer != NULL)
			{
				GLsizei written;

				glGetShaderInfoLog(vertexShaderObjectStarfield, iInfoLogLength, &written, szInfoLogBuffer);

				fprintf(gpFile, "[%s - %d] - \n[Vertex Shader Compilation Error Log : %s]\n", __FILE__, __LINE__, szInfoLogBuffer);
				free(szInfoLogBuffer);
				DestroyWindow(ghwnd);
			}
		}
	}

	iInfoLogLength = 0;
	iShaderCompiledStatus = 0;
	iShaderLinkerStatus = 0;
	szInfoLogBuffer = NULL;

	// Fragment Shader - Creating Shader
	fragmentShaderObjectStarfield = glCreateShader(GL_FRAGMENT_SHADER);
	const GLchar *pglcFragmentShaderSourceCode =
		"#version 430 core																	\n"
		"																					\n"
		"in vec4 starColor;                                        							\n"
		"																					\n"
		"uniform sampler2D starTexSampler;                                    				\n"
		"																					\n"
		"out vec4 FragmentColor;															\n"
		"																					\n"
		"void main(void)																	\n"
		"{																					\n"
		"	FragmentColor = starColor * texture(starTexSampler, gl_PointCoord);  			\n"
		"}																					\n";

	glShaderSource(fragmentShaderObjectStarfield, 1, (const GLchar **)&pglcFragmentShaderSourceCode, NULL);

	// Compiling Shader
	glCompileShader(fragmentShaderObjectStarfield);

	// Error Checking
	glGetShaderiv(fragmentShaderObjectStarfield, GL_COMPILE_STATUS, &iShaderCompiledStatus);
	if (iShaderCompiledStatus == GL_FALSE)
	{
		glGetShaderiv(fragmentShaderObjectStarfield, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLogBuffer = (GLchar *)malloc(iInfoLogLength * sizeof(GLchar));
			if (szInfoLogBuffer != NULL)
			{
				GLsizei written;

				glGetShaderInfoLog(fragmentShaderObjectStarfield, iInfoLogLength, &written, szInfoLogBuffer);

				fprintf(gpFile, "[%s - %d] - \n[Fragment Shader Compilation Error Log : %s]\n", __FILE__, __LINE__, szInfoLogBuffer);
				free(szInfoLogBuffer);
				DestroyWindow(ghwnd);
			}
		}
	}

	// Shader Program - Create Shader Program
	shaderProgramObjectStarfield = glCreateProgram();

	glAttachShader(shaderProgramObjectStarfield, vertexShaderObjectStarfield);	 // Attach Vertex Shader To Shader Program
	glAttachShader(shaderProgramObjectStarfield, fragmentShaderObjectStarfield); // Attach Fragment Shader To Shader Program

	// Bind Vertex Shader Position Attribute
	glBindAttribLocation(shaderProgramObjectStarfield, OUP_ATTRIBUTE_POSITION, "vPosition");
	glBindAttribLocation(shaderProgramObjectStarfield, OUP_ATTRIBUTE_COLOR, "vColor");

	// Link Shader Program
	glLinkProgram(shaderProgramObjectStarfield);

	// Error Checking
	glGetProgramiv(shaderProgramObjectStarfield, GL_LINK_STATUS, &iShaderLinkerStatus);
	if (iShaderLinkerStatus == GL_FALSE)
	{
		glGetShaderiv(shaderProgramObjectStarfield, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLogBuffer = (GLchar *)malloc(iInfoLogLength * sizeof(GLchar));
			if (szInfoLogBuffer != NULL)
			{
				GLsizei written;

				glGetShaderInfoLog(shaderProgramObjectStarfield, iInfoLogLength, &written, szInfoLogBuffer);

				fprintf(gpFile, "[%s - %d] - \n[Shader Program Linking Error Log : %s]\n", __FILE__, __LINE__, szInfoLogBuffer);
				free(szInfoLogBuffer);
				DestroyWindow(ghwnd);
			}
		}
	}

	// Get Uniform Location
	// vs
	starfield_uniforms_t.model_view_matrix = glGetUniformLocation(shaderProgramObjectStarfield, "u_modelViewMatrix");
	starfield_uniforms_t.projection_matrix = glGetUniformLocation(shaderProgramObjectStarfield, "u_projectionMatrix");
	starfield_uniforms_t.time = glGetUniformLocation(shaderProgramObjectStarfield, "u_time");

	// fs
	textureSamplerUniformKTX = glGetUniformLocation(shaderProgramObjectStarfield, "starTexSampler");

	struct star_t
	{
		vec3 position;
		vec3 color;
	};

	glGenVertexArrays(1, &starfield_vao);
	glBindVertexArray(starfield_vao);
	glGenBuffers(1, &starfield_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, starfield_vbo);
	glBufferData(GL_ARRAY_BUFFER, NUMBER_OF_STARS * sizeof(star_t), NULL, GL_STATIC_DRAW);

	star_t *star = (star_t *)glMapBufferRange(GL_ARRAY_BUFFER, 0, NUMBER_OF_STARS * sizeof(star_t), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

	for (int index = 0; index < NUMBER_OF_STARS; index++)
	{
		star[index].position[0] = (random_float() * 15.0f - 7.0f) * 100.0f;
		star[index].position[1] = (random_float() * 15.0f - 7.0f) * 100.0f;
		star[index].position[2] = random_float();

		star[index].color[0] = 0.8f + random_float() * 0.50f;
		star[index].color[1] = 0.8f + random_float() * 0.25f;
		star[index].color[2] = 0.8f + random_float() * 0.75f;
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);

	glVertexAttribPointer(OUP_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(star_t), NULL);
	glEnableVertexAttribArray(OUP_ATTRIBUTE_POSITION);

	glVertexAttribPointer(OUP_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, sizeof(star_t), (void *)sizeof(vec3));
	glEnableVertexAttribArray(OUP_ATTRIBUTE_COLOR);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Loading Texture
	star_texture = loadKTXImage("resources/textures/star.ktx");
	if (star_texture == 0)
	{
		fprintf(gpFile, "[%s - %d] - error>> loadKTXImage(star.ktx) failed...\n", __FILE__, __LINE__);
	}
	else
	{
		fprintf(gpFile, "[%s - %d] - info>> loadKTXImage(star.ktx) successful... [star_texture = %d]\n", __FILE__, __LINE__, star_texture);
	}
}

const unsigned char identifier_array[] = {0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A};

// loadKTXImage() Definition
unsigned int loadKTXImage(const char *filename, unsigned int tex)
{
	// Local Function Declaration
	const unsigned int swap32(const unsigned int u32);
	unsigned int calculate_face_size(const header &h);
	unsigned int calculate_stride(const header &h, unsigned int width, unsigned int pad = 4);

	// Local Variable Declaration
	FILE *fp;
	GLuint temp = 0;
	GLuint retval = 0;
	header h;
	size_t data_start, data_end;
	unsigned char *data;
	GLenum target = GL_NONE;

	// Code
	fp = fopen(filename, "rb");

	if (!fp)
	{
		return (0);
	}

	if (fread(&h, sizeof(h), 1, fp) != 1)
	{
		goto fail_read;
	}

	if (memcmp(h.identifier, identifier_array, sizeof(identifier_array)) != 0)
	{
		goto fail_header;
	}

	if (h.endianness == 0x04030201)
	{
		// No swap needed
	}
	else if (h.endianness == 0x01020304)
	{
		// Swap needed
		h.endianness = swap32(h.endianness);
		h.gltype = swap32(h.gltype);
		h.gltypesize = swap32(h.gltypesize);
		h.glformat = swap32(h.glformat);
		h.glinternalformat = swap32(h.glinternalformat);
		h.glbaseinternalformat = swap32(h.glbaseinternalformat);
		h.pixelwidth = swap32(h.pixelwidth);
		h.pixelheight = swap32(h.pixelheight);
		h.pixeldepth = swap32(h.pixeldepth);
		h.arrayelements = swap32(h.arrayelements);
		h.faces = swap32(h.faces);
		h.miplevels = swap32(h.miplevels);
		h.keypairbytes = swap32(h.keypairbytes);
	}
	else
	{
		goto fail_header;
	}

	// Guess target (texture type)
	if (h.pixelheight == 0)
	{
		if (h.arrayelements == 0)
		{
			target = GL_TEXTURE_1D;
		}
		else
		{
			target = GL_TEXTURE_1D_ARRAY;
		}
	}
	else if (h.pixeldepth == 0)
	{
		if (h.arrayelements == 0)
		{
			if (h.faces == 0)
			{
				target = GL_TEXTURE_2D;
			}
			else
			{
				target = GL_TEXTURE_CUBE_MAP;
			}
		}
		else
		{
			if (h.faces == 0)
			{
				target = GL_TEXTURE_2D_ARRAY;
			}
			else
			{
				target = GL_TEXTURE_CUBE_MAP_ARRAY;
			}
		}
	}
	else
	{
		target = GL_TEXTURE_3D;
	}

	// Check for insanity...
	if (target == GL_NONE ||					   // Couldn't figure out target
		(h.pixelwidth == 0) ||					   // Texture has no width???
		(h.pixelheight == 0 && h.pixeldepth != 0)) // Texture has depth but no height???
	{
		goto fail_header;
	}

	temp = tex;
	if (tex == 0)
	{
		glGenTextures(1, &tex);
	}

	glBindTexture(target, tex);

	data_start = ftell(fp) + h.keypairbytes;
	fseek(fp, 0, SEEK_END);
	data_end = ftell(fp);
	fseek(fp, data_start, SEEK_SET);

	data = new unsigned char[data_end - data_start];
	memset(data, 0, data_end - data_start);

	fread(data, 1, data_end - data_start, fp);

	if (h.miplevels == 0)
	{
		h.miplevels = 1;
	}

	switch (target)
	{
	case GL_TEXTURE_1D:
		glTexStorage1D(GL_TEXTURE_1D, h.miplevels, h.glinternalformat, h.pixelwidth);
		glTexSubImage1D(GL_TEXTURE_1D, 0, 0, h.pixelwidth, h.glformat, h.glinternalformat, data);
		break;

	case GL_TEXTURE_2D:
		// glTexImage2D(GL_TEXTURE_2D, 0, h.glinternalformat, h.pixelwidth, h.pixelheight, 0, h.glformat, h.gltype, data);
		if (h.gltype == GL_NONE)
		{
			glCompressedTexImage2D(GL_TEXTURE_2D, 0, h.glinternalformat, h.pixelwidth, h.pixelheight, 0, 420 * 380 / 2, data);
		}
		else
		{
			glTexStorage2D(GL_TEXTURE_2D, h.miplevels, h.glinternalformat, h.pixelwidth, h.pixelheight);
			{
				unsigned char *ptr = data;
				unsigned int height = h.pixelheight;
				unsigned int width = h.pixelwidth;
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				for (unsigned int i = 0; i < h.miplevels; i++)
				{
					glTexSubImage2D(GL_TEXTURE_2D, i, 0, 0, width, height, h.glformat, h.gltype, ptr);
					ptr += height * calculate_stride(h, width, 1);
					height >>= 1;
					width >>= 1;

					if (!height)
					{
						height = 1;
					}

					if (!width)
					{
						width = 1;
					}
				}
			}
		}
		break;

	case GL_TEXTURE_3D:
		glTexStorage3D(GL_TEXTURE_3D, h.miplevels, h.glinternalformat, h.pixelwidth, h.pixelheight, h.pixeldepth);
		glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, h.pixelwidth, h.pixelheight, h.pixeldepth, h.glformat, h.gltype, data);
		break;

	case GL_TEXTURE_1D_ARRAY:
		glTexStorage2D(GL_TEXTURE_1D_ARRAY, h.miplevels, h.glinternalformat, h.pixelwidth, h.arrayelements);
		glTexSubImage2D(GL_TEXTURE_1D_ARRAY, 0, 0, 0, h.pixelwidth, h.arrayelements, h.glformat, h.gltype, data);
		break;

	case GL_TEXTURE_2D_ARRAY:
		glTexStorage3D(GL_TEXTURE_2D_ARRAY, h.miplevels, h.glinternalformat, h.pixelwidth, h.pixelheight, h.arrayelements);
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, h.pixelwidth, h.pixelheight, h.arrayelements, h.glformat, h.gltype, data);
		break;

	case GL_TEXTURE_CUBE_MAP:
		glTexStorage2D(GL_TEXTURE_CUBE_MAP, h.miplevels, h.glinternalformat, h.pixelwidth, h.pixelheight);
		// glTexSubImage3D(GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0, h.pixelwidth, h.pixelheight, h.faces, h.glformat, h.gltype, data);
		{
			unsigned int face_size = calculate_face_size(h);
			for (unsigned int i = 0; i < h.faces; i++)
			{
				glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 0, 0, h.pixelwidth, h.pixelheight, h.glformat, h.gltype, data + face_size * i);
			}
		}
		break;

	case GL_TEXTURE_CUBE_MAP_ARRAY:
		glTexStorage3D(GL_TEXTURE_CUBE_MAP_ARRAY, h.miplevels, h.glinternalformat, h.pixelwidth, h.pixelheight, h.arrayelements);
		glTexSubImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, 0, 0, 0, h.pixelwidth, h.pixelheight, h.faces * h.arrayelements, h.glformat, h.gltype, data);
		break;

	default: // Should never happen
		goto fail_target;
	}

	if (h.miplevels == 1)
	{
		glGenerateMipmap(target);
	}

	retval = tex;

fail_target:
	delete[] data;

fail_header:;
fail_read:;
	fclose(fp);

	return (retval);
}

// swap32() Definition
const unsigned int swap32(const unsigned int u32)
{
	// Code
	union
	{
		unsigned int u32;
		unsigned char u8[4];
	} a, b;

	a.u32 = u32;
	b.u8[0] = a.u8[3];
	b.u8[1] = a.u8[2];
	b.u8[2] = a.u8[1];
	b.u8[3] = a.u8[0];

	return (b.u32);
}

// calculate_face_size() Definition
unsigned int calculate_face_size(const header &h)
{
	// Local Function Declaration
	unsigned int calculate_stride(const header &h, unsigned int width, unsigned int pad = 4);

	// Code
	unsigned int stride = calculate_stride(h, h.pixelwidth);

	return (stride * h.pixelheight);
}

// calculate_stride() Definition
unsigned int calculate_stride(const header &h, unsigned int width, unsigned int pad = 4)
{
	// Local Variable Declaration
	unsigned int channels = 0;

	// Code
	switch (h.glbaseinternalformat)
	{
	case GL_RED:
		channels = 1;
		break;

	case GL_RG:
		channels = 2;
		break;

	case GL_BGR:
	case GL_RGB:
		channels = 3;
		break;

	case GL_BGRA:
	case GL_RGBA:
		channels = 4;
		break;
	}

	unsigned int stride = h.gltypesize * channels * width;

	stride = (stride + (pad - 1)) & ~(pad - 1);

	return (stride);
}

GLuint seed = 0x13371337; // MUST BE GLOBAL

// random_float() Definition
GLfloat random_float(void)
{
	// Local Variable Declaration
	GLfloat res;
	GLuint tmp;

	// Code
	seed *= 16807;
	tmp = seed ^ (seed >> 4) ^ (seed << 15);
	*((unsigned int *)&res) = (tmp >> 9) | 0x3F800000;

	return (res - 1.0f);
}

// display_starfield() Definition
void display_starfield(void)
{
	// Code
	glUseProgram(shaderProgramObjectStarfield);
	mat4 modelViewMatrix = mat4::identity();
	mat4 projection = mat4::identity();

	modelViewMatrix = modelViewMatrix * translate(-40.0f, 0.0f, -999.9f);
	modelViewMatrix = modelViewMatrix * rotate(0.0f, 0.0f, rotationAngle * 0.0001f);

	glUniform1f(starfield_uniforms_t.time, timeValue);
	glUniformMatrix4fv(starfield_uniforms_t.model_view_matrix, 1, GL_FALSE, modelViewMatrix);
	glUniformMatrix4fv(starfield_uniforms_t.projection_matrix, 1, GL_FALSE, perspectiveProjectionMatrix);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, star_texture);
	glUniform1i(textureSamplerUniformKTX, 0);

	glBindVertexArray(starfield_vao);
	glEnable(GL_POINT_SPRITE);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glDrawArrays(GL_POINTS, 0, NUMBER_OF_STARS);
	glBindVertexArray(0);
	glUseProgram(0);
}

// update_starfield() Definition
void update_starfield(void)
{
	// Code
	timeValue += TIME_INCREMENT_VALUE;
	timeValue -= floor(timeValue);
}

// uninitialize_starfield() Definition
void uninitialize_starfield(void)
{
	// Local Variable Declaration
	GLsizei ShaderCount;
	GLsizei index;
	GLuint *pShaders = NULL;

	// Code
	if (starfield_vao)
	{
		glDeleteBuffers(1, &starfield_vao);
		starfield_vao = 0;
	}

	if (starfield_vbo)
	{
		glDeleteBuffers(1, &starfield_vbo);
		starfield_vbo = 0;
	}

	if (star_texture)
	{
		glDeleteBuffers(1, &star_texture);
		star_texture = 0;
	}

	if (shaderProgramObjectStarfield)
	{
		// Safe Shader Cleaup
		glUseProgram(shaderProgramObjectStarfield);

		glGetProgramiv(shaderProgramObjectStarfield, GL_ATTACHED_SHADERS, &ShaderCount);
		pShaders = (GLuint *)malloc(sizeof(GLuint) * ShaderCount);
		// TODO : Error Checking

		glGetAttachedShaders(shaderProgramObjectStarfield, ShaderCount, &ShaderCount, pShaders);
		for (index = 0; index < ShaderCount; index++)
		{
			glDetachShader(shaderProgramObjectStarfield, pShaders[index]);
			glDeleteShader(pShaders[index]);
			pShaders[index] = 0;
		}

		free(pShaders);

		glDeleteShader(shaderProgramObjectStarfield);
		shaderProgramObjectStarfield = 0;

		glUseProgram(0);
	}
}
