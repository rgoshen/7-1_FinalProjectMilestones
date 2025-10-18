///////////////////////////////////////////////////////////////////////////////
// shadermanager.cpp
// ============
// manage the loading and rendering of 3D scenes
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>

// declaration of global variables and constants
namespace
{
	// Shader uniform names
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";

	// UV Scale Constants - Complex Texturing Technique Requirements
	const float UV_SCALE_TILED_TEXTURE = 6.0f;      // Table oak wood (tiled - CS330 requirement)
	const float UV_SCALE_PARTIAL_TEXTURE = 0.75f;   // Sphere rubber (partial - CS330 requirement)
	const float UV_SCALE_WALL_TEXTURE = 2.0f;       // Background wall
	const float UV_SCALE_MUG_BODY_U = 2.0f;         // Mug body horizontal wrap
	const float UV_SCALE_MUG_BODY_V = 1.0f;         // Mug body vertical
	const float UV_SCALE_DEFAULT = 1.0f;            // Standard 1:1 mapping

	// Color Constants
	const float COLOR_BLACK_PLASTIC_R = 0.08f;
	const float COLOR_BLACK_PLASTIC_G = 0.08f;
	const float COLOR_BLACK_PLASTIC_B = 0.08f;
	const float COLOR_GREY_KEYS_R = 0.25f;
	const float COLOR_GREY_KEYS_G = 0.25f;
	const float COLOR_GREY_KEYS_B = 0.25f;
	const float COLOR_COFFEE_R = 0.32f;
	const float COLOR_COFFEE_G = 0.20f;
	const float COLOR_COFFEE_B = 0.10f;
	const float COLOR_SCREEN_R = 0.02f;
	const float COLOR_SCREEN_G = 0.02f;
	const float COLOR_SCREEN_B = 0.02f;
	const float COLOR_ALPHA_OPAQUE = 1.0f;

	// Transformation rotation constants
	const float ROTATION_NONE = 0.0f;
	const float ROTATION_QUARTER_TURN = 90.0f;
	const float ROTATION_MUG_SEAM = 25.0f;  // Rotates marble seam away from camera
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager *pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	m_pShaderManager = NULL;
	delete m_basicMeshes;
	m_basicMeshes = NULL;
}

/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	// indicate to always flip images vertically when loaded
	stbi_set_flip_vertically_on_load(true);

	// try to parse the image data from the specified image file
	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	// if the image was successfully read from the image file
	if (image)
	{
		std::cout << "Successfully loaded image:" << filename << ", width:" << width << ", height:" << height << ", channels:" << colorChannels << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// if the loaded image is in RGB format
		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		// if the loaded image is in RGBA format - it supports transparency
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			return false;
		}

		// generate the texture mipmaps for mapping textures to lower resolutions
		glGenerateMipmap(GL_TEXTURE_2D);

		// free the image data from local memory
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		// register the loaded texture and associate it with the special tag string
		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cout << "Could not load image:" << filename << std::endl;

	// Error loading the image
	return false;
}

/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glDeleteTextures(1, &m_textureIDs[i].ID);
		m_textureIDs[i].ID = 0;
		m_textureIDs[i].tag.clear();
	}
	m_loadedTextures = 0;
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return(textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return(textureSlot);
}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return(false);
	}

	int index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.ambientColor = m_objectMaterials[index].ambientColor;
			material.ambientStrength = m_objectMaterials[index].ambientStrength;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return bFound;
}

/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	// variables for this method
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// set the scale value in the transform buffer
	scale = glm::scale(scaleXYZ);
	// set the rotation values in the transform buffer
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	// set the translation value in the transform buffer
	translation = glm::translate(positionXYZ);

	modelView = translation * rotationX * rotationY * rotationZ * scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	// variables for this method
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(
	std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		int textureSlot = FindTextureSlot(textureTag);

		if (textureSlot >= 0)
		{
			m_pShaderManager->setIntValue(g_UseTextureName, true);
			m_pShaderManager->setSampler2DValue(g_TextureValueName, textureSlot);
		}
		else
		{
			// Texture tag not found: use solid color path to avoid sampling garbage.
			m_pShaderManager->setIntValue(g_UseTextureName, false);
			// (No SetShaderColor here; caller decides the fallback color.)
		}
	}
}

/***********************************************************
 *  SetTextureUVScale()
 *
 *  This method is used for setting the texture UV scale
 *  values into the shader.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(
	std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material;
		bool bReturn = false;

		bReturn = FindMaterial(materialTag, material);
		if (bReturn == true)
		{
			m_pShaderManager->setVec3Value("material.ambientColor", material.ambientColor);
			m_pShaderManager->setFloatValue("material.ambientStrength", material.ambientStrength);
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}

/**************************************************************/
/*** STUDENTS CAN MODIFY the code in the methods BELOW for  ***/
/*** preparing and rendering their own 3D replicated scenes.***/
/*** Please refer to the code in the OpenGL sample project  ***/
/*** for assistance.                                        ***/
/**************************************************************/


/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene 
 *  rendering
 ***********************************************************/
void SceneManager::PrepareScene()
{
	// only one instance of a particular mesh needs to be
	// loaded in memory no matter how many times it is drawn
	// in the rendered 3D scene

	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadBoxMesh();  // For table with thickness

	// Load our coffee mug components
	m_basicMeshes->LoadCylinderMesh();  // Main mug body
	m_basicMeshes->LoadTorusMesh();     // Handle and base rim

	// Load sphere for stress ball
	m_basicMeshes->LoadSphereMesh();

	// Load cone for monitor stand connector
	m_basicMeshes->LoadConeMesh();

	// Set vertical adjustment for mug components
	m_mugVerticalOffset = -0.9375f;  // Adjusted so mug sits properly on base (scaled down 25%)

	// Load all scene textures
	LoadSceneTextures();

	// Define material properties for all objects
	DefineObjectMaterials();

	// Define lighting for the scene
	DefineLights();
}

/***********************************************************
 *  LoadSceneTextures()
 *
 *  This method is used for loading all texture images from
 *  files and associating them with texture tags for easy
 *  reference during rendering.
 ***********************************************************/
