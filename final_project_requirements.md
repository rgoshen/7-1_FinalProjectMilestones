# CS 330 Final Project Requirements - Part 1: 3D Scene Code

## 📋 Project Overview

**Project Title:** 7-1 Final Project and Milestones  
**Course:** CS 330 - Computer Graphics and Visualization  
**Developer:** Rick Goshen  
**Company:** Triangle and Cube Studios  
**Objective:** Create a 3D representation of a 2D reference image using C++ and OpenGL

---

## 🎯 Total Points for Part 1: 90 points

---

## ✅ Requirement 1: 3D Objects (15 points)

### Criteria
- **Minimum 4 completed objects** must be present in the 3D scene
- **At least one object** must be constructed using **two or more primitive shapes**
- Create **low-polygon 3D representations** of real-world objects
- Use **organized geometry** with well-spaced and connected polygons (triangles)
- **Keep polygon count under 1,000 triangles total** for each individual object
- Objects should have **professional presentation** that replicates the 2D reference image

### Performance Levels
| Level | Score | Description |
|-------|-------|-------------|
| **Exceeds Expectations** | 100% (15pts) | Particularly sophisticated or creative 3D modeling; excellent polygon management; innovative approach to objects |
| **Meets Expectations** | 85% (12.75pts) | Creates accurate low-polygon 3D representations matching reference image with organized geometry |
| **Partially Meets** | 55% (8.25pts) | Errors in geometry organization; excessive polygon counts; poor object construction; missing required objects |
| **Does Not Meet** | 0% | Does not attempt criterion |

### Technical Requirements
- Available primitive shapes:
  - Box
  - Cone
  - Cylinder
  - Plane
  - Prism
  - Pyramid
  - Sphere
  - Tapered cylinder
  - Torus

---

## ✅ Requirement 2: Perspective and Orthographic Views (10 points)

### Criteria
- **Create code that allows switching** between perspective and orthographic projection views
- Users must be able to **toggle between both views during runtime**
- Both projection modes must render the scene correctly

### Performance Levels
| Level | Score | Description |
|-------|-------|-------------|
| **Exceeds Expectations** | 100% (10pts) | Particularly well-executed view switching with smooth transitions; exceptional implementation |
| **Meets Expectations** | 85% (8.5pts) | Creates functional ability to switch between perspective and orthographic views |
| **Partially Meets** | 55% (5.5pts) | View switching incomplete; only one projection type works; implementation errors |
| **Does Not Meet** | 0% | Does not attempt criterion |

### Implementation Notes
- Typically implemented with keyboard toggle (e.g., P key for Perspective, O key for Orthographic)
- Must maintain proper aspect ratios in both modes
- Camera position/orientation should remain consistent during toggle

---

## ✅ Requirement 3: Navigation (10 points)

### Criteria
- **Create code that allows user navigation** through the 3D scene
- Users must be able to **move through and around objects** using input controls
- Navigation should be intuitive and responsive

### Performance Levels
| Level | Score | Description |
|-------|-------|-------------|
| **Exceeds Expectations** | 100% (10pts) | Particularly intuitive or sophisticated navigation system; creative control schemes |
| **Meets Expectations** | 85% (8.5pts) | Creates functional navigation allowing movement throughout the scene |
| **Partially Meets** | 55% (5.5pts) | Navigation incomplete, limited, or difficult to use; poor responsiveness |
| **Does Not Meet** | 0% | Does not attempt criterion |

### Implementation Notes
- Common controls include:
  - WASD for directional movement
  - Q/E for vertical movement
  - Mouse for camera rotation (look around)
  - Mouse scroll for speed adjustment
- Must support 3D movement in all directions
- Camera should respond smoothly to input

---

## ✅ Requirement 4: Object Transformations (10 points)

### Criteria
- Apply **appropriate transformations** to all objects in the scene
- Objects must be **correctly scaled** relative to each other
- Objects must be **correctly positioned** in the scene
- Objects must be **correctly rotated** for proper orientation
- Transformations must **accurately replicate the 2D reference image**

### Performance Levels
| Level | Score | Description |
|-------|-------|-------------|
| **Exceeds Expectations** | 100% (10pts) | Excellent spatial relationships; precise positioning; innovative approach to transformations |
| **Meets Expectations** | 85% (8.5pts) | Applies proper transformations (scale, rotation, position) matching reference image |
| **Partially Meets** | 55% (5.5pts) | Scaling/positioning/rotation errors; poor spatial relationships; inconsistent with reference |
| **Does Not Meet** | 0% | Does not attempt criterion |

### Technical Implementation
- Use GLM transformation matrices:
  - `glm::scale()` for object sizing
  - `glm::rotate()` for object orientation
  - `glm::translate()` for object positioning
- Apply transformations in correct order: Scale → Rotate → Translate
- Maintain relative proportions between objects as shown in reference image

---

## ✅ Requirement 5: Textures (15 points)

