// ============================================================
//  Village Fair — OpenGL 3.3 Foundation
//  main.cpp
// ============================================================

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>

#include "shader.h"
#include "camera.h"
#include "BanyanTree.h"
#include "Stage.h"
#include "Fence.h"
#include "Gate.h"
#include "FerrisWheel.h"
#include "SwingRide.h"
#include "Stall.h"
#include "IceCreamCorner.h"
#include "CottonCandyCart.h"
#include "RingToss.h"
#include "Bioscope.h"
#include "Skybox.h"
#include "stb_image.h"

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

Camera camera(glm::vec3(0.0f, 3.5f, 33.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f); // Balanced distance and height for perfect framing
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool  firstMouse = true;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Bioscope Animation Globals
bool bioscopeIsPlaying = false;
float bioscopeAnimTime = 0.0f;
glm::vec3 bioscopePos(-12.0f, 0.0f, -22.0f);
std::vector<unsigned int> bioscopeFrames;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int createCubeVAO();
unsigned int createGroundVAO();
unsigned int loadTexture(char const* path);

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4); // Request 4x MSAA to smooth jagged edges

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Village Fair — OpenGL 3.3", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialise GLAD\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE); // Enable Hardware Anti-Aliasing (MSAA)

    Shader sceneShader("shaders/scene.vert", "shaders/scene.frag");
    Shader leafShader("shaders/leaf_instanced.vert", "shaders/leaf_instanced.frag");
    Shader skyboxShader("shaders/skybox.vert", "shaders/skybox.frag");

    // Initialize Skybox with day and night cubemaps
    Skybox skybox;
    skybox.load("skybox", "skybox_night");

    unsigned int cubeVAO = createCubeVAO();
    unsigned int groundVAO = createGroundVAO();

    // Textures
    unsigned int bambooTexture = loadTexture("bambo.png");
    unsigned int melaTexture = loadTexture("mela.png");

    unsigned int sofaTexture = loadTexture("sofaa.png");
    unsigned int platTexture = loadTexture("solid_blue.png");
    unsigned int grassTexture = loadTexture("grass.png");

    unsigned int redFabricTexture = loadTexture("red_fabric.png");
    unsigned int greenFabricTexture = loadTexture("green_fabric.png");
    unsigned int blueFabricTexture = loadTexture("blue_fabric.png");
    unsigned int yellowFabricTexture = loadTexture("yellow_fabric.png");
    if (yellowFabricTexture == 0) {
        yellowFabricTexture = loadTexture("Yellow_fabric.png"); // Try capitalized fallback
    }
    if (yellowFabricTexture == 0) {
        // VISUAL DEBUG: If yellow fails, use red as fallback so we can SEE it failed
        yellowFabricTexture = redFabricTexture; 
        std::cout << "DEBUG: yellow_fabric.png FAIL, falling back to RED." << std::endl;
    }
    unsigned int purpleFabricTexture = loadTexture("purple_fabric.png");
    unsigned int ceramicsTexture = loadTexture("bambo.png"); // ceramics.png doesn't exist, using bambo
    unsigned int texA = loadTexture("a.png");
    unsigned int texB = loadTexture("red_fabric.png"); // b.png fails, using red_fabric
    unsigned int texC = loadTexture("mela.png"); // c.png fails, using mela
    unsigned int texD = loadTexture("d.png");
    unsigned int cha1Tex = loadTexture("cha1.png");
    unsigned int cha2Tex = loadTexture("cha2.png");
    unsigned int cha3Tex = loadTexture("cha3.png");

    // Saree stall textures (load only 6 for hanging display)
    std::vector<unsigned int> sareeTextures;
    const char* shariFiles[] = {"shari2.png","shari3.png","shari5.png","shari7.png","shari9.png"};
    for (int i = 0; i < 5; ++i) {
        unsigned int tex = loadTexture(shariFiles[i]);
        if (tex != 0) {
            sareeTextures.push_back(tex);
            std::cout << "Loaded saree texture: " << shariFiles[i] << std::endl;
        } else {
            std::cout << "SKIPPED saree texture: " << shariFiles[i] << std::endl;
        }
    }
    // Stage textures
    unsigned int stageTex = loadTexture("stage.png");
    unsigned int flowerTex = loadTexture("flower.png");
    unsigned int woodTex = loadTexture("wood.png"); // Full wood structure
    
    std::vector<unsigned int> fabricTexs = {
        loadTexture("red_fabric.png"),
        loadTexture("blue_fabric.png"),
        loadTexture("green_fabric.png"),
        loadTexture("purple_fabric.png"),
        loadTexture("yellow_fabric.png")
    };
    unsigned int nokshaTex = loadTexture("noksha.png");
    
    unsigned int shariPriceTex = loadTexture("shari_price.png");
    unsigned int stripesTex = loadTexture("red_white_stripes.png");
    unsigned int popcornTex = loadTexture("popcorn.png");
    unsigned int wood2Tex = loadTexture("wood2.png");
    unsigned int burgerTex = loadTexture("burger.png");
    unsigned int whiteFabricTex = loadTexture("white_fabric.png");
    unsigned int redWhiteTex = loadTexture("red_white.png");
    
    // New stall panel and sign textures
    unsigned int ice1Tex = loadTexture("ice1.png");
    unsigned int ice2Tex = loadTexture("ice2.png");
    unsigned int ice3Tex = loadTexture("ice3.png");
    unsigned int iceSignTex = loadTexture("ice_sign.png");
    
    std::vector<unsigned int> scoopTexs = {
        loadTexture("scope1.png"),
        loadTexture("scope2.png"),
        loadTexture("scope3.png"),
        loadTexture("scope4.png"),
        loadTexture("scope5.png")
    };

    // Cotton Candy textures
    unsigned int cottonSignTex  = loadTexture("cotton_candy_sign.png");
    unsigned int cottonPanelTex = loadTexture("cotton_candy_panel.png");
    unsigned int cotton1Tex     = loadTexture("cotton1.png");
    unsigned int cotton2Tex     = loadTexture("cotton2.png");
    unsigned int cotton3Tex     = loadTexture("cotton3.png");
    unsigned int cstandTex      = loadTexture("cstand.png");

    // Preloading 10 Bioscope Animation Frames
    for (int i = 1; i <= 10; i++) {
        std::string path = "frame" + std::to_string(i) + ".png";
        bioscopeFrames.push_back(loadTexture(path.c_str()));
    }


    glm::vec3 lightDirection(-0.5f, -1.0f, -0.2f);
    glm::vec3 lightAmbient(0.18f, 0.18f, 0.20f);  // Deeper ambient for better contrast
    glm::vec3 lightDiffuse(0.72f, 0.70f, 0.65f);  // Natural sunlight
    glm::vec3 lightSpecular(0.50f, 0.50f, 0.50f); // Moderate highlights

    struct SceneObject {
        glm::vec3 position;
        glm::vec3 scale;
        glm::vec3 color;
        float     shininess;
    };

    std::vector<SceneObject> objects = {
        { glm::vec3(-15.0f, 0.5f,  -15.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.85f, 0.35f, 0.2f), 32.0f },
        { glm::vec3(-15.0f, 0.5f, 15.0f), glm::vec3(0.8f, 1.8f, 0.8f), glm::vec3(0.9f,  0.75f, 0.2f), 64.0f },
        { glm::vec3(22.0f, 1.5f, -22.0f), glm::vec3(0.4f, 3.0f, 0.4f), glm::vec3(0.6f,  0.6f,  0.65f), 128.0f },
    };

    BanyanTree banyanTree(glm::vec3(0.0f, 0.0f, 0.0f), cubeVAO);
    Fence fence(50.0f, 50.0f);
    Gate gate(melaTexture);
    FerrisWheel ferrisWheel;
    SwingRide swingRide(sofaTexture, platTexture);
    Stage culturalStage(cubeVAO);
    Stall stall(cubeVAO);
    stall.setSareeTextures(sareeTextures, shariPriceTex);
    IceCreamCorner iceCreamCorner(cubeVAO);
    CottonCandyCart cottonCandyCart(cubeVAO);
    RingToss ringToss(cubeVAO);
    Bioscope bioscope(cubeVAO);

    bool vKeyPressed = false;
    float ferrisWheelTime = 0.0f;
    float ferrisCurrentSpeed = 1.0f;
    float ferrisTargetSpeed = 1.0f;
    const float ferrisAcceleration = 0.5f; // Time required to stop/start smoothly

    bool bKeyPressed = false;
    float swingRideCurrentSpeed = 1.0f;
    float swingRideTargetSpeed = 1.0f;
    float swingRideTime = 0.0f;
    bool lKeyPressed = false;
    bool kKeyPressed = false;
    bool fairyLightsEnabled = true;
    bool mainLanternsEnabled = true;

    bool k1P = false, k2P = false, k3P = false, k4P = false;
    bool k5P = false, k6P = false, k7P = false, k8P = false, k9P = false;
    
    bool dirLightOn = true;
    bool spotLightOn = true;
    bool ambientOn = true;
    bool diffuseOn = true;
    bool specularOn = true;

    bool isNightMode = false;
    bool k0P = false;

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
            if (!vKeyPressed) {
                // Toggle target speed between 0 (stop) and 1 (full speed)
                ferrisTargetSpeed = (ferrisTargetSpeed > 0.5f) ? 0.0f : 1.0f;
                vKeyPressed = true;
            }
        } else {
            vKeyPressed = false;
        }

        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
            if (!lKeyPressed) {
                fairyLightsEnabled = !fairyLightsEnabled;
                lKeyPressed = true;
            }
        } else {
            lKeyPressed = false;
        }

        if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
            if (!kKeyPressed) {
                mainLanternsEnabled = !mainLanternsEnabled;
                kKeyPressed = true;
            }
        } else {
            kKeyPressed = false;
        }

        // Smoothly interpolate current speed towards target speed for momentum effect
        if (ferrisCurrentSpeed < ferrisTargetSpeed) {
            ferrisCurrentSpeed += ferrisAcceleration * deltaTime;
            if (ferrisCurrentSpeed > ferrisTargetSpeed) ferrisCurrentSpeed = ferrisTargetSpeed;
        } else if (ferrisCurrentSpeed > ferrisTargetSpeed) {
            ferrisCurrentSpeed -= ferrisAcceleration * deltaTime;
            if (ferrisCurrentSpeed < ferrisTargetSpeed) ferrisCurrentSpeed = ferrisTargetSpeed;
        }

        ferrisWheelTime += deltaTime * ferrisCurrentSpeed;

        if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
            if (!bKeyPressed) {
                swingRideTargetSpeed = (swingRideTargetSpeed > 0.5f) ? 0.0f : 1.0f;
                bKeyPressed = true;
            }
        } else {
            bKeyPressed = false;
        }

        if (swingRideCurrentSpeed < swingRideTargetSpeed) {
            swingRideCurrentSpeed += ferrisAcceleration * deltaTime;
            if (swingRideCurrentSpeed > swingRideTargetSpeed) swingRideCurrentSpeed = swingRideTargetSpeed;
        } else if (swingRideCurrentSpeed > swingRideTargetSpeed) {
            swingRideCurrentSpeed -= ferrisAcceleration * deltaTime;
            if (swingRideCurrentSpeed < swingRideTargetSpeed) swingRideCurrentSpeed = swingRideTargetSpeed;
        }

        swingRideTime += deltaTime * swingRideCurrentSpeed;

        ringToss.update(deltaTime);

        // Ring Toss Controls
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) ringToss.aimLeft(deltaTime);
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) ringToss.aimRight(deltaTime);
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) ringToss.aimUp(deltaTime);
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) ringToss.aimDown(deltaTime);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) ringToss.throwRing();
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) ringToss.fullReset();

        // Banyan Tree Lighting Controls
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
            if (!k1P) { banyanTree.toggleSector(0); k1P = true; }
        } else { k1P = false; }
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
            if (!k2P) { banyanTree.toggleSector(1); k2P = true; }
        } else { k2P = false; }
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
            if (!k3P) { banyanTree.toggleSector(2); k3P = true; }
        } else { k3P = false; }
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
            if (!k4P) { banyanTree.toggleSector(3); k4P = true; }
        } else { k4P = false; }
        
        // Global Lighting Controls
        if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) {
            if (!k5P) { dirLightOn = !dirLightOn; k5P = true; }
        } else { k5P = false; }
        if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) {
            if (!k6P) { spotLightOn = !spotLightOn; k6P = true; }
        } else { k6P = false; }
        if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) {
            if (!k7P) { ambientOn = !ambientOn; k7P = true; }
        } else { k7P = false; }
        if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) {
            if (!k8P) { diffuseOn = !diffuseOn; k8P = true; }
        } else { k8P = false; }
        if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS) {
            if (!k9P) { specularOn = !specularOn; k9P = true; }
        } else { k9P = false; }

        if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) {
            if (!k0P) { isNightMode = !isNightMode; k0P = true; }
        } else { k0P = false; }

        processInput(window);

        if (isNightMode) {
            glClearColor(0.02f, 0.05f, 0.15f, 1.0f); // Deep Night Blue
        } else {
            glClearColor(0.53f, 0.75f, 0.92f, 1.0f); // Sky Blue (Daylight)
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 500.0f);

        // ── Skybox (drawn FIRST so transparent ground can alpha-blend over it) ──
        skybox.draw(skyboxShader, view, projection, isNightMode);

        sceneShader.use();
        sceneShader.setMat4("view", view);
        sceneShader.setMat4("projection", projection);
        sceneShader.setVec3("viewPos", camera.Position);
        sceneShader.setVec3("lightDirection", lightDirection);
        
        if (isNightMode) {
            sceneShader.setVec3("lightAmbient",  glm::vec3(0.05f, 0.05f, 0.10f));
            sceneShader.setVec3("lightDiffuse",  glm::vec3(0.02f, 0.02f, 0.05f));
            sceneShader.setVec3("lightSpecular", glm::vec3(0.10f, 0.10f, 0.15f));
            sceneShader.setBool("dirLightOn",    false); // Sun is down
        } else {
            sceneShader.setVec3("lightAmbient",  lightAmbient);
            sceneShader.setVec3("lightDiffuse",  lightDiffuse);
            sceneShader.setVec3("lightSpecular", lightSpecular);
            sceneShader.setBool("dirLightOn",    dirLightOn);
        }

        // Pass Toggle Uniforms
        sceneShader.setBool("ambientOn", ambientOn);
        sceneShader.setBool("diffuseOn", diffuseOn);
        sceneShader.setBool("specularOn", specularOn);
        sceneShader.setBool("dirLightOn", isNightMode ? false : dirLightOn);
        sceneShader.setBool("spotLightOn", spotLightOn);
        sceneShader.setFloat("specularStrength", 0.15f); // Reduced global "whitish vibe"
        sceneShader.setFloat("pointLightIntensity", isNightMode ? 1.0f : 0.22f);

        // -- Lighting Setup Phase --
        for (int i = 0; i < 5; i++) {
            float zPos = 5.0f - (i * 6.2f);
            stall.setupPointLights(sceneShader, glm::vec3(21.8f, 0.0f, zPos), -90.0f, (i * 3), mainLanternsEnabled);
        }
        banyanTree.setupPointLights(sceneShader);
        fence.setupPointLights(sceneShader, isNightMode); 

        // 1. Draw Stalls (Right boundary, facing center)
        std::vector<unsigned int> fabricTextures = {
            redFabricTexture,
            greenFabricTexture,
            blueFabricTexture,
            yellowFabricTexture,
            purpleFabricTexture
        };
        for (int i = 0; i < 5; ++i) {
            float zPos = 5.0f - (i * 6.2f);
            
            unsigned int currentTA = texA, currentTB = texB, currentTC = texC, currentTD = texD;
            if (i == 3) {
                currentTA = cha1Tex;
                currentTB = cha2Tex;
                currentTC = cha3Tex;
                currentTD = 0;
            }
            
            // Re-indexed to start from 0, avoiding conflict with Banyan Tree (which starts at 16)
            stall.draw(sceneShader, glm::vec3(21.8f, 0.0f, zPos), -90.0f, fabricTextures[i % 5], bambooTexture, (i * 3), currentFrame, fairyLightsEnabled, mainLanternsEnabled, i, ceramicsTexture, currentTA, currentTB, currentTC, currentTD);
        }

        // Draw Ground
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::scale(model, glm::vec3(500.0f, 1.0f, 500.0f));
            sceneShader.setMat4("model", model);
            sceneShader.setVec3("objectColor", glm::vec3(0.90f, 0.90f, 0.90f)); // Brighten grass to blend with bright skybox floor
            sceneShader.setFloat("specularStrength", 0.015f); // Matte ground
            sceneShader.setVec3("emission",    glm::vec3(0.0f, 0.0f, 0.0f));
            sceneShader.setBool("isGround", true);
            
            // Respect global day/night lighting state
            sceneShader.setBool("useTexture", true);
            sceneShader.setInt("texture_diffuse", 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, grassTexture);
            
            glBindVertexArray(groundVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            
            sceneShader.setBool("useTexture", false);
            sceneShader.setBool("isGround", false);
        }

        // Draw Scene Objects
        glBindVertexArray(cubeVAO);
        for (const auto& obj : objects) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, obj.position);
            model = glm::scale(model, obj.scale);
            sceneShader.setMat4("model", model);
            sceneShader.setVec3("objectColor", obj.color);
            sceneShader.setFloat("shininess", obj.shininess);
            sceneShader.setFloat("specularStrength", 0.50f); // Natural outdoor specular
            sceneShader.setBool("useTexture", false);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // 2. Draw Environment
        glm::vec3 gatePos(0, 0, 24.8f);
        fence.draw(sceneShader, bambooTexture, isNightMode);
        gate.draw(sceneShader, gatePos);
        ferrisWheel.draw(sceneShader, glm::vec3(-19.0f, 0.0f, 22.0f), ferrisWheelTime);
        swingRide.draw(sceneShader, glm::vec3(16.0f, 0.0f, 16.0f), swingRideTime);
        iceCreamCorner.draw(sceneShader, glm::vec3(-22.5f, 0.0f, 11.0f), 90.0f, stripesTex, woodTex, popcornTex, wood2Tex, burgerTex, redWhiteTex, ice1Tex, ice2Tex, ice3Tex, iceSignTex, scoopTexs);
        cottonCandyCart.draw(sceneShader, glm::mat4(1.0f), glm::vec3(-22.5f, 0.0f, -7.0f), 90.0f, cottonSignTex, cottonPanelTex, currentFrame, cotton1Tex, cotton2Tex, cotton3Tex, cstandTex);
        ringToss.draw(sceneShader, glm::vec3(0.0f, 0.0f, -22.0f), greenFabricTexture, woodTex);
        // Render Bioscope with Animation
        int bFrameIdx = 0;
        if (bioscopeIsPlaying) {
            bioscopeAnimTime += deltaTime;
            bFrameIdx = (int)(bioscopeAnimTime * 4.0f) % 10; // 4 FPS for a more vintage feel
        }
        bioscope.draw(sceneShader, bioscopePos, woodTex, nokshaTex, whiteFabricTex, bioscopeIsPlaying, bioscopeFrames[bFrameIdx]);


        // Removed duplicate stall drawing loop and integrated indexing fix.

        // 4. Draw Banyan Tree
        leafShader.use();
        leafShader.setMat4("view", view);
        leafShader.setMat4("projection", projection);
        leafShader.setVec3("viewPos", camera.Position);
        leafShader.setVec3("lightDirection", lightDirection);
        leafShader.setVec3("lightAmbient", lightAmbient);
        leafShader.setVec3("lightDiffuse", lightDiffuse);
        leafShader.setBool("ambientOn", ambientOn);
        leafShader.setBool("diffuseOn", diffuseOn);
        leafShader.setBool("dirLightOn", dirLightOn);
        banyanTree.draw(sceneShader, leafShader);

        // 5. Draw Cultural Stage
        // Positioned gracefully in front of the Banyan Tree
        culturalStage.draw(sceneShader, glm::vec3(0.0f, 0.0f, 6.0f), stageTex, flowerTex, woodTex, fabricTexs, nokshaTex, currentFrame);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &groundVAO);
    glfwTerminate();
    return 0;
}