void SceneManager::LoadSceneTextures()
{
	bool bReturn = false;

	// Load oak wood texture for table plane (tiled - complex technique)
	bReturn = CreateGLTexture(
		"../../Utilities/textures/oak-wood.jpg",
		"oak");
	if (!bReturn) {
		std::cout << "Failed to load oak-wood.jpg texture" << std::endl;
	}

	// Load grey marble texture for mug body
	bReturn = CreateGLTexture(
		"../../Utilities/textures/grey-marble.jpg",
		"marble");
	if (!bReturn) {
		std::cout << "Failed to load grey-marble.jpg texture" << std::endl;
	}

	// Load pale wall texture for mug handle
	bReturn = CreateGLTexture(
		"../../Utilities/textures/pale-wall.jpg",
		"pale_wall");
	if (!bReturn) {
		std::cout << "Failed to load pale-wall.jpg texture" << std::endl;
	}

	// Load cracked cement texture for mug base
	bReturn = CreateGLTexture(
		"../../Utilities/textures/cracked-cement.jpg",
		"cement");
	if (!bReturn) {
		std::cout << "Failed to load cracked-cement.jpg texture" << std::endl;
	}

	// Load rubber coating texture for stress ball
	bReturn = CreateGLTexture(
		"../../Utilities/textures/rubber-coating.jpg",
		"rubber");
	if (!bReturn) {
		std::cout << "Failed to load rubber-coating.jpg texture" << std::endl;
	}

	// Bind all loaded textures to OpenGL texture slots
	BindGLTextures();
}

/***********************************************************
 *  DefineLights()
 *
 *  Defines a five-light system using directional lights to
 *  simulate realistic indoor lighting with natural sunlight.
 *  Enhanced fill lighting eliminates dark shadows per
 *  instructor feedback while maintaining visual depth.
 *
 *  CS330 REQUIREMENTS FULFILLED:
 *  - LIGHTING: 5 light sources (required: minimum 2)
 *  - NO COMPLETE SHADOWS: All objects remain visible
 *
 *  LIGHTING STRATEGY:
 *  - Light 0: Primary sunset sunlight (warm, directional)
 *  - Lights 1-4: Fill lights from multiple angles
 *  - Brightened fill values ensure no dark areas
 ***********************************************************/
void SceneManager::DefineLights()
{
	// ============================================================
	// LIGHT 0: Primary Sunlight (warm sunset, main illumination)
	// ============================================================
	// Direction: Low angle (~35° elevation) from front-right
	//   - Positive X (0.50) = coming from right side
	//   - Positive Y (0.57) = angled downward from above
	//   - Positive Z (0.20) = slight forward component
	m_dirLights[0].direction = glm::normalize(glm::vec3(0.50f, 0.57f, 0.20f));

	// Ambient: Warm base illumination present everywhere
	m_dirLights[0].ambient = glm::vec3(0.14f, 0.10f, 0.08f);  // Red-biased for sunset warmth

	// Diffuse: Strong warm orange/red tones simulate golden hour sunlight
	m_dirLights[0].diffuse = glm::vec3(1.0f, 0.75f, 0.55f);  // Full red, reduced green/blue

	// Specular: Warm highlights on glossy surfaces
	m_dirLights[0].specular = glm::vec3(0.25f, 0.18f, 0.12f);

	// Focal strength: Moderate focus for soft sunset lighting
	m_dirLights[0].focalStrength = 24.0f;

	// Specular intensity: Subtle highlights
	m_dirLights[0].specularIntensity = 0.15f;

	// ============================================================
	// LIGHT 1: Sky Fill (cool blue, from above) - BRIGHTENED
	// ============================================================
	// Direction: Nearly vertical from above, slight diagonal
	//   - Near-vertical Y (0.985) = overhead sky dome lighting
	//   - Minimal X,Z (-0.123) = very slight angle variation
	m_dirLights[1].direction = glm::normalize(glm::vec3(-0.123f, 0.985f, -0.123f));

	// Ambient: Cool blue ambient (brightened per instructor feedback)
	m_dirLights[1].ambient = glm::vec3(0.04f, 0.04f, 0.05f);  // DOUBLED from 0.02

	// Diffuse: Cool blue skylight (brightened significantly)
	m_dirLights[1].diffuse = glm::vec3(0.38f, 0.42f, 0.50f);  // INCREASED ~70% from (0.22, 0.26, 0.34)

	// Specular: None (diffuse fill only)
	m_dirLights[1].specular = glm::vec3(0.0f);
	m_dirLights[1].focalStrength = 12.0f;
	m_dirLights[1].specularIntensity = 0.0f;

	// ============================================================
	// LIGHT 2: Wall Bounce (warm, from left-front) - BRIGHTENED
	// ============================================================
	// Direction: Low angle from left-front, simulates light bouncing off wall
	//   - Negative X (-0.612) = coming from left side
	//   - Positive Y (0.500) = angled downward
	//   - Negative Z (-0.612) = coming from front
	m_dirLights[2].direction = glm::normalize(glm::vec3(-0.612f, 0.500f, -0.612f));

	// Ambient: Neutral warm (brightened per instructor feedback)
	m_dirLights[2].ambient = glm::vec3(0.02f);  // DOUBLED from 0.01

	// Diffuse: Warm bounce light (doubled intensity)
	m_dirLights[2].diffuse = glm::vec3(0.28f, 0.24f, 0.20f);  // DOUBLED from (0.14, 0.12, 0.10)

	// Specular: None (simulates diffuse bounce)
	m_dirLights[2].specular = glm::vec3(0.0f);
	m_dirLights[2].focalStrength = 10.0f;
	m_dirLights[2].specularIntensity = 0.0f;

	// ============================================================
	// LIGHT 3: Back Fill (neutral, from behind) - BRIGHTENED
	// ============================================================
	// Direction: Low angle from back-left, fills rear shadows
	//   - Negative X (-0.683) = coming from left
	//   - Low Y (0.259) = nearly horizontal
	//   - Positive Z (0.683) = coming from behind
	m_dirLights[3].direction = glm::normalize(glm::vec3(-0.683f, 0.259f, 0.683f));

	// Ambient: Neutral (brightened per instructor feedback)
	m_dirLights[3].ambient = glm::vec3(0.02f);  // DOUBLED from 0.01

	// Diffuse: Neutral grey (more than doubled)
	m_dirLights[3].diffuse = glm::vec3(0.18f);  // INCREASED 125% from 0.08

	// Specular: None (ambient fill only)
	m_dirLights[3].specular = glm::vec3(0.0f);
	m_dirLights[3].focalStrength = 8.0f;
	m_dirLights[3].specularIntensity = 0.0f;

	// ============================================================
	// LIGHT 4: Overhead Indoor (neutral cool, directly down)
	// ============================================================
	// Direction: Straight down, simulates ceiling light
	//   - Zero X,Z = perfectly vertical
	//   - Negative Y (-1.0) = pointing downward
	m_dirLights[4].direction = glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f));

	// Ambient: Cool neutral
	m_dirLights[4].ambient = glm::vec3(0.03f);

	// Diffuse: Cool neutral white (slight blue tint for indoor fluorescent feel)
	m_dirLights[4].diffuse = glm::vec3(0.25f, 0.25f, 0.28f);

	// Specular: None (ambient indoor lighting)
	m_dirLights[4].specular = glm::vec3(0.0f);
	m_dirLights[4].focalStrength = 16.0f;
	m_dirLights[4].specularIntensity = 0.0f;
}