### Criteria
- Create textures that are **appropriate and add realism** to objects
- **At least one texture** must use **texture coordinates to display only a portion** of the texture
- **At least one texture** must be **tiled multiple times across an object** (COMPLEX TECHNIQUE REQUIREMENT)
- Textures should **enhance visual fidelity** of the scene

### Performance Levels
| Level | Score | Description |
|-------|-------|-------------|
| **Exceeds Expectations** | 100% (15pts) | Creative texture application; professional appearance; excellent complex technique execution |
| **Meets Expectations** | 85% (12.75pts) | Appropriate texture choices with at least one tiled and one partial texture implemented |
| **Partially Meets** | 55% (8.25pts) | Missing tiling requirement; missing partial texture; poor texture choices; improper mapping |
| **Does Not Meet** | 0% | Does not attempt criterion |

### Technical Implementation
- **Tiled Texture Example:** Set UV scale > 1.0 (e.g., 6.0 for 6x6 tiling)
  ```cpp
  SetTextureUVScale(6.0f, 6.0f); // Tiles texture 6 times in each direction
  ```
- **Partial Texture Example:** Adjust UV coordinates to show only part of image
- Use `stb_image.h` library for texture loading
- Support common formats: JPG, PNG
- Implement proper texture wrapping (GL_REPEAT for tiling)
- Use mipmaps for better quality at distance

---

## ✅ Requirement 6: Lighting (15 points)

### Criteria
- Apply **at least two light sources** to the 3D scene
- Lighting must **enhance realism and depth perception**
- **No objects should be in complete shadow** (all objects must be clearly visible)
- All objects should be **properly illuminated** from multiple angles

### Performance Levels
| Level | Score | Description |
|-------|-------|-------------|
| **Exceeds Expectations** | 100% (15pts) | Sophisticated lighting setup; excellent scene illumination; creative use of multiple lights; particularly well-executed lighting effects |
| **Meets Expectations** | 85% (12.75pts) | Applies at least two light sources with no objects in complete shadow; proper illumination throughout scene |
| **Partially Meets** | 55% (8.25pts) | Insufficient lighting; objects in complete shadow; single light source only; poor color choices |
| **Does Not Meet** | 0% | Does not attempt criterion |

### Technical Implementation
- Implement **Phong lighting model** with:
  - Ambient component (base lighting level)
  - Diffuse component (directional light interaction)
  - Specular component (shininess/reflections)
- Light types to consider:
  - Directional lights (simulate sun/distant sources)
  - Point lights (simulate bulbs/local sources)
  - Spotlights (focused cone of light)
- Define materials for each object:
  - Ambient color and strength
  - Diffuse color
  - Specular color
  - Shininess value

### Critical Note
⚠️ **INSTRUCTOR FEEDBACK:** Scene may need additional light sources to eliminate dark areas and improve overall illumination. Ensure fill lights are bright enough to prevent harsh shadows.

---

## ✅ Requirement 7: Shader Program (10 points)

### Criteria
- Create a **custom shader program** with both vertex and fragment shaders
- Shaders must be **actively used in rendering** the 3D objects
- Shader code must be in **separate GLSL files** (not hardcoded strings in C++)
- Shaders should properly handle lighting calculations

### Performance Levels
| Level | Score | Description |
|-------|-------|-------------|
| **Exceeds Expectations** | 100% (10pts) | Sophisticated shader implementation; creative visual effects; advanced techniques |
| **Meets Expectations** | 85% (8.5pts) | Creates functional shader program for rendering with proper lighting calculations |
| **Partially Meets** | 55% (5.5pts) | Shader incomplete; improperly implemented; hardcoded in C++; missing lighting support |
| **Does Not Meet** | 0% | Does not attempt criterion |

### Technical Implementation
- **Vertex Shader (vertexShader.glsl):**
  - Transform vertex positions (model, view, projection matrices)
  - Pass texture coordinates to fragment shader
  - Calculate lighting data (normals, positions for fragment shader)
  
- **Fragment Shader (fragmentShader.glsl):**
  - Sample textures
  - Calculate Phong lighting (ambient + diffuse + specular)
  - Apply material properties
  - Output final pixel color

- Load shaders using ShaderManager class
- Compile and link shader program
- Pass uniforms for matrices, lights, materials, textures

---

## ✅ Requirement 8: Best Practices (5 points)

### Criteria
- Apply **coding best practices** in formatting, commenting, and functional logic
- Code should follow **industry-standard formatting**:
  - Consistent indentation (tabs or spaces)
  - Proper spacing around operators
  - Logical line breaks
- Code should be **well-commented** with descriptive comments:
  - Function descriptions
  - Complex logic explanations
  - Variable purpose clarification
- Code should be **easy to read and follow**
- Use **logical encapsulation into functions** for reusability and efficiency

