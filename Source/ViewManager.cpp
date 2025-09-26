///////////////////////////////////////////////////////////////////////////////
// viewmanager.h
// ============
// manage the viewing of 3D objects within the viewport
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "ViewManager.h"

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>    

// declaration of the global variables and defines
namespace
{
	// Window configuration constants
	const int WINDOW_WIDTH = 1000;
	const int WINDOW_HEIGHT = 800;
	const char* g_ViewName = "view";
	const char* g_ProjectionName = "projection";

	// Camera movement configuration constants
	const float DEFAULT_MOVEMENT_SPEED = 2.5f;
	const float MIN_MOVEMENT_SPEED = 1.0f;
	const float MAX_MOVEMENT_SPEED = 10.0f;
	const float MOVEMENT_SPEED_INCREMENT = 0.5f;
	const float DEFAULT_MOUSE_SENSITIVITY = 0.1f;

	// Orthographic projection configuration constants
	const float DEFAULT_ORTHO_ZOOM = 10.0f;
	const float MIN_ORTHO_ZOOM = 2.0f;
	const float MAX_ORTHO_ZOOM = 20.0f;
	const float ORTHO_ZOOM_INCREMENT = 0.5f;

	// camera object used for viewing and interacting with the 3D scene
	Camera* g_pCamera = nullptr;

	// mouse movement processing variables
	float g_lastMouseX = WINDOW_WIDTH / 2.0f;
	float g_lastMouseY = WINDOW_HEIGHT / 2.0f;
	bool g_firstMouseMovement = true;

	// frame timing variables
	float g_deltaTime = 0.0f;
	float g_lastFrameTime = 0.0f;

	// projection mode state variables
	bool g_isOrthographicMode = false;
	float g_orthographicZoomLevel = DEFAULT_ORTHO_ZOOM;

	/**
	 * @brief Clamps a value between minimum and maximum bounds
	 * @param value The value to clamp
	 * @param minVal The minimum allowed value
	 * @param maxVal The maximum allowed value
	 * @return The clamped value within the specified range
	 */
	inline float ClampValue(float value, float minVal, float maxVal)
	{
		return (value < minVal) ? minVal : (value > maxVal) ? maxVal : value;
	}
}

/***********************************************************
 *  ViewManager()
 *
 *  The constructor for the class
 ***********************************************************/
ViewManager::ViewManager(ShaderManager* pShaderManager)
{
	// initialize member variables
	m_pShaderManager = pShaderManager;
	m_pWindow = nullptr;

	// create and configure camera with default parameters
	g_pCamera = new Camera();
	g_pCamera->Position = glm::vec3(0.0f, 5.0f, 12.0f);
	g_pCamera->Front = glm::vec3(0.0f, -0.5f, -2.0f);
	g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
	g_pCamera->Zoom = 80.0f;
	g_pCamera->MovementSpeed = DEFAULT_MOVEMENT_SPEED;
	g_pCamera->MouseSensitivity = DEFAULT_MOUSE_SENSITIVITY;
}

/***********************************************************
 *  ~ViewManager()
 *
 *  The destructor for the class
 ***********************************************************/
ViewManager::~ViewManager()
{
	// free up allocated memory
	m_pShaderManager = nullptr;
	m_pWindow = nullptr;
	if (nullptr != g_pCamera)
	{
		delete g_pCamera;
		g_pCamera = nullptr;
	}
}

/***********************************************************
 *  CreateDisplayWindow()
 *
 *  Creates the main GLFW display window and configures input callbacks.
 *  Sets up mouse capture, transparency support, and input event handling.
 *
 *  @param windowTitle - The title to display in the window title bar
 *  @return GLFWwindow* - Pointer to the created window, or nullptr on failure
 ***********************************************************/
GLFWwindow* ViewManager::CreateDisplayWindow(const char* windowTitle)
{
	GLFWwindow* window = nullptr;

	// create the OpenGL display window
	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, windowTitle, nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return nullptr;
	}
	glfwMakeContextCurrent(window);

	// configure mouse input callbacks
	glfwSetCursorPosCallback(window, &ViewManager::Mouse_Position_Callback);
	glfwSetScrollCallback(window, &ViewManager::Mouse_Scroll_Callback);

	// hide and lock cursor to window center for camera controls
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// enable alpha blending for transparent rendering support
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_pWindow = window;
	return window;
}

/***********************************************************
 *  Mouse_Position_Callback()
 *
 *  GLFW callback function for mouse movement events. Calculates mouse
 *  movement deltas and applies them to camera orientation for look controls.
 *
 *  @param window - The GLFW window that received the event (unused)
 *  @param xMousePos - Current mouse X position in screen coordinates
 *  @param yMousePos - Current mouse Y position in screen coordinates
 ***********************************************************/
void ViewManager::Mouse_Position_Callback(GLFWwindow* window, double xMousePos, double yMousePos)
{
	// prevent large camera jumps on first mouse movement
	if (g_firstMouseMovement)
	{
		g_lastMouseX = xMousePos;
		g_lastMouseY = yMousePos;
		g_firstMouseMovement = false;
	}

	// calculate mouse movement deltas
	float xOffset = xMousePos - g_lastMouseX;
	float yOffset = g_lastMouseY - yMousePos;  // reversed: y-coordinates go bottom to top

	// store current position for next frame's delta calculation
	g_lastMouseX = xMousePos;
	g_lastMouseY = yMousePos;

	// apply mouse movement to camera orientation
	g_pCamera->ProcessMouseMovement(xOffset, yOffset);
}