/***********************************************************
 *  UploadLights()
 *
 *  This method uploads the directional light data to the
 *  active shader program. Called once per frame before
 *  drawing the scene.
 ***********************************************************/
void SceneManager::UploadLights()
{
	GLuint program = m_pShaderManager->m_programID;
	glUseProgram(program);
	m_pShaderManager->setIntValue("bUseLighting", true);

	auto setLight = [&](int index, const DIRECTIONAL_LIGHT& light)
		{
			std::string base = "lightSources[" + std::to_string(index) + "]";
			glm::vec3 position = glm::normalize(light.direction) * 1e6f; // keep the “distant point” approach
			m_pShaderManager->setVec3Value(base + ".position", position);
			m_pShaderManager->setVec3Value(base + ".ambientColor", light.ambient);
			m_pShaderManager->setVec3Value(base + ".diffuseColor", light.diffuse);
			m_pShaderManager->setVec3Value(base + ".specularColor", light.specular);
			m_pShaderManager->setFloatValue(base + ".focalStrength", light.focalStrength);
			m_pShaderManager->setFloatValue(base + ".specularIntensity", light.specularIntensity);
		};

	for (int i = 0; i < NUM_DIR_LIGHTS; ++i) setLight(i, m_dirLights[i]);
}

/***********************************************************
 *  DefineObjectMaterials()
 *
 *  This method is used for defining material properties
 *  for all objects in the scene. Materials control how
 *  surfaces interact with light and texture colors.
 ***********************************************************/
void SceneManager::DefineObjectMaterials()
{
	// Wood material for table plane
	OBJECT_MATERIAL woodMaterial;
	woodMaterial.ambientColor = glm::vec3(0.35f, 0.28f, 0.20f);
	woodMaterial.ambientStrength = 0.20f;
	woodMaterial.diffuseColor = glm::vec3(0.55f, 0.40f, 0.22f);
	woodMaterial.specularColor = glm::vec3(0.03f);
	woodMaterial.shininess = 8.0f;
	woodMaterial.tag = "wood";
	m_objectMaterials.push_back(woodMaterial);

	// Marble material for mug body
	OBJECT_MATERIAL marbleMaterial;
	marbleMaterial.ambientColor = glm::vec3(0.22f);
	marbleMaterial.ambientStrength = 0.22f;
	marbleMaterial.diffuseColor = glm::vec3(0.36f);
	marbleMaterial.specularColor = glm::vec3(0.01f);
	marbleMaterial.shininess = 4.0f;
	marbleMaterial.tag = "marble";
	m_objectMaterials.push_back(marbleMaterial);

	// Ceramic material for mug handle
	OBJECT_MATERIAL ceramicMaterial;
	ceramicMaterial.ambientColor = glm::vec3(0.34f);
	ceramicMaterial.ambientStrength = 0.16f;
	ceramicMaterial.diffuseColor = glm::vec3(0.60f);
	ceramicMaterial.specularColor = glm::vec3(0.02f);
	ceramicMaterial.shininess = 4.0f;
	ceramicMaterial.tag = "ceramic";
	m_objectMaterials.push_back(ceramicMaterial);

	// Concrete material for mug base
	OBJECT_MATERIAL concreteMaterial;
	concreteMaterial.ambientColor = glm::vec3(0.20f);
	concreteMaterial.ambientStrength = 0.30f;
	concreteMaterial.diffuseColor = glm::vec3(0.40f);
	concreteMaterial.specularColor = glm::vec3(0.04f);
	concreteMaterial.shininess = 6.0f;
	concreteMaterial.tag = "concrete";
	m_objectMaterials.push_back(concreteMaterial);

	// Coffee (liquid)
	OBJECT_MATERIAL coffeeMaterial;
	coffeeMaterial.ambientColor = glm::vec3(0.24f, 0.15f, 0.08f);
	coffeeMaterial.ambientStrength = 0.25f;
	coffeeMaterial.diffuseColor = glm::vec3(0.32f, 0.20f, 0.10f);
	coffeeMaterial.specularColor = glm::vec3(0.08f, 0.06f, 0.04f);
	coffeeMaterial.shininess = 5.0f;
	coffeeMaterial.tag = "coffee";
	m_objectMaterials.push_back(coffeeMaterial);

	// Blue stress ball (rubber)
	OBJECT_MATERIAL rubberMaterial;
	rubberMaterial.ambientColor = glm::vec3(0.15f, 0.20f, 0.35f);
	rubberMaterial.ambientStrength = 0.35f;
	rubberMaterial.diffuseColor = glm::vec3(0.30f, 0.40f, 0.70f);
	rubberMaterial.specularColor = glm::vec3(0.05f, 0.05f, 0.08f);  // Low specular for matte rubber
	rubberMaterial.shininess = 6.0f;  // Low shininess for rubber
	rubberMaterial.tag = "rubber";
	m_objectMaterials.push_back(rubberMaterial);

	// Black plastic for keyboard frame
	OBJECT_MATERIAL plasticMaterial;
	plasticMaterial.ambientColor = glm::vec3(0.02f, 0.02f, 0.02f);
	plasticMaterial.ambientStrength = 0.15f;
	plasticMaterial.diffuseColor = glm::vec3(0.08f, 0.08f, 0.08f);
	plasticMaterial.specularColor = glm::vec3(0.10f, 0.10f, 0.10f);
	plasticMaterial.shininess = 12.0f;
	plasticMaterial.tag = "plastic";
	m_objectMaterials.push_back(plasticMaterial);

	// Semi-gloss plastic for monitor frame
	OBJECT_MATERIAL semiGlossPlasticMaterial;
	semiGlossPlasticMaterial.ambientColor = glm::vec3(0.02f, 0.02f, 0.02f);
	semiGlossPlasticMaterial.ambientStrength = 0.15f;
	semiGlossPlasticMaterial.diffuseColor = glm::vec3(0.08f, 0.08f, 0.08f);
	semiGlossPlasticMaterial.specularColor = glm::vec3(0.15f, 0.15f, 0.15f);
	semiGlossPlasticMaterial.shininess = 28.0f;
	semiGlossPlasticMaterial.tag = "semi_gloss_plastic";
	m_objectMaterials.push_back(semiGlossPlasticMaterial);

	// Glossy black screen for monitor display
	OBJECT_MATERIAL screenMaterial;
	screenMaterial.ambientColor = glm::vec3(0.01f, 0.01f, 0.01f);
	screenMaterial.ambientStrength = 0.1f;
	screenMaterial.diffuseColor = glm::vec3(0.02f, 0.02f, 0.02f);
	screenMaterial.specularColor = glm::vec3(0.3f, 0.3f, 0.3f);
	screenMaterial.shininess = 64.0f;
	screenMaterial.tag = "screen";
	m_objectMaterials.push_back(screenMaterial);

	// Wall material (uses ceramic material values for matte appearance)
	OBJECT_MATERIAL wallMaterial;
	wallMaterial.ambientColor = glm::vec3(0.34f);
	wallMaterial.ambientStrength = 0.16f;
	wallMaterial.diffuseColor = glm::vec3(0.60f);
	wallMaterial.specularColor = glm::vec3(0.02f);
	wallMaterial.shininess = 4.0f;
	wallMaterial.tag = "wall";
	m_objectMaterials.push_back(wallMaterial);
}