### Performance Levels
| Level | Score | Description |
|-------|-------|-------------|
| **Exceeds Expectations** | 100% (5pts) | Exceptional code organization; comprehensive comments; excellent functional encapsulation; shows strong attention to best practices |
| **Meets Expectations** | 85% (4.25pts) | Applies proper formatting, commenting, and functional logic; code is readable and maintainable |
| **Partially Meets** | 55% (2.75pts) | Poor formatting; missing/inadequate comments; code duplication; lack of function encapsulation |
| **Does Not Meet** | 0% | Does not attempt criterion |

### Best Practices Checklist
- [ ] Functions have descriptive header comments
- [ ] Complex algorithms have inline explanatory comments
- [ ] Variables use meaningful names (not single letters except for loop counters)
- [ ] Consistent indentation throughout
- [ ] No duplicate code (use functions instead)
- [ ] Logical function organization (related functions grouped together)
- [ ] Proper C++ naming conventions:
  - `PascalCase` for classes
  - `camelCase` for functions and variables
  - `UPPER_CASE` for constants
- [ ] Appropriate use of const where applicable
- [ ] No magic numbers (use named constants)

---

## 🎯 Quick Reference Checklist

Before final submission, verify:

### Objects & Scene
- [ ] At least 4 distinct 3D objects present
- [ ] At least one object uses 2+ primitive shapes (complex object)
- [ ] All objects have polygon count under 1,000 triangles
- [ ] Objects match 2D reference image layout
- [ ] All objects properly scaled relative to each other
- [ ] All objects properly positioned
- [ ] All objects properly rotated/oriented

### Textures
- [ ] All objects have appropriate textures applied
- [ ] At least one texture is TILED (UV scale > 1.0)
- [ ] At least one texture uses partial coordinates
- [ ] Textures enhance realism and visual quality

### Lighting
- [ ] At least TWO light sources implemented
- [ ] NO objects in complete shadow (all objects visible)
- [ ] Lighting enhances depth and realism
- [ ] Phong lighting model properly implemented
- [ ] Materials defined for all objects

### Camera & Controls
- [ ] Perspective view works correctly (P key)
- [ ] Orthographic view works correctly (O key)
- [ ] WASD navigation functional
- [ ] Q/E vertical movement functional (in perspective mode)
- [ ] Mouse look/rotation functional
- [ ] Mouse scroll speed adjustment functional
- [ ] ESC key exits application

### Shaders
- [ ] Custom vertex shader in separate GLSL file
- [ ] Custom fragment shader in separate GLSL file
- [ ] Shaders properly handle transformations
- [ ] Shaders properly handle lighting calculations
- [ ] Shaders properly handle texture sampling

### Code Quality
- [ ] Code follows consistent formatting standards
- [ ] Functions have descriptive header comments
- [ ] Complex logic has explanatory comments
- [ ] No duplicate code (proper function encapsulation)
- [ ] Variables use meaningful names
- [ ] Code compiles without errors
- [ ] Code executes without crashes

### Submission Package
- [ ] ZIP file contains all source files (.cpp, .h)
- [ ] ZIP file contains Visual Studio project files
- [ ] ZIP file contains GLSL shader files
- [ ] ZIP file contains EXECUTABLE (.exe) ⚠️ REQUIRED
- [ ] EXE has been tested and runs correctly
- [ ] ZIP file is properly named

---

## 📊 Points Summary

| Requirement | Points | Status |
|------------|--------|--------|
| 3D Objects | 15 | ☐ |
| Perspective/Orthographic Views | 10 | ☐ |
| Navigation | 10 | ☐ |
| Object Transformations | 10 | ☐ |
| Textures | 15 | ☐ |
| Lighting | 15 | ☐ |
| Shader Program | 10 | ☐ |
| Best Practices | 5 | ☐ |
| **TOTAL PART 1** | **90** | |

---

## 📚 Supporting Resources

### Official Course Materials
- **Applying Lighting to a 3D Scene** - Phong lighting implementation guide
- **Visual Studio Export Tutorial** - How to create submission ZIP folder
- **Sourcing Textures Tutorial** - Finding free, open-source texture images

### OpenGL References
- GLFW Documentation - Window management and input handling
- GLEW Documentation - OpenGL extension loading
- GLM Documentation - Mathematics library for transformations
- LearnOpenGL.com - Comprehensive OpenGL tutorials

### Texture Sources
- Use free, open-source textures only
- Ensure proper licensing for any external resources
- Cite texture sources if required

---

## ⚠️ Critical Reminders

1. **The EXE file is MANDATORY** - Your project cannot be graded without it
2. **Test your EXE before submission** - Make sure it runs on a clean system
3. **No complete shadows allowed** - All objects must be visible with proper lighting
4. **Tiling is a complex technique requirement** - Don't forget to implement texture tiling
5. **Both projection modes must work** - Test perspective and orthographic thoroughly
6. **Polygon count matters** - Keep each object under 1,000 triangles
7. **Code quality is graded** - Don't skimp on comments and formatting

---

**End of Part 1 Requirements**

*For Part 2 requirements (Design Decisions Document), see separate documentation.*