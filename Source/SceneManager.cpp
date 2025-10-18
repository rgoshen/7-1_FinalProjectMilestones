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

// declaration of global variables
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
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
	m_mugVerticalOffset = -1.25f;  // Adjusted so mug sits properly on base (only half of base visible)

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
 *  This method defines five directional lights that simulate
 *  natural sunlight with enhanced indoor fill lighting to eliminate
 *  dark shadows per instructor feedback.
 ***********************************************************/
void SceneManager::DefineLights()
{
	// Light 0 – Sunlight (warm sunset, primary)
	// Direction: Lowered to ~35° elevation for sunset angle
	m_dirLights[0].direction = glm::normalize(glm::vec3(0.50f, 0.57f, 0.20f));
	m_dirLights[0].ambient = glm::vec3(0.14f, 0.10f, 0.08f);  // Warmer ambient
	m_dirLights[0].diffuse = glm::vec3(1.0f, 0.75f, 0.55f);   // Sunset orange/red tones
	m_dirLights[0].specular = glm::vec3(0.25f, 0.18f, 0.12f); // Warm specular
	m_dirLights[0].focalStrength = 24.0f;
	m_dirLights[0].specularIntensity = 0.15f;

	// Light 1 – Sky Fill (cool, opposite direction) - BRIGHTENED
	m_dirLights[1].direction = glm::normalize(glm::vec3(-0.123f, 0.985f, -0.123f));
	m_dirLights[1].ambient = glm::vec3(0.04f, 0.04f, 0.05f);  // Increased from 0.02
	m_dirLights[1].diffuse = glm::vec3(0.38f, 0.42f, 0.50f);  // Increased from (0.22, 0.26, 0.34)
	m_dirLights[1].specular = glm::vec3(0.0f);
	m_dirLights[1].focalStrength = 12.0f;
	m_dirLights[1].specularIntensity = 0.0f;

	// Light 2 – Wall Bounce (warm, low side fill) - BRIGHTENED
	m_dirLights[2].direction = glm::normalize(glm::vec3(-0.612f, 0.500f, -0.612f));
	m_dirLights[2].ambient = glm::vec3(0.02f);  // Increased from 0.01
	m_dirLights[2].diffuse = glm::vec3(0.28f, 0.24f, 0.20f);  // Doubled from (0.14, 0.12, 0.10)
	m_dirLights[2].specular = glm::vec3(0.0f);
	m_dirLights[2].focalStrength = 10.0f;
	m_dirLights[2].specularIntensity = 0.0f;

	// Light 3 – Back Fill (neutral, subtle) - BRIGHTENED
	m_dirLights[3].direction = glm::normalize(glm::vec3(-0.683f, 0.259f, 0.683f));
	m_dirLights[3].ambient = glm::vec3(0.02f);  // Increased from 0.01
	m_dirLights[3].diffuse = glm::vec3(0.18f);  // More than doubled from 0.08
	m_dirLights[3].specular = glm::vec3(0.0f);
	m_dirLights[3].focalStrength = 8.0f;
	m_dirLights[3].specularIntensity = 0.0f;

	// Light 4 – Overhead Indoor (neutral, direct downward)
	m_dirLights[4].direction = glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f));  // Straight down
	m_dirLights[4].ambient = glm::vec3(0.03f);
	m_dirLights[4].diffuse = glm::vec3(0.25f, 0.25f, 0.28f);  // Neutral cool white
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

	// Render the table surface
	RenderTablePlane();

	// Render the coffee mug components
	RenderMugBody();
	RenderMugInterior();
	RenderCoffee();
	RenderMugHandle();
	RenderMugBase();

	// Render desk objects
	RenderBlueSphere();
	RenderKeyboard();
	RenderTouchpad();
	RenderMonitor();
}

/***********************************************************
 *  RenderTablePlane()
 *
 *  Renders table with TILED oak texture
 *
 *  ARTISTIC CHOICE: Natural oak wood provides warm, organic
 *  foundation for the scene. Tiling at 6x demonstrates the
 *  complex texturing technique requirement while creating
 *  realistic wood grain detail across the large 20x20 plane.
 ***********************************************************/