/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene()
{
	// Upload lighting data to shader
	UploadLights();

	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	// Render the background wall (furthest back)
	RenderWall();

	// Render the table surface
	RenderTablePlane();

	// Render the coffee mug
	RenderMug();

	// Render desk objects
	RenderBlueSphere();
	RenderKeyboard();
	RenderTouchpad();
	RenderMonitor();
}

/***********************************************************
 *  RenderMug()
 *
 *  Renders complete coffee mug with components by calling
 *  component render methods in proper sequence.
 *
 *  ARTISTIC CHOICE: Coffee mug as centerpiece of workspace
 *  scene. Complex object demonstrating multi-primitive
 *  composition with varied materials and textures.
 ***********************************************************/
void SceneManager::RenderMug()
{
	RenderMugBody();
	RenderMugInterior();
	RenderCoffee();
	RenderMugHandle();
	RenderMugBase();
}

/***********************************************************
 *  RenderTablePlane()
 *
 *  Renders desk table surface as a low-profile box with
 *  tiled oak wood texture. This forms the base plane of the
 *  entire scene upon which all other objects rest.
 *
 *  ARTISTIC CHOICE: Natural oak wood provides warm, organic
 *  foundation for the scene. Tiling at 6x demonstrates the
 *  complex texturing technique requirement while creating
 *  realistic wood grain detail across the large 20x20 plane.
 *
 *  CS330 REQUIREMENTS FULFILLED:
 *  - TILED TEXTURE: UV scale 6.0x6.0 (required: >1.0)
 ***********************************************************/
void SceneManager::RenderTablePlane()
{
	// Apply TILED oak wood texture (Complex Texturing Technique Requirement)
	SetShaderTexture("oak");
	SetTextureUVScale(UV_SCALE_TILED_TEXTURE, UV_SCALE_TILED_TEXTURE);  // 6x6 tiling
	SetShaderMaterial("wood");

	// Transformation parameters for table surface
	// Scale: 20x20 plane with 0.5 height for thickness
	// Position: Centered at origin (0,0,0), lowered by -0.25 so top surface is at Y=0
	glm::vec3 scaleXYZ = glm::vec3(20.0f, 0.5f, 20.0f);
	float XrotationDegrees = ROTATION_NONE;
	float YrotationDegrees = ROTATION_NONE;
	float ZrotationDegrees = ROTATION_NONE;
	glm::vec3 positionXYZ = glm::vec3(0.0f, -0.25f, 0.0f);

	// Apply transformations (Scale → Rotate → Translate)
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
		ZrotationDegrees, positionXYZ);

	// Draw the box mesh (creates table with visible thickness)
	m_basicMeshes->DrawBoxMesh();
}

/***********************************************************
 *  RenderMugBody()
 *
 *  Renders the outer shell of the coffee mug as an open-top
 *  cylinder with grey marble texture. This is part of the
 *  complex multi-primitive mug object.
 *
 *  ARTISTIC CHOICE: Grey marble creates an elegant, premium
 *  appearance. The natural veining adds visual interest while
 *  maintaining the neutral color palette. Cylindrical mapping
 *  on complex shape demonstrates advanced texture techniques.
 ***********************************************************/
void SceneManager::RenderMugBody()
{
	// Apply grey marble texture to mug body
	// UV scale 2.0 horizontal wraps texture twice around circumference
	SetShaderTexture("marble");
	SetTextureUVScale(UV_SCALE_MUG_BODY_U, UV_SCALE_MUG_BODY_V);  // 2.0x1.0
	SetShaderMaterial("marble");

	// Transformation parameters for mug outer cylinder
	// Scale: 0.9 diameter, 2.25 height (scaled down 25% from original design)
	// Rotation Y: 25° rotates marble seam away from camera view
	// Position: Left side of table at (-5, 1.125, 1), adjusted by vertical offset
	glm::vec3 scaleXYZ = glm::vec3(0.9f, 2.25f, 0.9f);
	float XrotationDegrees = ROTATION_NONE;          // Upright cylinder
	float YrotationDegrees = ROTATION_MUG_SEAM;      // Rotate texture seam away from view
	float ZrotationDegrees = ROTATION_NONE;
	glm::vec3 positionXYZ = glm::vec3(-5.0f, 1.125f + m_mugVerticalOffset, 1.0f);

	// Apply transformations (Scale → Rotate → Translate)
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
		ZrotationDegrees, positionXYZ);

	// Draw cylinder without top cap to create open mug
	m_basicMeshes->DrawCylinderMesh(false, true, true);  // top=false, bottom=true, sides=true
}