unsigned int loadTexture(char const* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    int width, height, nrComponents;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format = GL_RGB;
        if (nrComponents == 1) format = GL_RED;
        else if (nrComponents == 3) format = GL_RGB;
        else if (nrComponents == 4) format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, (GLint)format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        // Enable Anisotropic filtering for sharper textures (fixes distant blur)
        float maxAniso = 0.0f;
        glGetFloatv(0x84FF, &maxAniso); // GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
        if (maxAniso > 0.0f) {
            glTexParameterf(GL_TEXTURE_2D, 0x84FE, maxAniso); // GL_TEXTURE_MAX_ANISOTROPY_EXT
        }
        
        stbi_image_free(data);
    } else {
        std::cout << "Texture failed to load: " << path << std::endl;
        glDeleteTextures(1, &textureID);
        textureID = 0;
        stbi_image_free(data);
    }
    return textureID;
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
    
    float distToBioscope = glm::distance(camera.Position, bioscopePos);

    // Interaction Controls
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS && distToBioscope < 8.0f) {
        if (!bioscopeIsPlaying) {
            bioscopeIsPlaying = true;
            std::cout << "BIOSCOPE: Animation Started. Distance: " << distToBioscope << std::endl;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS || distToBioscope > 10.0f) {
        if (bioscopeIsPlaying) {
            bioscopeIsPlaying = false;
            std::cout << "BIOSCOPE: Animation Stopped." << std::endl;
        }
    }

    if (bioscopeIsPlaying) {
        // Hijack camera: position it for a full-screen experience
        camera.Position = bioscopePos + glm::vec3(0.0f, 1.45f, 2.8f);
        camera.Yaw = -90.0f; 
        camera.Pitch = 0.0f;
        camera.updateCameraVectors();
        return; // Lock movement while viewing
    }

    // Translation Speed
    float speed = 5.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.Position += camera.Front * speed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.Position -= camera.Front * speed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.Position -= camera.Right * speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.Position += camera.Right * speed;
    // Up / Down
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) camera.Position += camera.Up * speed;
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) camera.Position -= camera.Up * speed;

    // Rotation Speed
    float rotSpeed = 45.0f * deltaTime;
    bool rotated = false;
    
    // Pitch (X = Up, C = Down)
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) { camera.Pitch += rotSpeed; rotated = true; }
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) { camera.Pitch -= rotSpeed; rotated = true; }
    
    // Yaw (Y = Right, U = Left)
    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) { camera.Yaw += rotSpeed; rotated = true; }
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) { camera.Yaw -= rotSpeed; rotated = true; }
    
    // Roll (Z = Right, Q = Left)
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) { camera.Roll += rotSpeed; rotated = true; }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) { camera.Roll -= rotSpeed; rotated = true; }

    // Enforce Pitch Constraints (Standard FPS logic)
    if (camera.Pitch > 89.0f) camera.Pitch = 89.0f;
    if (camera.Pitch < -89.0f) camera.Pitch = -89.0f;

    if (rotated) {
        camera.updateCameraVectors();
    }
}
void framebuffer_size_callback(GLFWwindow*, int width, int height) { glViewport(0, 0, width, height); }
void mouse_callback(GLFWwindow*, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn); float ypos = static_cast<float>(yposIn);
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    float xoffset = (xpos - lastX); float yoffset = (lastY - ypos);
    lastX = xpos; lastY = ypos; camera.ProcessMouseMovement(xoffset, yoffset);
}
void scroll_callback(GLFWwindow*, double, double yoffset) { camera.ProcessMouseScroll(static_cast<float>(yoffset)); }
unsigned int createCubeVAO() {
    float vertices[] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
    };
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO); glGenBuffers(1, &VBO);
    glBindVertexArray(VAO); glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); glEnableVertexAttribArray(2);
    glBindVertexArray(0); return VAO;
}
unsigned int createGroundVAO() {
    float k = 50.0f; // Adjusted for better detail visibility on the huge ground plane
    float vertices[] = {
        // positions          // normals         // texture coords
        -0.5f, 0.0f, -0.5f,  0.0f, 1.0f, 0.0f,   0.0f, k,
         0.5f, 0.0f,  0.5f,  0.0f, 1.0f, 0.0f,   k, 0.0f,
         0.5f, 0.0f, -0.5f,  0.0f, 1.0f, 0.0f,   k, k,

         0.5f, 0.0f,  0.5f,  0.0f, 1.0f, 0.0f,   k, 0.0f,
        -0.5f, 0.0f, -0.5f,  0.0f, 1.0f, 0.0f,   0.0f, k,
        -0.5f, 0.0f,  0.5f,  0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
    };
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO); glGenBuffers(1, &VBO);
    glBindVertexArray(VAO); glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); glEnableVertexAttribArray(2);
    glBindVertexArray(0); return VAO;
}