void SceneManager::RenderTablePlane()
{
	// Apply TILED oak wood texture (Complex Texturing Technique Requirement)
	SetShaderTexture("oak");
	SetTextureUVScale(6.0f, 6.0f);  // Tile 6x6 for wood grain detail
	SetShaderMaterial("wood");

	// Set transformations for table (using box for thickness)
	glm::vec3 scaleXYZ = glm::vec3(20.0f, 0.5f, 20.0f);
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ = glm::vec3(0.0f, -0.25f, 0.0f);

	// Apply transformations
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
		ZrotationDegrees, positionXYZ);

	// Draw the box (creates table with thickness)
	m_basicMeshes->DrawBoxMesh();
}

/***********************************************************
 *  RenderMugBody()
 *
 *  Renders mug body with grey marble texture
 *
 *  ARTISTIC CHOICE: Grey marble creates an elegant, premium
 *  appearance. The natural veining adds visual interest while
 *  maintaining the neutral color palette. Cylindrical mapping
 *  on complex shape demonstrates advanced texture techniques.
 ***********************************************************/
void SceneManager::RenderMugBody()
{
	// Apply grey marble texture to mug body
	SetShaderTexture("marble");
	SetTextureUVScale(2.0f, 1.0f);
	SetShaderMaterial("marble");

	// Set transformations for mug body
	glm::vec3 scaleXYZ = glm::vec3(1.2f, 3.0f, 1.2f);  // Taller than wide
	float XrotationDegrees = 0.0f;  // Upright cylinder
	float YrotationDegrees = 25.0f; // rotate marble seam away from handle/camera
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ = glm::vec3(0.0f, 1.5f + m_mugVerticalOffset, 0.0f);  // Sitting on plane

	// Apply transformations
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
		ZrotationDegrees, positionXYZ);

	// Draw the cylinder without top cap (creates open mug)
	m_basicMeshes->DrawCylinderMesh(false, true, true);  // No top, yes bottom, yes sides
}

/***********************************************************
 *  RenderMugInterior()
 *
 *  Renders interior of mug with pale wall texture
 *
 *  ARTISTIC CHOICE: Creates hollow mug interior visible from
 *  above. Uses pale wall texture to contrast with marble
 *  exterior and suggest a ceramic glazed interior surface.
 ***********************************************************/
void SceneManager::RenderMugInterior()
{
	// Apply pale wall texture to interior (creates light interior surface)
	SetShaderTexture("pale_wall");
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderMaterial("ceramic");

	// Set transformations for mug interior (smaller diameter creates wall thickness)
	glm::vec3 scaleXYZ = glm::vec3(1.08f, 2.7f, 1.08f);  // Smaller diameter than outer (1.2) creates visible wall thickness
	float XrotationDegrees = 0.0f;  // Upright cylinder
	float YrotationDegrees = 25.0f;  // keep interior seam aligned with outer body
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ = glm::vec3(0.0f, 1.35f + m_mugVerticalOffset, 0.0f);  // Lower position creates visible rim

	// Apply transformations
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
		ZrotationDegrees, positionXYZ);

	// Draw the interior cylinder (just sides, no caps - creates hollow cavity)
	m_basicMeshes->DrawCylinderMesh(false, false, true);  // No top, no bottom, only sides
}

/***********************************************************
 *  RenderCoffee()
 *
 *  Renders coffee liquid inside the mug
 *
 *  ARTISTIC CHOICE: Brown cylinder fills mug to 3/4 height
 *  to simulate coffee. Top cap creates liquid surface.
 ***********************************************************/
void SceneManager::RenderCoffee()
{

	// Set transformations for coffee (high enough to hide handle interior)
	glm::vec3 scaleXYZ = glm::vec3(1.06f, 2.6f, 1.06f);  // Slightly smaller than interior (1.08), very full mug
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ = glm::vec3(0.0f, 1.46f + m_mugVerticalOffset, 0.0f);  // Higher position for fuller coffee level

	// Apply transformations
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
		ZrotationDegrees, positionXYZ);

	// Apply coffee material and render as a flat, non-textured surface (color path)
    // This avoids inheriting any previous object’s texture/UV state.
	m_pShaderManager->setIntValue("bUseTexture", false); // Coffee: flat color, no texture
	SetShaderMaterial("coffee");
	SetShaderColor(0.32f, 0.20f, 0.10f, 1.0f); // coffee-brown albedo

	// Draw coffee cylinder with top cap (liquid surface)
	m_basicMeshes->DrawCylinderMesh(true, false, true);  // top = true, bottom = false, sides = true

	// Restore for subsequent textured draws
	m_pShaderManager->setIntValue("bUseTexture", true);
	SetTextureUVScale(1.0f, 1.0f);  // reset UVs after non-textured draw
}

