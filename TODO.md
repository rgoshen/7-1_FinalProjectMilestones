# Milestone Five Lighting Implementation Plan

## Phase 0: Guardrails (once)
- [x] Create a new branch: feat/m5-lighting (already done by user)
- [ ] Do not rename or remove any existing uniforms or structs
- [ ] Do not rewrite fragmentShader.glsl
- [ ] Only touch:
  - vertexShader.glsl (one small fix)
  - SceneManager.h/.cpp (add light uploader + call site)

---

## Phase 1: SceneManager (C++ side)
**Goal**: Upload four lights every frame using existing shader uniforms and keep lighting on.

### Tasks:
- [ ] Add `DefineLightsAndUpload()` declaration to SceneManager.h
- [ ] Implement `DefineLightsAndUpload()` in SceneManager.cpp with:
  - Lambda helper `setLight` function
  - Set `bUseLighting` to true
  - Light 0 – Sun (warm) e=80°, a=45° → (0.123, 0.985, 0.123)
  - Light 1 – Sky (cool, opposite)
  - Light 2 – Wall bounce (warm side)
  - Light 3 – Back fill (neutral)
  - Position calculation: `-glm::normalize(dirUnit) * 1e6f`
- [ ] Call `DefineLightsAndUpload()` at top of `RenderScene()`
- [ ] **COMMIT** after Phase 1 complete

---

## Phase 2: Vertex Shader (GLSL)
**Goal**: Fix normals for correct lighting on rotated/scaled geometry.

### Tasks:
- [ ] Replace `fragmentVertexNormal = inVertexNormal;` with:
  ```glsl
  mat3 normalMatrix = mat3(transpose(inverse(mat3(model))));
  fragmentVertexNormal = normalize(normalMatrix * inVertexNormal);
  ```
- [ ] Keep all existing in/out variable names unchanged
- [ ] **COMMIT** after Phase 2 complete

---

## Phase 3: Fragment Shader (GLSL) — Verify, don't rewrite
**Goal**: Verify shader meets expectations (NO EDITS REQUIRED)

### Verification Checklist:
- [ ] Uses `uniform bool bUseLighting;`
- [ ] Uses `uniform bool bUseTexture;`
- [ ] Uses `uniform sampler2D objectTexture;`
- [ ] Uses `uniform vec3 viewPosition;`
- [ ] Uses `uniform vec2 UVscale;`
- [ ] Has `struct LightSource` with correct fields
- [ ] Has `#define TOTAL_LIGHTS 4`
- [ ] Has `uniform LightSource lightSources[TOTAL_LIGHTS];`
- [ ] Sums all 4 lights in a loop
- [ ] Uses `fragmentPosition`, `fragmentVertexNormal`, `fragmentTextureCoordinate`
- [ ] Attenuation effectively disabled or not used (distant positions emulate directional)

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
