# Milestone Five Lighting Implementation Plan

## Phase 0: Guardrails (once)
- [x] Create a new branch: feat/m5-lighting (already done by user)
- [x] Do not rename or remove any existing uniforms or structs
- [x] Do not rewrite fragmentShader.glsl
- [x] Only touch:
  - vertexShader.glsl (one small fix)
  - SceneManager.h/.cpp (add light uploader + call site)

---

## Phase 1: SceneManager (C++ side)
**Goal**: Upload four lights every frame using existing shader uniforms and keep lighting on.

### Tasks:
- [x] Add `DefineLights()` and `UploadLights()` declarations to SceneManager.h
- [x] Implement `DefineLights()` in SceneManager.cpp with:
  - Light 0 – Sun (warm) e=80°, a=45° → (0.123, 0.985, 0.123)
  - Light 1 – Sky (cool, opposite)
  - Light 2 – Wall bounce (warm side)
  - Light 3 – Back fill (neutral)
- [x] Implement `UploadLights()` in SceneManager.cpp with:
  - Lambda helper `setLight` function
  - Set `bUseLighting` to true
  - Position calculation: `-glm::normalize(dirUnit) * 1e6f`
- [x] Call `DefineLights()` in `PrepareScene()`
- [x] Call `UploadLights()` at top of `RenderScene()`
- [x] **COMMIT** after Phase 1 complete

---

## Phase 2: Vertex Shader (GLSL)
**Goal**: Fix normals for correct lighting on rotated/scaled geometry.

### Tasks:
- [x] Replace `fragmentVertexNormal = inVertexNormal;` with:
  ```glsl
  mat3 normalMatrix = mat3(transpose(inverse(mat3(model))));
  fragmentVertexNormal = normalize(normalMatrix * inVertexNormal);
  ```
- [x] Keep all existing in/out variable names unchanged
- [x] **COMMIT** after Phase 2 complete (file outside repo)

---

## Phase 3: Fragment Shader (GLSL) — Verify, don't rewrite
**Goal**: Verify shader meets expectations (NO EDITS REQUIRED)

### Verification Checklist:
- [x] Uses `uniform bool bUseLighting;`
- [x] Uses `uniform bool bUseTexture;`
- [x] Uses `uniform sampler2D objectTexture;`
- [x] Uses `uniform vec3 viewPosition;`
- [x] Uses `uniform vec2 UVscale;`
- [x] Has `struct LightSource` with correct fields
- [x] Has `#define TOTAL_LIGHTS 4`
- [x] Has `uniform LightSource lightSources[TOTAL_LIGHTS];`
- [x] Sums all 4 lights in a loop
- [x] Uses `fragmentPosition`, `fragmentVertexNormal`, `fragmentTextureCoordinate`
- [x] Attenuation effectively disabled or not used (distant positions emulate directional)

---

## Phase 4: Build, Test, Tune
**Goal**: Verify lighting works correctly

### Tasks:
- [ ] Build the project successfully
- [ ] Visual checks (rotate camera):
  - [ ] Sunlight appears from high, front-right
  - [ ] No areas are pure black
  - [ ] Specular highlights move correctly as viewpoint changes
  - [ ] Desk plane shows readable specular highlight
- [ ] Tuning (if needed):
  - [ ] Lift dark areas: nudge Sky.diffuse up slightly
  - [ ] Too flat? Reduce fill diffuse, keep sun strong
  - [ ] Mug too glossy? Lower specularIntensity or focalStrength
- [ ] **COMMIT** after Phase 4 complete

---

## Acceptance Criteria
- [ ] Four lights upload correctly every frame (Sun, Sky, Wall, Back)
- [ ] `bUseLighting` set to true
- [ ] Vertex normals transformed with normal matrix
- [ ] Fragment shader unchanged and functioning with TOTAL_LIGHTS=4
- [ ] Scene shows natural sunlight and readable reflections on plane
- [ ] No crushed blacks; textures remain correct; code compiles cleanly