/***********************************************************
 *  RenderMugHandle()
 *
 *  Renders mug handle with pale wall texture
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
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderMaterial("ceramic");

	// Set transformations for mug handle
	glm::vec3 scaleXYZ = glm::vec3(0.8f, 0.4f, 0.3f);  // Narrower width, taller height, moderate thickness
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 90.0f;  // Rotate around Z to make it vertical
	glm::vec3 positionXYZ = glm::vec3(1.28f, 2.88f + m_mugVerticalOffset, 0.0f);  // Side of mug, slightly lower

	// Apply transformations
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
		ZrotationDegrees, positionXYZ);

	// Draw the torus
	m_basicMeshes->DrawTorusMesh();
}

/***********************************************************
 *  RenderMugBase()
 *
 *  Renders mug base rim with cracked cement texture
 *
 *  ARTISTIC CHOICE: Industrial cracked cement provides visual
 *  weight and grounds the composition. The weathered, textured
 *  appearance adds character and contrasts with the smooth
 *  marble body, creating an eclectic artistic aesthetic.
 ***********************************************************/
void SceneManager::RenderMugBase()
{
	// Apply cracked cement texture to base
	SetShaderTexture("cement");
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderMaterial("concrete");

	// Set transformations for mug base rim
	glm::vec3 scaleXYZ = glm::vec3(0.95f, 1.0f, 0.95f);
	float XrotationDegrees = 90.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ = glm::vec3(0.0f, 0.25f, 0.0f);

	// Apply transformations (Scale → Rotate → Translate)
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
		ZrotationDegrees, positionXYZ);

	// Draw the torus
	m_basicMeshes->DrawTorusMesh();
}

/***********************************************************
 *  RenderBlueSphere()
 *
 *  Renders blue rubber stress ball
 *
 *  ARTISTIC CHOICE: Vibrant blue stress ball adds pop of
 *  color to the neutral workspace palette and reinforces
 *  the desk workspace theme. Positioned front-left to create
 *  visual balance and draw the eye.
 ***********************************************************/
void SceneManager::RenderBlueSphere()
{
	// Apply rubber coating texture
	SetShaderTexture("rubber");
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderMaterial("rubber");

	// Set transformations for sphere
	glm::vec3 scaleXYZ = glm::vec3(0.8f, 0.8f, 0.8f);  // Scale 0.8 on default radius 1.0 = effective radius 0.8
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	// Table top is at Y=0.0, sphere radius is 0.8, so center at Y=0.8 sits tangent on table
	glm::vec3 positionXYZ = glm::vec3(-5.5f, 0.8f, 3.5f);  // Moved further left for spacing

	// Apply transformations
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
		ZrotationDegrees, positionXYZ);

	// Draw the sphere
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
	m_pShaderManager->setIntValue("bUseTexture", false);
	SetShaderMaterial("plastic");
	SetShaderColor(0.08f, 0.08f, 0.08f, 1.0f);

	// Base frame transformations - 3:1 ratio (width:depth)
	glm::vec3 scaleXYZ = glm::vec3(9.0f, 0.10f, 3.0f);  // Wider and proportionally narrower
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ = glm::vec3(0.0f, 0.05f, 4.0f);  // Closer to camera with space from mug

	// Apply transformations and draw frame
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
		ZrotationDegrees, positionXYZ);
	m_basicMeshes->DrawBoxMesh();

	// Layer 2: Three separate key sections (raised above frame) - lighter grey
	m_pShaderManager->setIntValue("bUseTexture", false);
	SetShaderMaterial("plastic");
	SetShaderColor(0.25f, 0.25f, 0.25f, 1.0f);  // Lighter grey for keys

	// Left section - Main keyboard area (extended to fill space)
	scaleXYZ = glm::vec3(5.0f, 0.05f, 2.8f);
	positionXYZ = glm::vec3(-1.8f, 0.13f, 4.0f);  // Left side
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
		ZrotationDegrees, positionXYZ);
	m_basicMeshes->DrawBoxMesh();

	// Middle section - Navigation/function area (moved closer to right)
	scaleXYZ = glm::vec3(1.5f, 0.05f, 2.8f);
	positionXYZ = glm::vec3(1.75f, 0.13f, 4.0f);  // Equal gap from left
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
		ZrotationDegrees, positionXYZ);
	m_basicMeshes->DrawBoxMesh();

	// Right section - Numpad area (equal gap from middle)
	scaleXYZ = glm::vec3(1.5f, 0.05f, 2.8f);
	positionXYZ = glm::vec3(3.55f, 0.13f, 4.0f);  // Equal gap from middle
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
		ZrotationDegrees, positionXYZ);
	m_basicMeshes->DrawBoxMesh();

	// Restore texture state for subsequent draws
	m_pShaderManager->setIntValue("bUseTexture", true);
	SetTextureUVScale(1.0f, 1.0f);
}