/***********************************************************
 *  RenderMugInterior()
 *
 *  Renders interior cylinder of mug with pale wall texture.
 *  Slightly smaller diameter than outer body creates visible
 *  wall thickness and hollow cavity.
 *
 *  ARTISTIC CHOICE: Creates hollow mug interior visible from
 *  above. Uses pale wall texture to contrast with marble
 *  exterior and suggest a ceramic glazed interior surface.
 ***********************************************************/
void SceneManager::RenderMugInterior()
{
	// Apply pale wall texture to interior (creates light interior surface)
	SetShaderTexture("pale_wall");
	SetTextureUVScale(UV_SCALE_DEFAULT, UV_SCALE_DEFAULT);  // 1:1 mapping
	SetShaderMaterial("ceramic");

	// Transformation parameters for interior cylinder
	// Scale: 0.81 diameter (0.09 smaller than 0.9 outer = visible wall thickness)
	//        2.025 height (slightly shorter than 2.25 outer)
	// Rotation Y: 25° matches outer body seam alignment
	// Position: Same X,Z as body, slightly lower Y for positioning
	glm::vec3 scaleXYZ = glm::vec3(0.81f, 2.025f, 0.81f);
	float XrotationDegrees = ROTATION_NONE;
	float YrotationDegrees = ROTATION_MUG_SEAM;  // Align with outer body
	float ZrotationDegrees = ROTATION_NONE;
	glm::vec3 positionXYZ = glm::vec3(-5.0f, 1.0125f + m_mugVerticalOffset, 1.0f);

	// Apply transformations (Scale → Rotate → Translate)
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
		ZrotationDegrees, positionXYZ);

	// Draw cylinder sides only (no caps) to create hollow interior cavity
	m_basicMeshes->DrawCylinderMesh(false, false, true);  // top=false, bottom=false, sides=true
}

/***********************************************************
 *  RenderCoffee()
 *
 *  Renders coffee liquid inside the mug as a solid-color
 *  cylinder. Uses color-based rendering (no texture) to
 *  demonstrate both texture and color rendering techniques.
 *
 *  ARTISTIC CHOICE: Brown cylinder fills mug to ~85% height
 *  to simulate coffee. Top cap creates visible liquid surface.
 ***********************************************************/
void SceneManager::RenderCoffee()
{
	// Transformation parameters for coffee cylinder
	// Scale: 0.795 diameter (slightly smaller than 0.81 interior)
	//        1.95 height (fills mug to ~85%, high enough to hide handle attachment)
	// Position: Same X,Z as mug, positioned to sit inside interior
	glm::vec3 scaleXYZ = glm::vec3(0.795f, 1.95f, 0.795f);
	float XrotationDegrees = ROTATION_NONE;
	float YrotationDegrees = ROTATION_NONE;
	float ZrotationDegrees = ROTATION_NONE;
	glm::vec3 positionXYZ = glm::vec3(-5.0f, 1.095f + m_mugVerticalOffset, 1.0f);

	// Apply transformations (Scale → Rotate → Translate)
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
		ZrotationDegrees, positionXYZ);

	// Render as solid color (no texture) to demonstrate color rendering technique
	// Explicitly disable texture to avoid inheriting previous object's texture state
	m_pShaderManager->setIntValue("bUseTexture", false);
	SetShaderMaterial("coffee");
	SetShaderColor(COLOR_COFFEE_R, COLOR_COFFEE_G, COLOR_COFFEE_B, COLOR_ALPHA_OPAQUE);

	// Draw cylinder with top cap (creates visible liquid surface)
	m_basicMeshes->DrawCylinderMesh(true, false, true);  // top=true, bottom=false, sides=true

	// Restore texture state for subsequent draws
	m_pShaderManager->setIntValue("bUseTexture", true);
	SetTextureUVScale(UV_SCALE_DEFAULT, UV_SCALE_DEFAULT);
}

/***********************************************************
 *  RenderMugHandle()
 *
 *  Renders mug handle as a vertical torus (donut shape)
 *  rotated 90° on Z-axis. Attached to right side of mug body.
 *
 *  ARTISTIC CHOICE: Pale plaster/wall texture provides light
 *  contrast against the darker marble body. This creates visual
 *  separation and hierarchy, drawing attention to the handle
 *  while maintaining cohesion through neutral tones.
 ***********************************************************/
void SceneManager::RenderMugHandle()
{
	// Apply pale wall texture to handle
	SetShaderTexture("pale_wall");
	SetTextureUVScale(UV_SCALE_DEFAULT, UV_SCALE_DEFAULT);
	SetShaderMaterial("ceramic");

	// Transformation parameters for handle torus
	// Scale: 0.6 outer radius, 0.3 inner radius, 0.225 thickness
	// Rotation Z: 90° rotates torus vertical (default torus is horizontal)
	// Position: Right side of mug at X=-4.04 (mug center -5.0 + radius 0.9 + offset 0.06)
	//           Mid-height at Y=2.16 for ergonomic placement
	glm::vec3 scaleXYZ = glm::vec3(0.6f, 0.3f, 0.225f);
	float XrotationDegrees = ROTATION_NONE;
	float YrotationDegrees = ROTATION_NONE;
	float ZrotationDegrees = ROTATION_QUARTER_TURN;  // Make torus vertical
	glm::vec3 positionXYZ = glm::vec3(-4.04f, 2.16f + m_mugVerticalOffset, 1.0f);

	// Apply transformations (Scale → Rotate → Translate)
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
		ZrotationDegrees, positionXYZ);

	// Draw the torus mesh
	m_basicMeshes->DrawTorusMesh();
}

