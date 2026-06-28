#  Village Fair — OpenGL 3.3 Interactive 3D Scene

A real-time interactive 3D simulation of a festive Indian village fair (মেলা), 
built with OpenGL 3.3 Core Profile in C++. Explore a richly detailed carnival 
environment featuring rides, stalls, decorations, and interactive mini-games.

---

## Demo

> https://youtu.be/54HNsVIwGT0?si=W2o81PHDbN5VMO53
---

## Features

### Scene Elements
| Component | Description |
|---|---|
| **Banyan Tree** | Procedurally generated with recursive branching, 60 festive light bulbs across 4 sectors, and 1000s of instanced leaves |
| **Ferris Wheel** | 8 gondolas with smooth acceleration/deceleration; gondolas stay upright during rotation |
| **Swing Ride** | Rotating platform with 8 arms and dual hanging seats; momentum-based physics |
| **Stage** | Cultural performance stage with marigold garlands, hanging stars, lanterns, bobo balloons, and patterned floor mats |
| **Ring Toss** | Playable carnival game — aim and throw rings at 15 bottles with gravity and collision detection |
| **Bioscope** | Vintage coin-operated viewer with 10-frame animation playback and first-person camera takeover |
| **Vendor Stalls** | 5 variants: Food, Toy, Pottery, Tea, and Saree — each with unique props, fairy lights, and point lights |
| **Ice Cream Corner** | Booth with scoop displays, popcorn stand, burger dome, curved umbrella, and seating |
| **Cotton Candy Cart** | Mobile cart with animated candy puffs and rotating machine |
| **Gate & Fence** | Arch entrance gate with bunting; bamboo fence with 12 pole-lamp point lights |
| **Skybox** | Day/night switchable cubemap environment |

### Technical Highlights
- **40+ point lights** across the scene with per-light toggles
- Phong shading with directional + point light support
- Instanced rendering for fence posts and leaf particles
- Bezier and B-spline surface generation
- Emission maps for self-lit decorations (lanterns, bulbs, stars)
- 4x MSAA anti-aliasing
- Physics-based ring toss with gravity and cylindrical collision detection
- Day/night mode toggling lighting parameters and skybox
- Texture fallback system (40+ textures)
- Frame-independent animation via `deltaTime`

---

## Controls

### Camera
| Key | Action |
|---|---|
| `W` / `S` | Move forward / backward |
| `A` / `D` | Strafe left / right |
| `E` / `R` | Move up / down |
| `Mouse` | Look around |
| `Scroll Wheel` | Zoom |
| `X` / `U` | Pitch up / down |
| `C` / `Q` | Yaw left / right |
| `Z` | Roll |

### Rides & Interactions
| Key | Action |
|---|---|
| `V` | Toggle Ferris Wheel rotation |
| `B` | Toggle Swing Ride |
| `T` | Enter Bioscope view (must be within 8 units) |
| `N` | Exit Bioscope view |
| `Space` | Throw ring (Ring Toss game) |
| `Arrow Keys` | Aim ring / rotate camera |

### Lighting
| Key | Action |
|---|---|
| `0` | Toggle Day / Night |
| `1` – `4` | Toggle Banyan Tree sector lights (4 sectors) |
| `5` | Toggle ambient light |
| `6` | Toggle diffuse light |
| `7` | Toggle specular light |
| `8` | Toggle directional light |
| `9` | Toggle point lights |
| `L` | Toggle fairy lights |
| `K` | Toggle main lanterns |




## Dependencies

| Library | Purpose | Version |
|---|---|---|
| [GLFW](https://www.glfw.org/) | Window creation & input | 3.3+ |
| [GLAD](https://glad.dav1d.de/) | OpenGL function loader | 0.1.36+ |
| [GLM](https://github.com/g-truc/glm) | Math (vectors, matrices) | 0.9.9+ |
| [stb_image](https://github.com/nothings/stb) | Texture loading | Included |
| OpenGL | Graphics API | 3.3 Core Profile |

---

## Building (Windows / Visual Studio)

### Prerequisites
1. **Visual Studio 2022** with the *Desktop development with C++* workload
2. Download and place libraries at the expected paths (or update the `.vcxproj`):
   - `C:\Users\<You>\Desktop\opengl\Include\` — GLFW & GLAD headers
   - `C:\Users\<You>\Desktop\opengl\Lib\` — `glfw3.lib`
   - `C:\Users\<You>\Desktop\glm\` — GLM headers

### Steps
```bash
# 1. Clone the repository

# 2. Open the solution in Visual Studio
start g_project.sln

# 3. Set configuration to Release / x64 (or Debug / x64)

# 4. Build → Build Solution  (Ctrl + Shift + B)

# 5. Run the executable from the project directory so relative
#    shader and texture paths resolve correctly
Important: Run the executable from the project root directory.

Shaders are read from shaders/ and textures from the working directory.

Architecture Overview

main.cpp
  └── Initializes OpenGL context (GLFW + GLAD)
  └── Compiles 3 shader programs (scene, leaf_instanced, skybox)
  └── Loads 40+ textures
  └── Instantiates all scene objects
  └── Main render loop
        ├── processInput()          ← keyboard/mouse
        ├── [object].draw(shader)   ← each scene component
        └── glfwSwapBuffers()
Each scene object is a self-contained header-only class that:

Builds its own VAO/VBO on construction
Exposes a draw(Shader) method called each frame
Manages its own animation state via deltaTime
Rendering Pipeline

Vertex Shader (scene.vert)
  └── MVP transform
  └── Normal transform (normal matrix)
  └── Pass UV, world position, normal to fragment stage

Fragment Shader (scene.frag)
  └── Phong lighting: 1 directional + up to 40 point lights
  └── Texture sampling with fallback
  └── Emission map overlay for self-lit objects
  └── Toggle flags: ambientOn, diffuseOn, specularOn

Authors
naboni — Scene design, OpenGL implementation, all C++ source code

License
This project is for educational/academic purposes.

Third-party libraries (GLFW, GLAD, GLM, stb_image) are subject to their own licenses.




