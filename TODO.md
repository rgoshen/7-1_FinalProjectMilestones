# TODO - CS 330 Final Project Completion

**Current Status:** 6 objects completed (mug, sphere, keyboard, touchpad, table, monitor) + 5-light system.

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
- [x] Commit changes
- [x] Merge to main (via PR #4)

---

## Phase 2: Add Keyboard Object
**Branch:** `feature/add-keyboard`

- [x] Create feature branch from main
- [x] Create `RenderKeyboard()` method (uses existing box mesh)
- [x] Define dark tech material in `DefineObjectMaterials()`
- [x] Add texture for keyboard (using solid colors - texture caused crashes)
- [x] Position at center desk (x:0, z:4.0 for proper spacing)
- [x] Scale as thin rectangle (7.0w x 0.10h x 3.0d frame, 6.8w x 0.05h x 2.8d keys)
- [x] Call `RenderKeyboard()` in `RenderScene()`
- [x] Test build and visual verification
- [x] Update SUMMARY.md with implementation notes
- [x] Commit changes
- [x] Merge to main

---

## Phase 3: Add Touchpad Object
**Branch:** `feature/add-touchpad`

- [x] Create feature branch from main
- [x] Create `RenderTouchpad()` method (uses existing box mesh)
- [x] Define touchpad material (reuses keyboard plastic material)
- [x] Position to right of keyboard (2.0×2.0 square, smaller than keyboard)
- [x] Two-layer design: black frame + minimally raised lighter surface
- [x] Call `RenderTouchpad()` in `RenderScene()`
- [x] Test build and visual verification (5 objects total now)
- [x] Update SUMMARY.md with implementation notes
- [x] Commit changes
- [x] Merge to main

---

## Phase 4: Enhanced Scene Lighting
**Branch:** `feature/enhance-lighting`

- [x] Create feature branch from main
- [x] Update `NUM_DIR_LIGHTS` to 5 in `SceneManager.h`
- [x] Update `TOTAL_LIGHTS` to 5 in `fragmentShader.glsl`
- [x] Add Light 4 (overhead indoor) definition in `DefineLights()`
- [x] Brighten Light 1 diffuse: (0.22, 0.26, 0.34) → (0.38, 0.42, 0.50)
- [x] Brighten Light 1 ambient: 0.02 → 0.04
- [x] Brighten Light 2 diffuse: (0.14, 0.12, 0.10) → (0.28, 0.24, 0.20)
- [x] Brighten Light 2 ambient: 0.01 → 0.02
- [x] Brighten Light 3 diffuse: 0.08 → 0.18
- [x] Brighten Light 3 ambient: 0.01 → 0.02
- [x] Adjust Light 0 for sunset: lower elevation (~35°), add warm orange/red tones
- [x] Test build and visual verification (no dark shadows)
- [x] Update SUMMARY.md with lighting rationale
- [x] Commit changes
- [x] Merge to main

---

## Phase 4a: Add Monitor Object
**Branch:** `feature/add-monitor`

- [x] Create feature branch from main
- [x] Load cone mesh in `PrepareScene()` for connector
- [x] Create `RenderMonitor()` orchestrator method with five components:
  - [x] Monitor screen (thin box, 16:9 aspect ratio 7.6w x 4.2h x 0.25d)
  - [x] Monitor frame (thin box, 16:9 aspect ratio 8.0w x 4.5h x 0.3d)
  - [x] Stand pole (rectangular box, vertical support 0.5w x 5.075h x 0.2d)
  - [x] Stand base (flattened sphere 1.8 diameter x 0.15 height)
  - [x] Connector (horizontal cylinder 0.15 diameter x 0.35 length)
- [x] Define monitor materials in `DefineObjectMaterials()`:
  - [x] Glossy black screen material (shininess 64.0 for high gloss)
  - [x] Semi-gloss plastic frame material (shininess 28.0)
  - [x] Matte plastic for base/pole/connector (reused existing material)
- [x] Position monitor behind mug (Z=-5.8 to -5.3)
  - [x] Screen/Frame: centered at Y=4.25, Z=-5.35/-5.30
  - [x] Pole: extends from base to 3/4 monitor height
  - [x] Base: on desk surface at Z=-5.8
  - [x] Connector: bridges pole to monitor back at Z=-5.684
- [x] Refactored to component methods (RenderMonitorBase, Pole, Connector, Frame, Screen)
- [x] Call `RenderMonitor()` in `RenderScene()`
- [x] Test build and visual verification
- [x] Update SUMMARY.md with implementation notes
- [x] Commit changes
- [x] Merge to main

---

## Phase 5: Scene Composition Adjustments
**Branch:** `refactor/scene-adjustments`

- [x] Create feature branch from current state
- [x] Refactor RenderMug pattern to match RenderMonitor
  - [x] Create RenderMug() method that calls component methods
  - [x] Update RenderScene() to call RenderMug() instead of individual components
  - [x] Maintains same pattern: orchestrator function calls sub-components
- [x] Adjust mug size and position to match reference image
  - [x] Scale down by 25% (all components: body, interior, coffee, handle, base)
  - [x] Reposition to X=-5.0, Z=1.0 (left and forward for depth layering)
- [x] Adjust monitor position and scale to match reference image
  - [x] Increase width: 8.0 → 11.0 (maintaining 16:9 aspect ratio)
  - [x] Extend pole height for taller monitor: 5.075 → 6.340625
  - [x] Move forward: Z=-5.8/-5.3 → Z=-2.5/-2.0 (closer to keyboard)
  - [x] Increase base size: 1.8 → 2.5 diameter
- [x] Adjust keyboard position and scale to match reference image
- [x] Adjust touchpad position and scale to match reference image
- [x] Adjust sphere position and scale to match reference image
- [x] Review overall scene composition and proportions
- [x] Verify camera default position provides good initial view
- [x] Test navigation to ensure all objects are easily viewable
- [x] Verify no visual collisions or overlaps from multiple angles
- [x] Ensure foreground/midground/background layering is clear
- [x] Update SUMMARY.md with adjustment rationale
- [x] Commit changes
- [x] Merge to main

---

## Phase 5a: Add Background Wall
**Branch:** `feature/add-wall`

- [x] Create feature branch from main
- [x] Create `RenderWall()` method in SceneManager
- [x] Add wall declaration to SceneManager.h
- [x] Position wall behind monitor at back table edge (Z=-10)
- [x] Use large box (25w x 13h x 1.0d - wider than table, ground to upper background)
- [x] Apply pale_wall.jpg texture with 2x2 UV scale
- [x] Add wall material to `DefineObjectMaterials()` (ceramic material values)
- [x] Call `RenderWall()` in `RenderScene()` (first, before all objects)
- [x] Test build and visual verification
- [x] Verify monitor visibility improved against light background
- [x] Update SUMMARY.md with wall implementation notes
- [x] Commit changes
- [x] Merge to main

**Rationale:** Dark monitor against black background lacks contrast. Light-colored wall will make monitor stand out clearly and add realism to desk workspace setting.

---

## Phase 6: Verify Partial Texture Requirement
**Branch:** `feature/partial-texture` (if needed)

- [ ] Audit current UV coordinates for all textured objects
- [ ] Verify at least one texture uses partial UV coords (not 0.0-1.0 range)
- [ ] If missing: modify one object to use partial texture mapping
- [ ] Test and verify texture displays correctly
- [ ] Update SUMMARY.md
- [ ] Commit and merge (if changes made)

---

## Phase 7: Code Quality & Comments
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

## Phase 8: Final Validation & Build
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