/***********************************************************
 *  RenderMugBase()
 *
 *  Renders mug base rim as a horizontal torus (donut shape)
 *  rotated 90° on X-axis. Forms decorative rim at bottom of mug.
 *
 *  ARTISTIC CHOICE: Industrial cracked cement provides visual
 *  weight and grounds the composition. The weathered, textured
 *  appearance adds character and contrasts with the smooth
 *  marble body, creating an eclectic artistic aesthetic.
 ***********************************************************/
void SceneManager::RenderMugBase()
{
	// Apply cracked cement texture to base rim
	SetShaderTexture("cement");
	SetTextureUVScale(UV_SCALE_DEFAULT, UV_SCALE_DEFAULT);
	SetShaderMaterial("concrete");

	// Transformation parameters for base rim torus
	// Scale: 0.7125 outer radius (slightly smaller than mug body 0.9)
	//        0.75 inner radius, creates thick rim appearance
	// Rotation X: 90° makes torus horizontal (default is in XY plane)
	// Position: Bottom of mug at Y=0.1875, same X,Z as mug body
	glm::vec3 scaleXYZ = glm::vec3(0.7125f, 0.75f, 0.7125f);
	float XrotationDegrees = ROTATION_QUARTER_TURN;  // Make torus horizontal
	float YrotationDegrees = ROTATION_NONE;
	float ZrotationDegrees = ROTATION_NONE;
	glm::vec3 positionXYZ = glm::vec3(-5.0f, 0.1875f, 1.0f);

	// Apply transformations (Scale → Rotate → Translate)
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
		ZrotationDegrees, positionXYZ);

	// Draw the torus mesh
	m_basicMeshes->DrawTorusMesh();
}

/***********************************************************
 *  RenderBlueSphere()
 *
 *  Renders blue rubber stress ball with partial texture
 *  mapping. This demonstrates the CS330 partial texture
 *  requirement by using UV coordinates < 1.0 to display
 *  only a portion of the texture image.
 *
 *  ARTISTIC CHOICE: Vibrant blue stress ball adds pop of
 *  color to the neutral workspace palette and reinforces
 *  the desk workspace theme. Positioned front-left to create
 *  visual balance and draw the eye.
 *
 *  CS330 REQUIREMENTS FULFILLED:
 *  - PARTIAL TEXTURE: UV scale 0.75x0.75 (displays upper-left 75%)
 ***********************************************************/
void SceneManager::RenderBlueSphere()
{
	// Apply rubber coating texture with PARTIAL UV mapping (Complex Texturing Technique Requirement)
	// UV scale < 1.0 displays only a portion of the texture
	SetShaderTexture("rubber");
	SetTextureUVScale(UV_SCALE_PARTIAL_TEXTURE, UV_SCALE_PARTIAL_TEXTURE);  // 0.75x0.75
	SetShaderMaterial("rubber");

	// Transformation parameters for stress ball
	// Scale: 0.8 uniform (creates sphere with radius 0.8)
	// Position: Front-left at (-5.5, 0.8, 3.5)
	//   - Y=0.8 places sphere tangent to table surface (table top at Y=0.0, radius=0.8)
	//   - X=-5.5 positions near mug for visual grouping
	//   - Z=3.5 places forward for depth layering
	glm::vec3 scaleXYZ = glm::vec3(0.8f, 0.8f, 0.8f);
	float XrotationDegrees = ROTATION_NONE;
	float YrotationDegrees = ROTATION_NONE;
	float ZrotationDegrees = ROTATION_NONE;
	glm::vec3 positionXYZ = glm::vec3(-5.5f, 0.8f, 3.5f);

	// Apply transformations (Scale → Rotate → Translate)
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
		ZrotationDegrees, positionXYZ);

	// Draw the sphere mesh
	m_basicMeshes->DrawSphereMesh();
}

/***********************************************************
 *  RenderKeyboard()
 *
 *  Renders keyboard with realistic three-section layout:
 *  - Layer 1: Black plastic housing/frame
 *  - Layer 2: Three separate key sections (left main, middle nav, right numpad)
 *
 *  ARTISTIC CHOICE: Three-section layout mimics real keyboard design
 *  with visible gaps showing black frame underneath. Uses only 4 box
 *  primitives total while creating convincing keyboard appearance.
 ***********************************************************/
void SceneManager::RenderKeyboard()
{
	// Layer 1: Black plastic keyboard frame/housing
	// Using solid color rendering (no texture) for matte plastic appearance
	m_pShaderManager->setIntValue("bUseTexture", false);
	SetShaderMaterial("plastic");
	SetShaderColor(COLOR_BLACK_PLASTIC_R, COLOR_BLACK_PLASTIC_G, COLOR_BLACK_PLASTIC_B, COLOR_ALPHA_OPAQUE);

	// Transformation parameters for keyboard base frame
	// Scale: 9.0 wide x 0.10 thin x 3.0 deep (3:1 width:depth ratio)
	// Position: Centered at X=0, forward at Z=4.0 (front-center of desk)
	//   - Y=0.05 sits just above table surface (table top at Y=0.0)
	glm::vec3 scaleXYZ = glm::vec3(9.0f, 0.10f, 3.0f);
	float XrotationDegrees = ROTATION_NONE;
	float YrotationDegrees = ROTATION_NONE;
	float ZrotationDegrees = ROTATION_NONE;
	glm::vec3 positionXYZ = glm::vec3(0.0f, 0.05f, 4.0f);

	// Apply transformations and draw frame
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
		ZrotationDegrees, positionXYZ);
	m_basicMeshes->DrawBoxMesh();

	// Layer 2: Three separate key sections (raised above frame)
	// Lighter grey color distinguishes keys from black frame
	m_pShaderManager->setIntValue("bUseTexture", false);
	SetShaderMaterial("plastic");
	SetShaderColor(COLOR_GREY_KEYS_R, COLOR_GREY_KEYS_G, COLOR_GREY_KEYS_B, COLOR_ALPHA_OPAQUE);

	// Left section - Main keyboard area
	// Largest section containing letter keys, numbers, and main function keys
	scaleXYZ = glm::vec3(5.0f, 0.05f, 2.8f);  // 5.0 wide, very thin 0.05 height
	positionXYZ = glm::vec3(-1.8f, 0.13f, 4.0f);  // Left side, raised to Y=0.13
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
		ZrotationDegrees, positionXYZ);
	m_basicMeshes->DrawBoxMesh();

	// Middle section - Navigation/function area
	// Home/End/Page keys and arrow cluster
	scaleXYZ = glm::vec3(1.5f, 0.05f, 2.8f);  // Narrower than main area
	positionXYZ = glm::vec3(1.75f, 0.13f, 4.0f);  // Gap creates visual separation
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
		ZrotationDegrees, positionXYZ);
	m_basicMeshes->DrawBoxMesh();

	// Right section - Numpad area
	// Number pad with calculator-style layout
	scaleXYZ = glm::vec3(1.5f, 0.05f, 2.8f);  // Same size as middle section
	positionXYZ = glm::vec3(3.55f, 0.13f, 4.0f);  // Equal gap from middle
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
		ZrotationDegrees, positionXYZ);
	m_basicMeshes->DrawBoxMesh();

	// Restore texture state for subsequent textured draws
	m_pShaderManager->setIntValue("bUseTexture", true);
	SetTextureUVScale(UV_SCALE_DEFAULT, UV_SCALE_DEFAULT);
}