/***********************************************************
 *  RenderTouchpad()
 *
 *  Renders touchpad with two-layer flush design:
 *  - Layer 1: Black plastic frame/border
 *  - Layer 2: Grey touch surface (flush with frame)
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
	m_pShaderManager->setIntValue("bUseTexture", false);
	SetShaderMaterial("plastic");
	SetShaderColor(0.08f, 0.08f, 0.08f, 1.0f);

	// Base frame transformations - square shape (smaller)
	glm::vec3 scaleXYZ = glm::vec3(2.0f, 0.10f, 2.0f);  // Smaller square touchpad frame
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ = glm::vec3(6.0f, 0.05f, 4.0f);  // Right of keyboard

	// Apply transformations and draw frame
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
		ZrotationDegrees, positionXYZ);
	m_basicMeshes->DrawBoxMesh();

	// Layer 2: Grey touch surface (minimally raised)
	SetShaderMaterial("plastic");
	SetShaderColor(0.25f, 0.25f, 0.25f, 1.0f);  // Lighter grey for surface

	// Touch surface transformations (slightly smaller, minimally raised)
	scaleXYZ = glm::vec3(1.8f, 0.05f, 1.8f);  // Smaller to expose frame border, thinner for raised look
	positionXYZ = glm::vec3(6.0f, 0.08f, 4.0f);  // Raised 0.03 units above frame

	// Apply transformations and draw surface
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
		ZrotationDegrees, positionXYZ);
	m_basicMeshes->DrawBoxMesh();

	// Restore texture state for subsequent draws
	m_pShaderManager->setIntValue("bUseTexture", true);
	SetTextureUVScale(1.0f, 1.0f);
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
	glm::vec3 scaleXYZ = glm::vec3(1.8f, 0.15f, 1.8f);  // Flattened sphere for stability
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ = glm::vec3(0.0f, 0.15f, -5.8f);  // Behind mug, aligned with post

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
	// Monitor bottom at Y=2.0, height=4.5, so top at Y=6.5
	// Pole extends to 3/4 monitor height = 3.375, so top at Y=5.375
	// Pole goes from Y=0.3 (base top) to Y=5.375, height=5.075, center at Y=2.8375
	glm::vec3 scaleXYZ = glm::vec3(0.5f, 5.075f, 0.2f);  // Wide thin rectangle
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ = glm::vec3(0.0f, 2.8375f, -5.8f);  // Centered on base, behind monitor

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
	// Connects post front (Z=-5.7) to monitor back (Z=-5.5)
	// Cylinder rotated 90° on X-axis to make it horizontal (pointing in Z direction)
	// Post front at -5.7, monitor back at -5.5, extend into both for solid connection
	// Center at -5.625, extends from -5.8 (into post) to -5.45 (into monitor back)
	glm::vec3 scaleXYZ = glm::vec3(0.15f, 0.35f, 0.15f);  // Horizontal cylinder, length 0.35
	float XrotationDegrees = 90.0f;  // Rotate to horizontal (point in Z direction)
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ = glm::vec3(0.0f, 4.25f, -5.684f);  // Centered between post and monitor back

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
	glm::vec3 scaleXYZ = glm::vec3(8.0f, 4.5f, 0.3f);  // 16:9 aspect ratio
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ = glm::vec3(0.0f, 4.25f, -5.35f);  // Lowered: bottom at Y=2.0, top at Y=6.5

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
	glm::vec3 scaleXYZ = glm::vec3(7.6f, 4.2f, 0.25f);  // Smaller than frame
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ = glm::vec3(0.0f, 4.25f, -5.30f);  // Aligned with frame center, slightly forward

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
		ZrotationDegrees, positionXYZ);
	m_basicMeshes->DrawBoxMesh();

	// Restore texture state
	m_pShaderManager->setIntValue("bUseTexture", true);
	SetTextureUVScale(1.0f, 1.0f);
}
