#ifndef BANYANTREE_H
#define BANYANTREE_H

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstdlib>
#include <ctime>
#include "shader.h"
#include "cylinder.h"
#include "stb_image.h"
#include "sphereWithTexture.h"
#include <iostream>

struct TreePart {
    glm::mat4 transform;
    glm::vec3 color;
};

class BanyanTree {
public:
    void toggleSector(int index) {
        if (index >= 0 && index < 4) sectorStates[index] = !sectorStates[index];
    }
    BanyanTree(glm::vec3 position, unsigned int cubeVAO) 
        : basePosition(position), cylinder(16), trunkCubeVAO(cubeVAO), 
          bulbSphere(0.35f, 16, 8, glm::vec3(1.0f), glm::vec3(1.0f), glm::vec3(0.5f), 32.0f, 0, 0, 0, 0, 1, 1) {
        srand(static_cast<unsigned int>(time(NULL)));
        
        // 1. Load Textures
        leafTexture = loadTexture("leaf.png");
        trunkTexture = loadTexture("tree.png");
        bulbTexture = loadTexture("lamp.png");

        // 2. Setup Leaf Geometry (Textured Quad)
        setupLeafVAO();

        // 3. Generate Tree Data
        generateOrganicTrunk();

        // 4. Setup Instancing Buffers for Leaves
        setupInstancing();

        // 5. Generate Decorative Lights
        generateFestiveLights();
    }

    void setupPointLights(const Shader& shader) {
        glm::vec3 vibrantOrange = glm::vec3(1.0f, 0.38f, 0.0f);
        int lightIdx = 16;
        int sectorCount[4] = { 0, 0, 0, 0 };

        for (int i = 0; i < bulbMatrices.size() && lightIdx < 32; ++i) {
            int sector = bulbSectors[i];
            bool isOn = sectorStates[sector];

            // Pick 4 bulbs from each sector to act as Hero lights (4s * 4 = 16)
            if (sectorCount[sector] < 4) {
                std::string base = "pointLights[" + std::to_string(lightIdx) + "].";
                shader.setBool(base + "isEnabled", isOn);
                if (isOn) {
                    glm::vec3 p = glm::vec3(bulbMatrices[i][3]);
                    shader.setVec3(base + "position", p);
                    shader.setVec3(base + "color", vibrantOrange * 1.1f);
                    shader.setFloat(base + "constant", 1.0f);
                    shader.setFloat(base + "linear", 0.09f);
                    shader.setFloat(base + "quadratic", 0.032f);
                }
                sectorCount[sector]++;
                lightIdx++;
            }
        }
    }

    void draw(const Shader& branchShader, const Shader& leafShader) {
        // Draw Branches/Trunk/Roots using the standard branchShader
        branchShader.use();
        branchShader.setBool("useTexture", true);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, trunkTexture);
        
        for (const auto& branch : branches) {
            branchShader.setMat4("model", branch.transform);
            branchShader.setVec3("objectColor", branch.color);
            branchShader.setFloat("shininess", 1.0f); // Spreads any remaining light very thinly
            branchShader.setFloat("specularStrength", 0.05f); // 95% reduction in plastic-like glints
            cylinder.draw();
        }

        for (const auto& root : roots) {
            branchShader.setMat4("model", root.transform);
            branchShader.setVec3("objectColor", root.color);
            branchShader.setFloat("shininess", 1.0f);
            branchShader.setFloat("specularStrength", 0.02f); // Roots are even more matte
            cylinder.draw();
        }