/***********************************************************
 *  RenderTouchpad()
 *
 *  Renders touchpad with two-layer flush design:
 *  - Layer 1: Black plastic frame/border
 *  - Layer 2: Grey touch surface (minimally raised)
 *
 *  ARTISTIC CHOICE: Square touchpad positioned right of
 *  keyboard creates balanced workspace composition. Flush
 *  surface design (vs raised keyboard keys) reflects realistic
 *  touchpad appearance. Uses only 2 box primitives for
 *  extremely low polygon count.
 ***********************************************************/
void SceneManager::RenderTouchpad()
{
	// Layer 1: Black plastic touchpad frame/border
	// Using solid color rendering (no texture) matching keyboard style
	m_pShaderManager->setIntValue("bUseTexture", false);
	SetShaderMaterial("plastic");
	SetShaderColor(COLOR_BLACK_PLASTIC_R, COLOR_BLACK_PLASTIC_G, COLOR_BLACK_PLASTIC_B, COLOR_ALPHA_OPAQUE);

	// Transformation parameters for touchpad base frame
	// Scale: 2.0x2.0 square (much smaller than keyboard)
	// Position: Right of keyboard at X=6.0, same Z=4.0 and Y=0.05
	glm::vec3 scaleXYZ = glm::vec3(2.0f, 0.10f, 2.0f);
	float XrotationDegrees = ROTATION_NONE;
	float YrotationDegrees = ROTATION_NONE;
	float ZrotationDegrees = ROTATION_NONE;
	glm::vec3 positionXYZ = glm::vec3(6.0f, 0.05f, 4.0f);

	// Apply transformations and draw frame
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
		ZrotationDegrees, positionXYZ);
	m_basicMeshes->DrawBoxMesh();

	// Layer 2: Grey touch surface (minimally raised above frame)
	// Slightly smaller than frame to expose border, creating realistic look
	SetShaderMaterial("plastic");
	SetShaderColor(COLOR_GREY_KEYS_R, COLOR_GREY_KEYS_G, COLOR_GREY_KEYS_B, COLOR_ALPHA_OPAQUE);

	// Transformation parameters for touch surface
	// Scale: 1.8x1.8 (0.2 smaller than 2.0 frame exposes 0.1 border on each side)
	//        0.05 height (half the 0.10 frame height, creates subtle raised appearance)
	// Position: Same X,Z as frame, raised to Y=0.08 (0.03 above frame top at Y=0.05+0.05=0.10 → 0.08 creates flush look)
	scaleXYZ = glm::vec3(1.8f, 0.05f, 1.8f);
	positionXYZ = glm::vec3(6.0f, 0.08f, 4.0f);

	// Apply transformations and draw surface
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
		ZrotationDegrees, positionXYZ);
	m_basicMeshes->DrawBoxMesh();

	// Restore texture state for subsequent textured draws
	m_pShaderManager->setIntValue("bUseTexture", true);
	SetTextureUVScale(UV_SCALE_DEFAULT, UV_SCALE_DEFAULT);
}

/***********************************************************
 *  RenderMonitor()
 *
 *  Renders complete flat screen monitor with stand by calling
 *  component render methods in proper sequence.
 *
 *  ARTISTIC CHOICE: Modern minimalist monitor design with
 *  central stand creates professional workspace aesthetic.
 *  Positioned behind keyboard as visual focal point.
 ***********************************************************/
void SceneManager::RenderMonitor()
{
	RenderMonitorBase();
	RenderMonitorPole();
	RenderMonitorConnector();
	RenderMonitorFrame();
	RenderMonitorScreen();
}

/***********************************************************
 *  RenderMonitorBase()
 *
 *  Renders monitor stand base as flattened sphere.
 *
 *  ARTISTIC CHOICE: Flattened sphere creates modern, stable
 *  base design similar to contemporary monitor stands.
 ***********************************************************/
void SceneManager::RenderMonitorBase()
{
	// Disable textures - using solid colors
	m_pShaderManager->setIntValue("bUseTexture", false);
	SetShaderMaterial("plastic");
	SetShaderColor(0.08f, 0.08f, 0.08f, 1.0f);

	// Set transformations for flattened sphere base
	glm::vec3 scaleXYZ = glm::vec3(2.5f, 0.15f, 2.5f);  // Larger flattened sphere for stability with wider monitor
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ = glm::vec3(0.0f, 0.15f, -2.5f);  // Moved forward, behind mug

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
		ZrotationDegrees, positionXYZ);
	m_basicMeshes->DrawSphereMesh();

	// Restore texture state
	m_pShaderManager->setIntValue("bUseTexture", true);
	SetTextureUVScale(1.0f, 1.0f);
}

/***********************************************************
 *  RenderMonitorPole()
 *
 *  Renders monitor stand pole as tall thin rectangle.
 *
 *  ARTISTIC CHOICE: Thin rectangular pole provides minimal
 *  visual obstruction while supporting monitor display.
 ***********************************************************/
