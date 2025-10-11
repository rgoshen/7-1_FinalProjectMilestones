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

### Tasks

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

### Tasks

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

### Verification Checklist

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

## Phase 3B: Lighting & Material Adjustment Plan (tuning pass)

**Goal**: Fix over-bright mug specular and inverted desk reflection feel; achieve natural sunlight balance without changing shader logic.

### Context Summary

- Mug appears washed out from overly strong specular.
- Desk top should show soft highlight; underside should remain dark.
- Balance ambient/diffuse/specular per Phong model while keeping existing uniforms/structs. :contentReference[oaicite:0]{index=0}

### Tasks (C++ only — keep shaders as-is)

**In `SceneManager::DefineLights()` / `DefineLightsAndUpload()`**

- [x] Tune sunlight intensities:
  - [x] `dirLights[0].diffuse  = glm::vec3(0.90f, 0.88f, 0.83f);`
  - [x] `dirLights[0].specular = glm::vec3(0.70f, 0.70f, 0.70f);`
  - [x] `dirLights[0].ambient  = glm::vec3(0.15f, 0.15f, 0.17f);`
- [x] Ensure fill lights (1–3) have **low/zero specular** and moderate diffuse.
- [x] Confirm `viewPosition` uniform is uploaded each frame (ViewManager.cpp:288). :contentReference[oaicite:1]{index=1}
- [x] Verify sunlight direction has **positive Y** (0.985 - light from above).

**In `DefineObjectMaterials()`**

- [x] Mug / marble (ceramic):
  - [x] `marbleMaterial.specularColor = glm::vec3(0.30f, 0.30f, 0.30f);`
  - [x] `marbleMaterial.shininess = 16.0f;`  // less glossy
- [x] Desk (wood):
  - [x] `woodMaterial.specularColor = glm::vec3(0.10f, 0.08f, 0.05f);`
  - [x] `woodMaterial.shininess = 12.0f;`
  - [x] Keep diffuse > specular to maintain a matte wood feel. :contentReference[oaicite:2]{index=2}

**Underside Light Bleed (if present)**

- [x] Inspect box/plane underside normals; flip if needed (not needed - normals correct).
- [x] During debug only: `glDisable(GL_CULL_FACE);` to visualize; normally **leave culling enabled**.
- [x] Re-check that ambient on fill lights isn't excessive - **FIXED**: Reduced fill light ambient values:
  - Light 1 ambient: 0.12 → 0.03
  - Light 2 ambient: 0.05 → 0.01
  - Light 3 ambient: 0.04 → 0.01
  - Total ambient reduced from ~0.36 to ~0.20 (sunlight still at 0.15) :contentReference[oaicite:3]{index=3}

### Verification (orbit camera)

- [ ] Mug shows texture detail; no blown-out white.
- [ ] Desk top has a soft highlight; underside remains mostly dark.
- [ ] If scene is too flat: nudge **sunlight diffuse** by +0.05 (don’t raise specular first). :contentReference[oaicite:4]{index=4}

### Notes

- Keep Phong components conceptually aligned (ambient/diffuse/specular) and tune intensities/materials—not shader math. :contentReference[oaicite:5]{index=5}
- Maintain four balanced lights and avoid pure white clamping on bright surfaces. :contentReference[oaicite:6]{index=6}

---

## Phase 3C: Mug Washout Fix (material-only pass)

**Goal**: Reveal marble detail on the mug by reducing its effective specular and overall brightness without touching shaders.

### Changes (C++: DefineObjectMaterials() only)

- [x] Set marble material to darker diffuse, lower specular, and softer shininess:
  - [x] `marbleMaterial.ambientColor` = `vec3(0.22f)`
  - [x] `marbleMaterial.ambientStrength = 0.22f`
  - [x] `marbleMaterial.diffuseColor = vec3(0.42f)`
  - [x] `marbleMaterial.specularColor = vec3(0.02f)`
  - [x] `marbleMaterial.shininess = 4.0`
- [x] (Optional safety clamp if still bright) lower the sun's specular punch slightly:
  - [x] `m_dirLights[0].specularIntensity = 0.18` (was higher)
  - [x] Keep fills at 0.0 specular.

### Quick validation steps (user steps)

- Turn `SINGLE_LIGHT_DEBUG = true` and orbit camera:
  - Mug should show visible veins and soft, dim highlight.
  - No pixels should look pure white on the mug.
- Turn it back off and re-check with all lights:
  - Table remains matte; underside stays dark.

### If it’s still washed out (diagnostics — pick one) (user step)

- A. Texture × specular coupling: some shaders accidentally multiply texture color into specular. If yours does, cut specular further:
  - `marbleMaterial.specularColor = vec3(0.01)` and `shininess = 3.0`.
- B. Diffuse too high with a bright albedo: drop diffuse a bit more:
  - `marbleMaterial.diffuseColor = vec3(0.36–0.38)`.
  - C. Camera/view uniform: confirm `viewPosition` is updated every frame (wrong eye pos exaggerates highlights).

### Success criteria

- Marble veins are clearly visible from the default camera.
- Highlight on mug is subtle and never clips to pure white.
- Table lighting and underside behavior remain unchanged.

### Nice-to-have (optional, not required for grading)

- If you want physically nicer color response later, consider sRGB textures (`GL_SRGB8/GL_SRGB8_ALPHA8`) with `GL_FRAMEBUFFER_SRGB` enabled. Leave this for a separate pass.

## Phase 4: Build, Test, Tune

**Goal**: Verify lighting works correctly

### Tasks

- [x] Build the project successfully
- [x] Visual checks (rotate camera):
  - [x] Sunlight appears from high, front-right
  - [x] No areas are pure black
  - [x] Specular highlights move correctly as viewpoint changes
  - [x] Desk plane shows readable specular highlight
- [x] Tuning (if needed):
  - [x] Lift dark areas: nudge Sky.diffuse up slightly
  - [x] Too flat? Reduce fill diffuse, keep sun strong
  - [x] Mug too glossy? Lower specularIntensity or focalStrength
- [x] **COMMIT** after Phase 4 complete

---

## Acceptance Criteria

- [x] Four lights upload correctly every frame (Sun, Sky, Wall, Back)
- [x] `bUseLighting` set to true
- [x] Vertex normals transformed with normal matrix
- [x] Fragment shader unchanged and functioning with TOTAL_LIGHTS=4
- [x] Scene shows natural sunlight and readable reflections on plane
- [x] No crushed blacks; textures remain correct; code compiles cleanly