        // Draw Leaves using Instanced Rendering & leafShader
        leafShader.use();
        leafShader.setFloat("specularStrength", 0.15f); // Subtle waxy leaf sheen
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, leafTexture);
        leafShader.setInt("leafTexture", 0);

        glBindVertexArray(leafInstancedVAO);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, leafMatrices.size());
        glBindVertexArray(0);

        // Draw Decorative Wires (White cables)
        branchShader.use();
        branchShader.setBool("useTexture", false);
        branchShader.setVec3("objectColor", glm::vec3(1.0f, 1.0f, 1.0f));
        branchShader.setFloat("specularStrength", 0.0f);
        branchShader.setVec3("emission", glm::vec3(0.0f));

        for (const auto& wire : wireMatrices) {
            branchShader.setMat4("model", wire);
            cylinder.draw();
        }

        // 2. Draw Sockets (Small black junctions)
        branchShader.setVec3("objectColor", glm::vec3(0.02f, 0.02f, 0.02f));
        for (const auto& socket : socketMatrices) {
            branchShader.setMat4("model", socket);
            cylinder.draw();
        }

        // 3. Draw Bulb Highlights
        glm::vec3 vibrantOrange = glm::vec3(1.0f, 0.38f, 0.0f); 
        glm::vec3 glassGray = glm::vec3(0.85f, 0.85f, 0.9f);
        branchShader.setFloat("specularStrength", 0.5f); 
        branchShader.setFloat("shininess", 128.0f);

        for (int i = 0; i < bulbMatrices.size(); ++i) {
            int sector = bulbSectors[i];
            bool isOn = sectorStates[sector];
            
            branchShader.setBool("useTexture", true);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, bulbTexture);

            branchShader.setMat4("model", bulbMatrices[i]);
            branchShader.setVec3("objectColor", isOn ? vibrantOrange : glassGray);
            branchShader.setVec3("emission", isOn ? (vibrantOrange * 3.5f) : glm::vec3(0.0f));
            
            glBindVertexArray(bulbSphere.sphereTexVAO);
            glDrawElements(GL_TRIANGLES, bulbSphere.getIndexCount(), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }
        branchShader.setVec3("emission", glm::vec3(0.0f)); 
    }