void SceneManager::RenderMonitorPole()
{
	// Disable textures - using solid colors
	m_pShaderManager->setIntValue("bUseTexture", false);
	SetShaderMaterial("plastic");
	SetShaderColor(0.08f, 0.08f, 0.08f, 1.0f);

	// Set transformations for vertical rectangular pole
	// Monitor bottom at Y=2.0, height=6.1875, so top at Y=8.1875
	// Pole extends to 3/4 monitor height = 4.640625, so top at Y=6.640625
	// Pole goes from Y=0.3 (base top) to Y=6.640625, height=6.340625, center at Y=3.4703125
	glm::vec3 scaleXYZ = glm::vec3(0.5f, 6.340625f, 0.2f);  // Wide thin rectangle, extended for taller monitor
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ = glm::vec3(0.0f, 3.4703125f, -2.5f);  // Moved forward with base

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
		ZrotationDegrees, positionXYZ);
	m_basicMeshes->DrawBoxMesh();

	// Restore texture state
	m_pShaderManager->setIntValue("bUseTexture", true);
	SetTextureUVScale(1.0f, 1.0f);
}

/***********************************************************
 *  RenderMonitorConnector()
 *
 *  Renders connector as small horizontal cylinder bridging
 *  post to monitor back.
 *
 *  ARTISTIC CHOICE: Horizontal cylinder arm connects post
 *  to monitor center, parallel to desk surface.
 ***********************************************************/
void SceneManager::RenderMonitorConnector()
{
	// Disable textures - using solid colors
	m_pShaderManager->setIntValue("bUseTexture", false);
	SetShaderMaterial("plastic");
	SetShaderColor(0.08f, 0.08f, 0.08f, 1.0f);

	// Set transformations for horizontal cylinder connector
	// Connects post front to monitor back
	// Cylinder rotated 90° on X-axis to make it horizontal (pointing in Z direction)
	glm::vec3 scaleXYZ = glm::vec3(0.15f, 0.35f, 0.15f);  // Horizontal cylinder, length 0.35
	float XrotationDegrees = 90.0f;  // Rotate to horizontal (point in Z direction)
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ = glm::vec3(0.0f, 5.09375f, -2.388f);  // Moved forward, aligned with monitor center height

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
		ZrotationDegrees, positionXYZ);
	m_basicMeshes->DrawCylinderMesh();

	// Restore texture state
	m_pShaderManager->setIntValue("bUseTexture", true);
	SetTextureUVScale(1.0f, 1.0f);
}

/***********************************************************
 *  RenderMonitorFrame()
 *
 *  Renders monitor outer frame/bezel as thin rectangle.
 *
 *  ARTISTIC CHOICE: Semi-gloss black frame with 16:9 aspect
 *  ratio creates realistic modern monitor appearance.
 ***********************************************************/
void SceneManager::RenderMonitorFrame()
{
	// Disable textures - using solid colors
	m_pShaderManager->setIntValue("bUseTexture", false);
	SetShaderMaterial("semi_gloss_plastic");
	SetShaderColor(0.08f, 0.08f, 0.08f, 1.0f);

	// Set transformations for monitor frame
	glm::vec3 scaleXYZ = glm::vec3(11.0f, 6.1875f, 0.3f);  // 16:9 aspect ratio, increased width
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ = glm::vec3(0.0f, 5.09375f, -2.05f);  // Moved forward: bottom at Y=2.0, top at Y=8.1875

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
		ZrotationDegrees, positionXYZ);
	m_basicMeshes->DrawBoxMesh();

	// Restore texture state
	m_pShaderManager->setIntValue("bUseTexture", true);
	SetTextureUVScale(1.0f, 1.0f);
}

/***********************************************************
 *  RenderMonitorScreen()
 *
 *  Renders monitor display screen as glossy black rectangle.
 *
 *  ARTISTIC CHOICE: Glossy black screen with high specular
 *  creates realistic powered-off LCD display appearance.
 ***********************************************************/
void SceneManager::RenderMonitorScreen()
{
	// Disable textures - using solid colors
	m_pShaderManager->setIntValue("bUseTexture", false);
	SetShaderMaterial("screen");
	SetShaderColor(0.02f, 0.02f, 0.02f, 1.0f);

	// Set transformations for screen (slightly smaller than frame, inset)
	glm::vec3 scaleXYZ = glm::vec3(10.6f, 5.8875f, 0.25f);  // Smaller than frame to create bezel
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ = glm::vec3(0.0f, 5.09375f, -2.0f);  // Moved forward, aligned with frame center, slightly forward

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
		ZrotationDegrees, positionXYZ);
	m_basicMeshes->DrawBoxMesh();

	// Restore texture state
	m_pShaderManager->setIntValue("bUseTexture", true);
	SetTextureUVScale(1.0f, 1.0f);
}

/***********************************************************
 *  RenderWall()
 *
 *  Renders background wall behind desk and monitor as a
 *  large vertical plane. Provides visual backdrop that
 *  enhances monitor visibility.
 *
 *  ARTISTIC CHOICE: Light-colored wall provides contrast
 *  for dark monitor, adds realism to workspace setting,
 *  and frames the scene with defined background boundary.
 ***********************************************************/
void SceneManager::RenderWall()
{
	// Apply pale wall texture with 2x2 tiling for subtle texture detail
	SetShaderTexture("pale_wall");
	SetTextureUVScale(UV_SCALE_WALL_TEXTURE, UV_SCALE_WALL_TEXTURE);  // 2.0x2.0
	SetShaderMaterial("wall");

	// Transformation parameters for background wall
	// Scale: 25.0 wide (wider than 20.0 table), 13.0 tall, 1.0 depth
	// Position: Centered at X=0, far back at Z=-10, centered at Y=6.0
	//   - With 13.0 height and center at Y=6.0, wall extends from Y=-0.5 to Y=12.5
	//   - Positioned at back edge of table, behind all scene objects
	glm::vec3 scaleXYZ = glm::vec3(25.0f, 13.0f, 1.0f);
	float XrotationDegrees = ROTATION_NONE;
	float YrotationDegrees = ROTATION_NONE;
	float ZrotationDegrees = ROTATION_NONE;
	glm::vec3 positionXYZ = glm::vec3(0.0f, 6.0f, -10.0f);

	// Apply transformations (Scale → Rotate → Translate)
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
		ZrotationDegrees, positionXYZ);

	// Draw the box mesh (forms wall plane)
	m_basicMeshes->DrawBoxMesh();
}
