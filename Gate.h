#ifndef GATE_H
#define GATE_H

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"
#include "cylinder.h"

class Gate {
public:
    Gate(unsigned int melaTex) : melaTexture(melaTex), cylinder(12) {
        setupSignVAO();
        setupBuntingVAO();
    }

    void draw(Shader& shader, glm::vec3 pos) {
        shader.use();
        shader.setBool("isInstanced", false);

        // 1. Pillars
        float pillarHeight = 4.0f;
        float openingWidth = 6.0f;
        for (float side : {-1.0f, 1.0f}) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, pos + glm::vec3(side * openingWidth / 2.0f, 0, 0));
            model = glm::scale(model, glm::vec3(0.2f, pillarHeight, 0.2f));
            shader.setMat4("model", model);
            shader.setVec3("objectColor", glm::vec3(0.4f, 0.3f, 0.15f));
            cylinder.draw();
        }

        // 2. Arch (Semi-circle)
        int segments = 12;
        float radius = openingWidth / 2.0f;
        for (int i = 0; i < segments; ++i) {
            float angle1 = glm::pi<float>() * i / segments;
            float angle2 = glm::pi<float>() * (i + 1) / segments;
            
            glm::vec3 p1(cos(angle1) * radius, sin(angle1) * radius, 0);
            glm::vec3 p2(cos(angle2) * radius, sin(angle2) * radius, 0);
            
            glm::vec3 mid = (p1 + p2) * 0.5f + glm::vec3(0, pillarHeight - 0.45f, 0); // Lowered more to ensure connection
            glm::vec3 dir = p2 - p1;
            float len = glm::length(dir);
            
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, pos + mid);
            float angle = atan2(dir.y, dir.x);
            model = glm::rotate(model, angle, glm::vec3(0, 0, 1));
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 0, 1));
            model = glm::scale(model, glm::vec3(0.18f, len + 0.3f, 0.18f)); // Thicker and longer to overlap
            
            shader.setMat4("model", model);
            cylinder.draw();
        }

        // 3. Signboard (Brightened with emission)
        shader.setBool("useTexture", true);
        shader.setVec3("objectColor", glm::vec3(1.0f, 1.0f, 1.0f)); 
        shader.setFloat("specularStrength", 0.0f); // Non-reflective sign
        shader.setVec3("emission", glm::vec3(0.12f, 0.12f, 0.12f)); // Subtle visibility
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, melaTexture);
        shader.setInt("texture_diffuse", 0);
        
        glm::mat4 signModel = glm::mat4(1.0f);
        signModel = glm::translate(signModel, pos + glm::vec3(0, pillarHeight + 1.25f, 0.15f));
        signModel = glm::scale(signModel, glm::vec3(4.2f, 2.1f, 1.0f));
        shader.setMat4("model", signModel);
        
        glBindVertexArray(signVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        shader.setVec3("emission", glm::vec3(0.0f)); // Reset emission

        // 4. Bunting Flags (More dense for festive look)
        shader.setBool("useTexture", false);
        int numFlags = 15;
        glBindVertexArray(buntingVAO);
        for (int i = 0; i < numFlags; ++i) {
            float t = (float)i / (float)(numFlags - 1);
            float x = -radius + t * radius * 2.0f;
            float y = pillarHeight + 0.5f - 0.35f * sin(t * glm::pi<float>());
            
            glm::mat4 bModel = glm::mat4(1.0f);
            bModel = glm::translate(bModel, pos + glm::vec3(x, y, 0.2f)); 
            bModel = glm::scale(bModel, glm::vec3(0.3f, 0.4f, 1.0f));
            shader.setMat4("model", bModel);
            
            // Alternating festive colors
            glm::vec3 colors[] = { {1,0,0}, {0,1,0}, {0,0,1}, {1,1,0}, {1,0,1} };
            shader.setVec3("objectColor", colors[i % 5]);
            
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }
        glBindVertexArray(0);
    }

private:
    unsigned int melaTexture;
    Cylinder cylinder;
    unsigned int signVAO, buntingVAO;

    void setupSignVAO() {
        float vertices[] = {
            // Standard CCW Front Face
            // positions(xyz)   // tex(uv)  // normals(xyz)
            -0.5f,  0.5f, 0.0f,  0.0f, 1.0f,  0,0,1,
            -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,  0,0,1,
             0.5f, -0.5f, 0.0f,  1.0f, 0.0f,  0,0,1,

            -0.5f,  0.5f, 0.0f,  0.0f, 1.0f,  0,0,1,
             0.5f, -0.5f, 0.0f,  1.0f, 0.0f,  0,0,1,
             0.5f,  0.5f, 0.0f,  1.0f, 1.0f,  0,0,1
        };
        unsigned int VBO;
        glGenVertexArrays(1, &signVAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(signVAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    void setupBuntingVAO() {
        float vertices[] = {
            -0.5f,  0.5f, 0.0f,  0,0,1,
             0.5f,  0.5f, 0.0f,  0,0,1,
             0.0f, -0.5f, 0.0f,  0,0,1
        };
        unsigned int VBO;
        glGenVertexArrays(1, &buntingVAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(buntingVAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }
};

#endif