private:
    glm::vec3 basePosition;
    Cylinder cylinder;
    unsigned int trunkCubeVAO;
    unsigned int leafTexture;
    unsigned int trunkTexture;
    unsigned int leafInstancedVAO;
    unsigned int instanceVBO;

    std::vector<TreePart> branches;
    std::vector<TreePart> roots;
    std::vector<glm::mat4> leafMatrices;
    
    // Festive Light Data
    SphereWithTexture bulbSphere;
    unsigned int bulbTexture;
    std::vector<glm::mat4> bulbMatrices;
    std::vector<glm::mat4> wireMatrices;
    std::vector<glm::mat4> socketMatrices;
    std::vector<int> bulbSectors; 
    bool sectorStates[4] = { false, false, false, false };

    glm::vec3 barkColor = glm::vec3(0.32f, 0.26f, 0.18f); 
    glm::vec3 rootColor = glm::vec3(0.38f, 0.30f, 0.22f);

    unsigned int loadTexture(char const* path) {
        unsigned int textureID;
        glGenTextures(1, &textureID);

        int width, height, nrComponents;
        unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
        if (data) {
            GLenum format;
            if (nrComponents == 1) format = GL_RED;
            else if (nrComponents == 3) format = GL_RGB;
            else if (nrComponents == 4) format = GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
        } else {
            std::cout << "Texture failed to load! Using solid color fallback." << std::endl;
            // Create a small solid deep green texture as fallback
            unsigned char fallbackData[] = { 20, 80, 20, 255 }; // RGBA Deep Green
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, fallbackData);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }
        return textureID;
    }

    void setupLeafVAO() {
        float leafVertices[] = {
            // positions        // texture coords
            -0.5f,  0.5f, 0.0f,  0.0f, 1.0f,
            -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
             0.5f, -0.5f, 0.0f,  1.0f, 0.0f,

            -0.5f,  0.5f, 0.0f,  0.0f, 1.0f,
             0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
             0.5f,  0.5f, 0.0f,  1.0f, 1.0f
        };

        unsigned int leafVBO;
        glGenVertexArrays(1, &leafInstancedVAO);
        glGenBuffers(1, &leafVBO);

        glBindVertexArray(leafInstancedVAO);
        glBindBuffer(GL_ARRAY_BUFFER, leafVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(leafVertices), leafVertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void setupInstancing() {
        glBindVertexArray(leafInstancedVAO);
        glGenBuffers(1, &instanceVBO);
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, leafMatrices.size() * sizeof(glm::mat4), &leafMatrices[0], GL_STATIC_DRAW);

        // Matrix is 4 vec4s
        for (unsigned int i = 0; i < 4; i++) {
            glEnableVertexAttribArray(2 + i);
            glVertexAttribPointer(2 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(i * sizeof(glm::vec4)));
            glVertexAttribDivisor(2 + i, 1);
        }
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void generateOrganicTrunk() {
        int numStrands = 8;
        float baseRadius = 1.0f;
        for (int i = 0; i < numStrands; ++i) {
            float angle = glm::two_pi<float>() * i / numStrands;
            float r = baseRadius * (0.8f + (rand() % 40) / 100.0f);
            glm::vec3 offset(cos(angle) * r, 0.0f, sin(angle) * r);
            glm::vec3 targetDir = glm::normalize(glm::vec3(-offset.x * 0.15f, 1.0f, -offset.z * 0.15f));
        // Increased Initial Height
        generateBranch(basePosition + offset, targetDir, 6.5f, 0.75f, 6);
    }
    // Main central trunk even taller
    generateBranch(basePosition, glm::vec3(0.001f, 1.0f, 0.0f), 7.5f, 0.95f, 6);
}

void generateBranch(glm::vec3 start, glm::vec3 dir, float length, float width, int depth) {
    if (depth == 0) {
        generateLeaves(start);
        return;
    }

    dir = glm::normalize(dir);
    glm::vec3 end = start + dir * length;

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, start);

    glm::vec3 up(0.0f, 1.0f, 0.0f);
    float dot = glm::dot(up, dir);
    if (glm::abs(dot) < 0.999f) {
        glm::vec3 axis = glm::normalize(glm::cross(up, dir));
        float angle = glm::acos(dot);
        model = glm::rotate(model, angle, axis);
    }

    model = glm::scale(model, glm::vec3(width, length, width));
    branches.push_back({model, barkColor * (0.85f + (rand() % 30) / 100.0f)});

    int numChildren = (depth > 4) ? 2 : (depth > 2 ? 3 : 2);
    for (int i = 0; i < numChildren; ++i) {
        float spread = 1.6f; // Restored to previous conservative spread
        glm::vec3 randomOffset(
            (rand() % 200 - 100) / 100.0f * spread,
            (rand() % 100 - 30) / 100.0f * (0.3f), // slight vertical bias restored
            (rand() % 200 - 100) / 100.0f * spread
        );
        
        glm::vec3 nextDir = glm::normalize(dir + randomOffset);
        generateBranch(end, nextDir, length * 0.85f, width * 0.70f, depth - 1);
    }

    // Aerial roots removed as requested
    /*
    if (depth < 6 && depth > 2 && start.y > 3.0f && (rand() % 10 < 3)) {
         generateWavyRoot(start + dir * (length * 0.5f));
    }
    */
}

    void generateWavyRoot(glm::vec3 start) {
        int segments = 4;
        float segHeight = start.y / segments;
        glm::vec3 currPos = start;
        float rootWidth = 0.05f + (rand() % 3) / 100.0f;

        for (int i = 0; i < segments; ++i) {
            glm::vec3 nextPos = currPos - glm::vec3((rand() % 10 - 5) / 30.0f, segHeight, (rand() % 10 - 5) / 30.0f);
            glm::vec3 dir = nextPos - currPos;
            float len = glm::length(dir);
            dir = glm::normalize(dir);

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, currPos);

            glm::vec3 up(0.0f, 1.0f, 0.0f);
            float d = glm::dot(up, dir);
            if (glm::abs(d) < 0.999f) {
                glm::vec3 axis = glm::normalize(glm::cross(up, dir));
                model = glm::rotate(model, glm::acos(d), axis);
            }
            model = glm::scale(model, glm::vec3(rootWidth, len, rootWidth));
            roots.push_back({model, rootColor});
            currPos = nextPos;
            if (currPos.y < 0.0f) break;
        }
    }

    void generateLeaves(glm::vec3 pos) {
        int numLeafClusters = 22 + rand() % 12;
        for (int p = 0; p < numLeafClusters; ++p) {
            glm::mat4 model = glm::mat4(1.0f);
            float spread = 3.5f;

            // Radial outward bias: push leaves away from the tree center
            glm::vec3 radialDir(pos.x - basePosition.x, 0.0f, pos.z - basePosition.z);
            float distFromCenter = glm::length(radialDir);
            if (distFromCenter > 0.01f) radialDir = glm::normalize(radialDir);
            else radialDir = glm::vec3((rand() % 200 - 100) / 100.0f, 0.0f, (rand() % 200 - 100) / 100.0f);
            float outwardPush = 1.2f + (rand() % 100) / 100.0f * 0.8f;

            glm::vec3 clusterPos = pos + glm::vec3(
                (rand() % 200 - 100) / 100.0f * spread + radialDir.x * outwardPush,
                (rand() % 100 - 65) / 100.0f * (spread * 0.5f), // slight downward droop
                (rand() % 200 - 100) / 100.0f * spread + radialDir.z * outwardPush
            );

            model = glm::translate(model, clusterPos);
            model = glm::rotate(model, (float)(rand() % 360), glm::vec3(0, 1, 0));

            float s = 1.0f + (rand() % 60) / 100.0f;
            model = glm::scale(model, glm::vec3(s));

            leafMatrices.push_back(model);
        }
    }

    void generateFestiveLights() {
        bulbMatrices.clear();
        wireMatrices.clear();
        socketMatrices.clear();
        bulbSectors.clear();

        std::vector<glm::vec3> pools[4]; // 0:Front, 1:Left, 2:Right, 3:Back
        
        auto classifyAndPush = [&](glm::vec3 p) {
            if (p.y < 3.0f || p.y > 13.0f) return;
            float angle = glm::degrees(atan2(p.z, p.x));
            int sector = 3; 
            if (angle > 45.0f && angle < 135.0f) sector = 0; 
            else if (angle < -45.0f && angle > -135.0f) sector = 3; 
            else if (angle >= -45.0f && angle <= 45.0f) sector = 2; 
            else sector = 1; 
            pools[sector].push_back(p);
        };

        for (const auto& leaf : leafMatrices) classifyAndPush(glm::vec3(leaf[3]));
        for (const auto& branch : branches) classifyAndPush(glm::vec3(branch.transform[3]));

        // Strategy: Select exactly 15 bulbs from each quadrant (Total 60)
        // Using organic random selection for "uneven" look
        for (int sector = 0; sector < 4; ++sector) {
            if (pools[sector].empty()) continue;
            
            int countInSector = 0;
            int maxInSector = 15;
            
            // Randomly shuffle or just pick with random offsets
            for (int i = 0; i < maxInSector; ++i) {
                int randIdx = rand() % pools[sector].size();
                glm::vec3 pos = pools[sector][randIdx];
                
                bulbSectors.push_back(sector);

                float wireLen = 0.85f + (rand() % 80) / 100.0f;
                // Wire
                glm::mat4 wireM = glm::translate(glm::mat4(1.0f), pos - glm::vec3(0.0f, wireLen, 0.0f));
                wireM = glm::scale(wireM, glm::vec3(0.045f, wireLen, 0.045f)); 
                wireMatrices.push_back(wireM);
                // Socket
                glm::vec3 socketPos = pos - glm::vec3(0.0f, wireLen, 0.0f);
                glm::mat4 socketM = glm::translate(glm::mat4(1.0f), socketPos - glm::vec3(0.0f, 0.22f, 0.0f));
                socketM = glm::scale(socketM, glm::vec3(0.15f, 0.22f, 0.15f)); 
                socketMatrices.push_back(socketM);
                // Bulb
                glm::vec3 bulbPos = socketPos - glm::vec3(0.0f, 0.22f, 0.0f);
                glm::mat4 bulbM = glm::translate(glm::mat4(1.0f), bulbPos);
                bulbMatrices.push_back(bulbM);
            }
        }
        std::cout << "BIOSCOPE: Organic 4-Way Balanced lighting complete (60 bulbs total)." << std::endl;
    }
};

#endif
