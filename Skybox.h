#ifndef SKYBOX_H
#define SKYBOX_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>
#include <iostream>
#include "stb_image.h"
#include "shader.h"

// ============================================================
//  Skybox Class
//  Loads a cubemap texture from 6 face images and renders
//  a skybox cube that surrounds the entire scene.
//
//  Supports day/night mode via two separate cubemaps.
//
//  Usage:
//    Skybox skybox;
//    skybox.load("skybox", "skybox_night");
//    skybox.draw(skyboxShader, view, projection, isNight);
// ============================================================
class Skybox {
public:
    unsigned int dayTexture;
    unsigned int nightTexture;

    Skybox() : dayTexture(0), nightTexture(0), skyboxVAO(0), skyboxVBO(0) {}

    ~Skybox() {
        if (skyboxVAO) glDeleteVertexArrays(1, &skyboxVAO);
        if (skyboxVBO) glDeleteBuffers(1, &skyboxVBO);
        if (dayTexture) glDeleteTextures(1, &dayTexture);
        if (nightTexture) glDeleteTextures(1, &nightTexture);
    }

    // --------------------------------------------------------
    //  Load both day and night cubemaps and create the cube VAO
    // --------------------------------------------------------
    void load(const std::string& dayDir, const std::string& nightDir) {
        setupCubeVAO();
        dayTexture = loadCubemap(dayDir);
        nightTexture = loadCubemap(nightDir);

        if (dayTexture == 0)
            std::cerr << "[Skybox] WARNING: Day cubemap failed to load from: " << dayDir << std::endl;
        if (nightTexture == 0)
            std::cerr << "[Skybox] WARNING: Night cubemap failed to load from: " << nightDir << std::endl;
    }

    // --------------------------------------------------------
    //  Draw the skybox
    //  Should be called AFTER all scene objects.
    //  Handles depth state internally.
    // --------------------------------------------------------
    void draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection, bool isNight) {
        unsigned int activeTexture = isNight ? nightTexture : dayTexture;
        if (activeTexture == 0 || skyboxVAO == 0) return;

        // Save current depth state
        GLint prevDepthFunc;
        glGetIntegerv(GL_DEPTH_FUNC, &prevDepthFunc);
        GLboolean prevDepthMask;
        glGetBooleanv(GL_DEPTH_WRITEMASK, &prevDepthMask);

        // Set skybox depth state
        glDepthFunc(GL_LEQUAL);     // Pass where depth == 1.0 (empty background)
        glDepthMask(GL_FALSE);      // Don't write to depth buffer

        shader.use();

        // Strip translation from view matrix — only rotation affects skybox
        glm::mat4 skyboxView = glm::mat4(glm::mat3(view));

        shader.setMat4("view", skyboxView);
        shader.setMat4("projection", projection);

        // Bind cubemap
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, activeTexture);
        shader.setInt("skybox", 0);

        // Draw cube
        glBindVertexArray(skyboxVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        // Restore depth state
        glDepthFunc(prevDepthFunc);
        glDepthMask(prevDepthMask);
    }

private:
    unsigned int skyboxVAO, skyboxVBO;

    // --------------------------------------------------------
    //  Create a simple cube VAO (positions only, no normals/UVs)
    // --------------------------------------------------------
    void setupCubeVAO() {
        float skyboxVertices[] = {
            // positions          
            // Back face (-Z)
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            // Front face (+Z)
            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            // Left face (-X)
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,

            // Right face (+X)
             1.0f,  1.0f, -1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,

            // Top face (+Y)
            -1.0f,  1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            // Bottom face (-Y)
            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
        };

        glGenVertexArrays(1, &skyboxVAO);
        glGenBuffers(1, &skyboxVBO);
        glBindVertexArray(skyboxVAO);
        glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
    }

    // --------------------------------------------------------
    //  Load 6 face images into an OpenGL cubemap texture
    //  Face order follows OpenGL convention:
    //    +X, -X, +Y, -Y, +Z, -Z
    // --------------------------------------------------------
    unsigned int loadCubemap(const std::string& directory) {
        std::vector<std::string> faces = {
            directory + "/right.png",   // GL_TEXTURE_CUBE_MAP_POSITIVE_X
            directory + "/left.png",    // GL_TEXTURE_CUBE_MAP_NEGATIVE_X
            directory + "/top.png",     // GL_TEXTURE_CUBE_MAP_POSITIVE_Y
            directory + "/bottom.png",  // GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
            directory + "/front.png",   // GL_TEXTURE_CUBE_MAP_POSITIVE_Z
            directory + "/back.png"     // GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
        };

        unsigned int textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

        // Cubemap faces should NOT be flipped
        stbi_set_flip_vertically_on_load(false);

        int width, height, nrChannels;
        bool allLoaded = true;

        for (unsigned int i = 0; i < faces.size(); i++) {
            unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
            if (data) {
                GLenum format = GL_RGB;
                if (nrChannels == 1) format = GL_RED;
                else if (nrChannels == 3) format = GL_RGB;
                else if (nrChannels == 4) format = GL_RGBA;

                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                             0, (GLint)format, width, height, 0,
                             format, GL_UNSIGNED_BYTE, data);
                std::cout << "[Skybox] Loaded: " << faces[i] 
                          << " (" << width << "x" << height << ", " << nrChannels << "ch)" << std::endl;
                stbi_image_free(data);
            }
            else {
                std::cerr << "[Skybox] FAILED to load: " << faces[i] << std::endl;
                stbi_image_free(data);
                allLoaded = false;
            }
        }

        // Restore normal flip for other textures
        stbi_set_flip_vertically_on_load(true);

        if (!allLoaded) {
            std::cerr << "[Skybox] WARNING: Some cubemap faces failed to load." << std::endl;
        }

        // Cubemap texture parameters — clamp to edge to avoid seams
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        return textureID;
    }
};

#endif // SKYBOX_H
