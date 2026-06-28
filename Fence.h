#ifndef FENCE_H
#define FENCE_H

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstdlib>
#include "shader.h"
#include "cylinder.h"
#include "sphere.h"

class Fence {
public:
    Fence(float width, float depth, float poleSpacing = 0.6f) 
        : width(width), depth(depth), poleSpacing(poleSpacing), sphere(15) {
        generateFence();
        setupInstancing();
    }

    void draw(Shader& shader, unsigned int textureID, bool isNightMode) {
        shader.use();
        shader.setBool("isInstanced", true);
        shader.setBool("useTexture", true);
        shader.setVec3("objectColor", glm::vec3(1.0f)); // Modulate with white
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        shader.setInt("texture_diffuse", 0);

        glBindVertexArray(fenceVAO);
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, cylinderVertexCount, instanceMatrices.size());
        
        // Draw horizontal supports 
        shader.setBool("isInstanced", false);
        for (const auto& support : supports) {
            shader.setMat4("model", support);
            cylinder.draw();
        }
        
        glBindVertexArray(0);
        shader.setBool("isInstanced", false);
        shader.setBool("useTexture", false);

        // Draw Decorative Bulbs
        // Champagne warm white color
        glm::vec3 bulbColor(1.0f, 0.95f, 0.75f);
        shader.setVec3("objectColor", bulbColor);
        
        // Glow only at night
        if (isNightMode) {
            shader.setVec3("emission", bulbColor * 0.8f);
        } else {
            shader.setVec3("emission", glm::vec3(0.0f));
        }

        for (const auto& pos : bulbPositions) {
            shader.setVec3("emission", glm::vec3(0.0f));

            // Cylinder base=y0, top=y1.
            // pos = world-space fence top. Pole base starts here.
            const float poleH      = 0.36f;
            const float collarH    = 0.022f;
            const float globeScale = 0.016f;
            const float globeR     = 15.0f * globeScale;

            // 1. Black lamp pole
            shader.setVec3("objectColor", glm::vec3(0.07f, 0.07f, 0.08f));
            glm::mat4 pole = glm::translate(glm::mat4(1.0f), pos);
            pole = glm::scale(pole, glm::vec3(0.025f, poleH, 0.025f));
            shader.setMat4("model", pole);
            cylinder.draw();

            // 2. Brass collar — base at pole top
            shader.setVec3("objectColor", glm::vec3(0.65f, 0.55f, 0.30f));
            glm::mat4 collar = glm::translate(glm::mat4(1.0f), pos + glm::vec3(0.0f, poleH, 0.0f));
            collar = glm::scale(collar, glm::vec3(0.034f, collarH, 0.034f));
            shader.setMat4("model", collar);
            cylinder.draw();

            // 3. Globe — center sits on collar top
            float globeCenterY = poleH + collarH + globeR;
            glm::vec3 globeCol(1.00f, 0.94f, 0.76f);
            shader.setVec3("objectColor", globeCol);
            if (isNightMode) {
                shader.setVec3("emission", globeCol * 1.3f);
            }
            glm::mat4 globe = glm::translate(glm::mat4(1.0f), pos + glm::vec3(0.0f, globeCenterY, 0.0f));
            globe = glm::scale(globe, glm::vec3(globeScale, globeScale, globeScale));
            shader.setMat4("model", globe);
            sphere.drawSphere(shader);
        }
        shader.setVec3("emission", glm::vec3(0.0f));
    }

    void setupPointLights(Shader& shader, bool isEnabled) {
        // We have ~30-40 bulbs now, but only 12 light slots (32-43).
        // Spread 12 lights evenly across the fence perimeter.
        int numBulbs = (int)bulbPositions.size();
        if (numBulbs == 0) return;
        
        int step = (numBulbs > 12) ? (numBulbs / 12) : 1;

        for (int i = 0; i < 12; ++i) {
            int bulbIdx = (i * step) % numBulbs;
            int lightIdx = 32 + i;
            std::string prefix = "pointLights[" + std::to_string(lightIdx) + "].";
            
            // globeCenterY = 0.36 + 0.022 + 15*0.016 = 0.622
            glm::vec3 lightPos = bulbPositions[bulbIdx] + glm::vec3(0.0f, 0.622f, 0.0f);

            shader.setVec3(prefix + "position", lightPos);
            shader.setVec3(prefix + "color", glm::vec3(0.72f, 0.62f, 0.42f));
            shader.setFloat(prefix + "constant", 1.0f);
            shader.setFloat(prefix + "linear", 0.045f);
            shader.setFloat(prefix + "quadratic", 0.008f);
            shader.setBool(prefix + "isEnabled", isEnabled);
        }
    }