/***********************************************************
 *  Mouse_Scroll_Callback()
 *
 *  GLFW callback function for mouse scroll wheel events. Adjusts camera
 *  movement speed in perspective mode or view zoom in orthographic mode.
 *
 *  @param window - The GLFW window that received the event (unused)
 *  @param xoffset - Horizontal scroll offset (unused)
 *  @param yoffset - Vertical scroll offset (positive = up, negative = down)
 ***********************************************************/
void ViewManager::Mouse_Scroll_Callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (g_isOrthographicMode)
	{
		// adjust orthographic view zoom level
		g_orthographicZoomLevel -= yoffset * ORTHO_ZOOM_INCREMENT;
		g_orthographicZoomLevel = ClampValue(g_orthographicZoomLevel, MIN_ORTHO_ZOOM, MAX_ORTHO_ZOOM);
	}
	else
	{
		// adjust camera movement speed in perspective mode
		float speedChange = yoffset * MOVEMENT_SPEED_INCREMENT;
		g_pCamera->MovementSpeed += speedChange;
		g_pCamera->MovementSpeed = ClampValue(g_pCamera->MovementSpeed, MIN_MOVEMENT_SPEED, MAX_MOVEMENT_SPEED);
	}
}

/***********************************************************
 *  ProcessKeyboardEvents()
 *
 *  Processes keyboard input for camera movement and projection mode switching.
 *  Handles WASD movement, Q/E vertical movement, ESC to exit, and P/O for
 *  projection mode toggling.
 ***********************************************************/
void ViewManager::ProcessKeyboardEvents()
{
	// exit application on ESC key
	if (glfwGetKey(m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(m_pWindow, true);
	}

	// WASD movement controls (camera-relative directions)
	if (glfwGetKey(m_pWindow, GLFW_KEY_W) == GLFW_PRESS)
		g_pCamera->ProcessKeyboard(FORWARD, g_deltaTime);
	if (glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS)
		g_pCamera->ProcessKeyboard(BACKWARD, g_deltaTime);
	if (glfwGetKey(m_pWindow, GLFW_KEY_A) == GLFW_PRESS)
		g_pCamera->ProcessKeyboard(LEFT, g_deltaTime);
	if (glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS)
		g_pCamera->ProcessKeyboard(RIGHT, g_deltaTime);

	// Q/E vertical movement controls (world Y-axis, perspective mode only)
	if (!g_isOrthographicMode)
	{
		if (glfwGetKey(m_pWindow, GLFW_KEY_Q) == GLFW_PRESS)
			g_pCamera->Position.y += g_pCamera->MovementSpeed * g_deltaTime;
		if (glfwGetKey(m_pWindow, GLFW_KEY_E) == GLFW_PRESS)
			g_pCamera->Position.y -= g_pCamera->MovementSpeed * g_deltaTime;
	}

	// projection mode switching controls
	if (glfwGetKey(m_pWindow, GLFW_KEY_P) == GLFW_PRESS)
		g_isOrthographicMode = false;  // switch to perspective mode
	if (glfwGetKey(m_pWindow, GLFW_KEY_O) == GLFW_PRESS)
		g_isOrthographicMode = true;   // switch to orthographic mode
}

/***********************************************************
 *  PrepareSceneView()
 *
 *  Prepares the 3D scene for rendering by calculating frame timing,
 *  processing input events, and setting up view/projection matrices
 *  based on the current projection mode (perspective or orthographic).
 ***********************************************************/
void ViewManager::PrepareSceneView()
{
	glm::mat4 view;
	glm::mat4 projection;

	// calculate per-frame timing for smooth, frame-rate independent movement
	float currentFrame = glfwGetTime();
	g_deltaTime = currentFrame - g_lastFrameTime;
	g_lastFrameTime = currentFrame;

	// process keyboard and mouse input events
	ProcessKeyboardEvents();

	// get current camera view matrix
	view = g_pCamera->GetViewMatrix();

	// create projection matrix based on current projection mode
	if (g_isOrthographicMode)
	{
		// orthographic projection with adjustable zoom level
		float aspectRatio = static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT);
		projection = glm::ortho(-g_orthographicZoomLevel * aspectRatio, g_orthographicZoomLevel * aspectRatio,
		                       -g_orthographicZoomLevel, g_orthographicZoomLevel, 0.1f, 100.0f);
	}
	else
	{
		// perspective projection with camera zoom (field of view)
		projection = glm::perspective(glm::radians(g_pCamera->Zoom),
		                            static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT),
		                            0.1f, 100.0f);
	}

	// update shader uniforms with current matrices and camera position
	if (nullptr != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ViewName, view);
		m_pShaderManager->setMat4Value(g_ProjectionName, projection);
		m_pShaderManager->setVec3Value("viewPosition", g_pCamera->Position);
	}
}