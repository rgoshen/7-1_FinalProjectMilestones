# TODO - CS 330 Final Project Completion

**Current Status:** 1 complex object (coffee mug). Need 3 more objects + lighting improvements.

**Strategy:** Incremental development - one feature branch per object/enhancement.

---

## Phase 1: Add Blue Sphere Object
**Branch:** `feature/add-blue-sphere`

- [x] Create feature branch from main
- [x] Load sphere mesh in `PrepareScene()`
- [x] Create `RenderBlueSphere()` method with transformations
- [x] Define rubber material in `DefineObjectMaterials()`
- [x] Load rubber-coating.jpg texture
- [x] Call `RenderBlueSphere()` in `RenderScene()`
- [x] Position at front-left (-3.5, 0.8, 3.5) - Y=0.8 for tangent placement
- [x] Test build and visual verification
- [x] Update SUMMARY.md with implementation notes
- [ ] Commit changes
- [ ] Merge to main

---

## Phase 2: Add Keyboard Object
**Branch:** `feature/add-keyboard`

- [ ] Create feature branch from main
- [ ] Create `RenderKeyboard()` method (uses existing box mesh)
- [ ] Define dark tech material in `DefineObjectMaterials()`
- [ ] Add texture for keyboard (if using textured approach)
- [ ] Position at center desk (~x:0, z:1.5)
- [ ] Scale as thin rectangle (~4.5w x 0.15h x 1.5d)
- [ ] Call `RenderKeyboard()` in `RenderScene()`
- [ ] Test build and visual verification
- [ ] Update SUMMARY.md with implementation notes
- [ ] Commit changes
- [ ] Merge to main

---

## Phase 3: Add Touchpad Object
**Branch:** `feature/add-touchpad`

- [ ] Create feature branch from main
- [ ] Create `RenderTouchpad()` method (uses existing box mesh)
- [ ] Reuse or create material for touchpad
- [ ] Position at right side (~x:4.5, z:1.0)
- [ ] Scale as thin square (~2.0w x 0.08h x 2.5d)
- [ ] Call `RenderTouchpad()` in `RenderScene()`
- [ ] Test build and visual verification (4 objects total now)
- [ ] Update SUMMARY.md with implementation notes
- [ ] Commit changes
- [ ] Merge to main

---

## Phase 4: Enhanced Scene Lighting
**Branch:** `feature/enhance-lighting`

- [ ] Create feature branch from main
- [ ] Update `NUM_DIR_LIGHTS` to 5 in `SceneManager.h`
- [ ] Update `TOTAL_LIGHTS` to 5 in `fragmentShader.glsl`
- [ ] Add Light 4 (overhead) definition in `DefineLights()`
- [ ] Brighten Light 1 diffuse: (0.22, 0.26, 0.34) → (0.38, 0.42, 0.50)
- [ ] Brighten Light 1 ambient: 0.02 → 0.04
- [ ] Brighten Light 2 diffuse: (0.14, 0.12, 0.10) → (0.28, 0.24, 0.20)
- [ ] Brighten Light 2 ambient: 0.01 → 0.02
- [ ] Brighten Light 3 diffuse: 0.08 → 0.18
- [ ] Brighten Light 3 ambient: 0.01 → 0.02
- [ ] Test build and visual verification (no dark shadows)
- [ ] Update SUMMARY.md with lighting rationale
- [ ] Commit changes
- [ ] Merge to main

---

## Phase 5: Verify Partial Texture Requirement
**Branch:** `feature/partial-texture` (if needed)

- [ ] Audit current UV coordinates for all textured objects
- [ ] Verify at least one texture uses partial UV coords (not 0.0-1.0 range)
- [ ] If missing: modify one object to use partial texture mapping
- [ ] Test and verify texture displays correctly
- [ ] Update SUMMARY.md
- [ ] Commit and merge (if changes made)

---

## Phase 6: Code Quality & Comments
**Branch:** `refactor/code-quality`

- [ ] Create feature branch from main
- [ ] Add function header comments to all `Render*()` methods
- [ ] Add inline comments for transformation logic
- [ ] Add inline comments for lighting setup
- [ ] Extract magic numbers to named constants (positions, scales)
- [ ] Extract magic numbers for light values
- [ ] Extract magic numbers for UV scales
- [ ] Review for code duplication
- [ ] Verify consistent formatting (tabs/spacing)
- [ ] Update SUMMARY.md
- [ ] Commit changes
- [ ] Merge to main

---

## Phase 7: Final Validation & Build
**Branch:** N/A (validation only)

- [ ] Build Release configuration (Win32)
- [ ] Test .exe runs without errors
- [ ] Verify 4+ objects visible in scene
- [ ] Verify tiled texture present (oak wood 6x6) ✓
- [ ] Verify partial texture present
- [ ] Verify 5 light sources active
- [ ] Test perspective/orthographic toggle (P/O keys)
- [ ] Test WASD navigation
- [ ] Test Q/E vertical movement
- [ ] Test mouse look
- [ ] Test mouse scroll zoom
- [ ] Visual inspection: no completely dark objects
- [ ] Check for compiler warnings (should be 0)
- [ ] Collect submission files (exe, source, shaders, textures, project files)

---

## Parking Lot (Optional Enhancements)

- [ ] Add pen holder (cone) for 5th object
- [ ] Add smart speaker (cylinder) for 6th object
- [ ] Add monitor with stand
- [ ] Optimize material properties