private:
    float width, depth, poleSpacing;
    Cylinder cylinder;
    Sphere sphere;
    unsigned int fenceVAO;
    unsigned int instanceVBO;
    unsigned int cylinderVertexCount;
    std::vector<glm::mat4> instanceMatrices;
    std::vector<glm::mat4> supports;
    std::vector<glm::vec3> bulbPositions;

    void generateFence() {
        float halfW = width / 2.0f;
        float halfD = depth / 2.0f;

        // Front opening width
        float openingWidth = 8.0f; // Increased buffer to avoid clipping gate pillars

        // Precise locations for 12 elegant side-lamps
        std::vector<glm::vec3> lampSpots = {
            // Corners
            glm::vec3(-halfW, 0, -halfD), glm::vec3(halfW, 0, -halfD),
            glm::vec3(-halfW, 0, halfD),  glm::vec3(halfW, 0, halfD),
            // Mid-sides (Left/Right)
            glm::vec3(-halfW, 0, 0),      glm::vec3(halfW, 0, 0),
            // Mid-sides (Back)
            glm::vec3(-halfW/2.0f, 0, -halfD), glm::vec3(halfW/2.0f, 0, -halfD),
            // Near opening (Front)
            glm::vec3(-openingWidth/2.0f - 1.0f, 0, halfD), glm::vec3(openingWidth/2.0f + 1.0f, 0, halfD),
            // Mid-sides (Front)
            glm::vec3(-halfW + 5.0f, 0, halfD), glm::vec3(halfW - 5.0f, 0, halfD)
        };

        // Trace sides
        auto addSide = [&](glm::vec3 start, glm::vec3 end, bool isFront = false) {
            float dist = glm::distance(start, end);
            int steps = (int)(dist / poleSpacing);
            glm::vec3 dir = glm::normalize(end - start);
            
            for (int i = 0; i <= steps; ++i) {
                glm::vec3 pos = start + dir * (i * poleSpacing);
                
                // Skip opening on front
                if (isFront && glm::abs(pos.x) < openingWidth/2.0f) continue;

                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, pos);
                
                // Handmade variation
                float height = 3.2f + (rand() % 40) / 100.0f; // Increased height to block background
                float tiltX = (rand() % 10 - 5) / 100.0f;
                float tiltZ = (rand() % 10 - 5) / 100.0f;
                
                model = glm::rotate(model, tiltX, glm::vec3(1,0,0));
                model = glm::rotate(model, tiltZ, glm::vec3(0,0,1));
                model = glm::scale(model, glm::vec3(0.08f, height, 0.08f)); // Single bamboo thickness
                
                instanceMatrices.push_back(model);

                // Add a bulb every 8 poles as requested
                if (i % 8 == 0) {
                    bulbPositions.push_back(pos + glm::vec3(0, height, 0));
                }
            }

            // Horizontal supports (Bera rails)
            for (float h : {0.8f, 1.8f, 2.8f}) {
                auto addSupport = [&](glm::vec3 s, glm::vec3 e) {
                    float d = glm::distance(s, e);
                    if (d < 0.2f) return;
                    glm::vec3 center = (s + e) * 0.5f + glm::vec3(0, h, 0);
                    
                    glm::mat4 m = glm::mat4(1.0f);
                    m = glm::translate(m, s + glm::vec3(0, h, 0)); // Anchor at start point s
                    
                    // Rotation logic
                    float rotY = atan2(e.x - s.x, e.z - s.z);
                    m = glm::rotate(m, rotY, glm::vec3(0, 1, 0));
                    m = glm::rotate(m, glm::radians(90.0f), glm::vec3(1, 0, 0));
                    m = glm::scale(m, glm::vec3(0.04f, d, 0.04f));
                    supports.push_back(m);
                };

                if (isFront) {
                    float buffer = openingWidth / 2.0f;
                    // Left piece
                    addSupport(start, glm::vec3(-buffer, start.y, start.z));
                    // Right piece
                    addSupport(glm::vec3(buffer, end.y, end.z), end);
                } else {
                    addSupport(start, end);
                }
            }
        };

        // Right
        addSide(glm::vec3(halfW, 0, -halfD), glm::vec3(halfW, 0, halfD));
        // Left
        addSide(glm::vec3(-halfW, 0, halfD), glm::vec3(-halfW, 0, -halfD));
        // Back
        addSide(glm::vec3(halfW, 0, -halfD), glm::vec3(-halfW, 0, -halfD));
        // Front (with opening)
        addSide(glm::vec3(-halfW, 0, halfD), glm::vec3(halfW, 0, halfD), true);
    }

    void setupInstancing() {
        fenceVAO = cylinder.VAO;
        cylinderVertexCount = cylinder.vertexCount;

        glBindVertexArray(fenceVAO);
        glGenBuffers(1, &instanceVBO);
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, instanceMatrices.size() * sizeof(glm::mat4), &instanceMatrices[0], GL_STATIC_DRAW);

        // Matrix needs 4 attribute locations (3, 4, 5, 6)
        for (unsigned int i = 0; i < 4; i++) {
            glEnableVertexAttribArray(3 + i);
            glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(i * sizeof(glm::vec4)));
            glVertexAttribDivisor(3 + i, 1);
        }
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
};

#endif
