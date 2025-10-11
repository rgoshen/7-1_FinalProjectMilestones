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
		glGenTextures(1, &m_textureIDs[i].ID);
	}
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

	return(true);
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
		m_pShaderManager->setIntValue(g_UseTextureName, true);

		int textureID = -1;
		textureID = FindTextureSlot(textureTag);
		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID);
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

	// Bind all loaded textures to OpenGL texture slots
	BindGLTextures();
}

/***********************************************************
 *  DefineLights()
 *
 *  This method defines four directional lights that simulate
 *  natural sunlight entering through a high window (80° elevation,
 *  45° azimuth) with soft ambient fill from the environment.
 ***********************************************************/
void SceneManager::DefineLights()
{
	// Light 0 – Sunlight (warm, primary)
	// Direction: 80° elevation, 45° azimuth → (0.123, 0.985, 0.123)
	m_dirLights[0].direction = glm::normalize(glm::vec3(0.123f, 0.985f, 0.123f));
	m_dirLights[0].ambient = glm::vec3(0.10f, 0.10f, 0.12f);
	m_dirLights[0].diffuse = glm::vec3(1.00f, 0.98f, 0.92f);
	m_dirLights[0].specular = glm::vec3(1.00f, 1.00f, 1.00f);
	m_dirLights[0].focalStrength = 32.0f;
	m_dirLights[0].specularIntensity = 1.0f;

	// Light 1 – Sky Fill (cool, opposite direction)
	m_dirLights[1].direction = glm::normalize(glm::vec3(-0.123f, 0.985f, -0.123f));
	m_dirLights[1].ambient = glm::vec3(0.12f, 0.13f, 0.16f);
	m_dirLights[1].diffuse = glm::vec3(0.35f, 0.40f, 0.50f);
	m_dirLights[1].specular = glm::vec3(0.0f);
	m_dirLights[1].focalStrength = 16.0f;
	m_dirLights[1].specularIntensity = 0.10f;

	// Light 2 – Wall Bounce (warm, low side fill)
	m_dirLights[2].direction = glm::normalize(glm::vec3(-0.612f, 0.500f, -0.612f));
	m_dirLights[2].ambient = glm::vec3(0.05f);
	m_dirLights[2].diffuse = glm::vec3(0.20f, 0.17f, 0.15f);
	m_dirLights[2].specular = glm::vec3(0.05f);
	m_dirLights[2].focalStrength = 16.0f;
	m_dirLights[2].specularIntensity = 0.20f;

	// Light 3 – Back Fill (neutral, subtle)
	m_dirLights[3].direction = glm::normalize(glm::vec3(-0.683f, 0.259f, 0.683f));
	m_dirLights[3].ambient = glm::vec3(0.04f);
	m_dirLights[3].diffuse = glm::vec3(0.12f);
	m_dirLights[3].specular = glm::vec3(0.0f);
	m_dirLights[3].focalStrength = 8.0f;
	m_dirLights[3].specularIntensity = 0.05f;
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
	GLuint program = m_pShaderManager->GetShaderProgramID();
	glUseProgram(program);

	// Enable lighting in the shader
	m_pShaderManager->setIntValue("bUseLighting", true);

	// Lambda helper to upload a single light's data to the shader
	auto setLight = [&](int index, const DIRECTIONAL_LIGHT& light)
	{
		std::string base = "lightSources[" + std::to_string(index) + "]";

		// Convert directional light to distant point light position
		// Negate and scale direction vector to place light far away
		glm::vec3 position = -glm::normalize(light.direction) * 1e6f;

		m_pShaderManager->setVec3Value((base + ".position").c_str(), position);
		m_pShaderManager->setVec3Value((base + ".ambientColor").c_str(), light.ambient);
		m_pShaderManager->setVec3Value((base + ".diffuseColor").c_str(), light.diffuse);
		m_pShaderManager->setVec3Value((base + ".specularColor").c_str(), light.specular);
		m_pShaderManager->setFloatValue((base + ".focalStrength").c_str(), light.focalStrength);
		m_pShaderManager->setFloatValue((base + ".specularIntensity").c_str(), light.specularIntensity);
	};

	// Upload all lights using the helper
	for (int i = 0; i < NUM_DIR_LIGHTS; ++i)
	{
		setLight(i, m_dirLights[i]);
	}
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
	woodMaterial.ambientColor = glm::vec3(0.4f, 0.3f, 0.2f);
	woodMaterial.ambientStrength = 0.2f;
	woodMaterial.diffuseColor = glm::vec3(0.6f, 0.4f, 0.2f);
	woodMaterial.specularColor = glm::vec3(0.3f, 0.3f, 0.3f);
	woodMaterial.shininess = 16.0;  // Low-medium for natural wood
	woodMaterial.tag = "wood";
	m_objectMaterials.push_back(woodMaterial);

	// Marble material for mug body
	OBJECT_MATERIAL marbleMaterial;
	marbleMaterial.ambientColor = glm::vec3(0.3f, 0.3f, 0.3f);
	marbleMaterial.ambientStrength = 0.3f;
	marbleMaterial.diffuseColor = glm::vec3(0.6f, 0.6f, 0.6f);
	marbleMaterial.specularColor = glm::vec3(0.8f, 0.8f, 0.8f);
	marbleMaterial.shininess = 64.0;  // High for polished marble
	marbleMaterial.tag = "marble";
	m_objectMaterials.push_back(marbleMaterial);

	// Ceramic material for mug handle
	OBJECT_MATERIAL ceramicMaterial;
	ceramicMaterial.ambientColor = glm::vec3(0.4f, 0.4f, 0.4f);
	ceramicMaterial.ambientStrength = 0.2f;
	ceramicMaterial.diffuseColor = glm::vec3(0.8f, 0.8f, 0.8f);
	ceramicMaterial.specularColor = glm::vec3(0.4f, 0.4f, 0.4f);
	ceramicMaterial.shininess = 32.0;  // Medium for matte ceramic
	ceramicMaterial.tag = "ceramic";
	m_objectMaterials.push_back(ceramicMaterial);

	// Concrete material for mug base
	OBJECT_MATERIAL concreteMaterial;
	concreteMaterial.ambientColor = glm::vec3(0.2f, 0.2f, 0.2f);
	concreteMaterial.ambientStrength = 0.3f;
	concreteMaterial.diffuseColor = glm::vec3(0.4f, 0.4f, 0.4f);
	concreteMaterial.specularColor = glm::vec3(0.1f, 0.1f, 0.1f);
	concreteMaterial.shininess = 8.0;  // Very low for rough concrete
	concreteMaterial.tag = "concrete";
	m_objectMaterials.push_back(concreteMaterial);
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
	glm::vec3 scaleXYZ = glm::vec3(20.0f, 0.5f, 20.0f);  // 20x20 surface with 0.5 unit thickness
	float XrotationDegrees = 0.0f;  // Flat on ground
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ = glm::vec3(0.0f, -0.25f, 0.0f);  // Lowered so top is at Y=0

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
	// Apply grey marble texture to mug body (Part 1 of cohesive multi-textured object)
	SetShaderTexture("marble");
	SetTextureUVScale(1.0f, 1.0f);  // Standard 1:1 cylindrical mapping
	SetShaderMaterial("marble");

	// Set transformations for mug body
	glm::vec3 scaleXYZ = glm::vec3(1.2f, 3.0f, 1.2f);  // Taller than wide
	float XrotationDegrees = 0.0f;  // Upright cylinder
	float YrotationDegrees = 0.0f;
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
	SetTextureUVScale(1.0f, 1.0f);  // Standard mapping
	SetShaderMaterial("ceramic");

	// Set transformations for mug interior (smaller diameter creates wall thickness)
	glm::vec3 scaleXYZ = glm::vec3(1.08f, 2.7f, 1.08f);  // Smaller diameter than outer (1.2) creates visible wall thickness
	float XrotationDegrees = 0.0f;  // Upright cylinder
	float YrotationDegrees = 0.0f;
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
	// Use brown color for coffee (no texture)
	SetShaderColor(0.4f, 0.25f, 0.15f, 1.0f);  // Rich brown coffee color

	// Set transformations for coffee (high enough to hide handle interior)
	glm::vec3 scaleXYZ = glm::vec3(1.06f, 2.6f, 1.06f);  // Slightly smaller than interior (1.08), very full mug
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ = glm::vec3(0.0f, 1.3f + m_mugVerticalOffset, 0.0f);  // Higher position for fuller coffee level

	// Apply transformations
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
		ZrotationDegrees, positionXYZ);

	// Draw coffee cylinder with top cap (liquid surface)
	m_basicMeshes->DrawCylinderMesh(true, true, true);  // Yes top, yes bottom, yes sides
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
	// Apply pale wall texture to handle (Part 2 of cohesive multi-textured object)
	SetShaderTexture("pale_wall");
	SetTextureUVScale(1.0f, 1.0f);  // Standard mapping
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
	// Apply cracked cement texture to base (Part 3 of cohesive multi-textured object)
	SetShaderTexture("cement");
	SetTextureUVScale(1.0f, 1.0f);  // Standard mapping
	SetShaderMaterial("concrete");

	// Set transformations for mug base rim
	glm::vec3 scaleXYZ = glm::vec3(0.95f, 1.0f, 0.95f);  // Smaller than mug body (1.2f), thicker
	float XrotationDegrees = 90.0f;  // Flat on ground
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ = glm::vec3(0.0f, 0.25f, 0.0f);  // Slightly higher above plane

	// Apply transformations (Scale → Rotate → Translate)
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
		ZrotationDegrees, positionXYZ);

	// Draw the torus
	m_basicMeshes->DrawTorusMesh();
}
