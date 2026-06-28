#ifndef SWINGRIDE_H
#define SWINGRIDE_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"
#include "cylinder.h"
#include "sphere.h"

class SwingRide {
public:
    SwingRide(unsigned int sofaTex, unsigned int _platTex) : sofaTexture(sofaTex), platTexture(_platTex), cylinder(16) {
        setupTexturedCubeVAO();
        setupPlatformVAO();
    }

    void draw(Shader& shader, glm::vec3 pos, float time) {
        float rotationSpeed = 1.0f;
        float wheelAngle = time * rotationSpeed;
        float baseHeight = 7.0f; 
        
        auto drawCyl = [&](glm::mat4 baseMat, glm::vec3 localP1, glm::vec3 localP2, float thickness, glm::vec3 color) {
            glm::vec3 dir = localP2 - localP1;
            float len = glm::length(dir);
            if (len < 0.001f) return;
            dir /= len;
            glm::vec3 up(0, 1, 0);
            glm::mat4 m = glm::translate(baseMat, localP1);
            
            float dotP = glm::dot(up, dir);
            if (dotP < -0.999f) {
                m = glm::rotate(m, glm::pi<float>(), glm::vec3(1, 0, 0));
            } else if (dotP < 0.999f) {
                glm::vec3 axis = glm::normalize(glm::cross(up, dir));
                float angle = acos(dotP);
                m = glm::rotate(m, angle, axis);
            }
            
            m = glm::scale(m, glm::vec3(thickness, len, thickness));
            shader.setMat4("model", m);
            shader.setVec3("objectColor", color);
            shader.setBool("useTexture", false);
            cylinder.draw();
        };

        // 1. Central Pillar & Platform
        glm::mat4 identity = glm::mat4(1.0f);
        identity = glm::translate(identity, pos);
        
        // Procedural Solid Circular Platform (Bezier Evaluation)
        glm::mat4 platMat = glm::translate(identity, glm::vec3(0.0f, 0.1f, 0.0f));
        platMat = glm::scale(platMat, glm::vec3(7.5f, 1.0f, 7.5f));
        shader.setMat4("model", platMat);
        shader.setBool("useTexture", true);
        shader.setInt("texture_diffuse", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, platTexture);
        shader.setVec3("objectColor", glm::vec3(0.5f, 0.6f, 0.8f)); // Fallback if texture fails
        
        glBindVertexArray(platformVAO);
        glDrawArrays(GL_TRIANGLES, 0, platformVertexCount);
        glBindVertexArray(0);
        shader.setBool("useTexture", false);

        // Main Pillar
        drawCyl(identity, glm::vec3(0, 0, 0), glm::vec3(0, baseHeight, 0), 0.6f, glm::vec3(0.4f, 0.75f, 0.4f));

        // 2. Hub Master Matrix
        glm::mat4 hubMatrix = glm::mat4(1.0f);
        hubMatrix = glm::translate(hubMatrix, pos + glm::vec3(0, baseHeight, 0));
        hubMatrix = glm::rotate(hubMatrix, wheelAngle, glm::vec3(0, -1, 0)); 

        // Top Red Dome
        glm::mat4 domeMat = glm::scale(hubMatrix, glm::vec3(0.9f, 0.5f, 0.9f));
        shader.setMat4("model", domeMat);
        shader.setVec3("objectColor", glm::vec3(0.9f, 0.2f, 0.2f)); 
        shader.setBool("useTexture", false);
        sphere.drawSphere(shader);

        // 3. Main Arms and Struts
        int numArms = 8;
        float armLength = 6.5f;
        float armDrop = 3.5f; 

        for (int i = 0; i < numArms; ++i) {
            float angle = (float)i / numArms * 2.0f * glm::pi<float>();
            glm::vec3 armEnd(cos(angle) * armLength, -armDrop, sin(angle) * armLength);
            
            glm::vec3 armColor = (i % 2 == 0) ? glm::vec3(0.4f, 0.75f, 0.4f) : glm::vec3(0.9f, 0.2f, 0.2f); 
            drawCyl(hubMatrix, glm::vec3(0, 0, 0), armEnd, 0.15f, armColor);
            
            // Blue joint
            glm::mat4 jointMat = glm::translate(hubMatrix, armEnd);
            jointMat = glm::scale(jointMat, glm::vec3(0.25f));
            shader.setMat4("model", jointMat);
            shader.setVec3("objectColor", glm::vec3(0.2f, 0.3f, 0.9f));
            shader.setBool("useTexture", false);
            sphere.drawSphere(shader);

            // Green dropping strut
            glm::vec3 dropEnd = armEnd + glm::vec3(0, -1.5f, 0); 
            drawCyl(hubMatrix, armEnd, dropEnd, 0.12f, glm::vec3(0.3f, 0.8f, 0.4f)); 

            // Red horizontal support bar for seats
            glm::vec3 tang(-sin(angle), 0, cos(angle)); 
            float seatSpread = 0.9f;
            glm::vec3 leftPivot = dropEnd + tang * seatSpread;
            glm::vec3 rightPivot = dropEnd - tang * seatSpread;

            drawCyl(hubMatrix, leftPivot, rightPivot, 0.1f, glm::vec3(0.9f, 0.2f, 0.2f));

            float worldAngle = wheelAngle + angle;
            
            glm::vec3 pL = glm::vec3(hubMatrix * glm::vec4(leftPivot, 1.0f));
            glm::vec3 pR = glm::vec3(hubMatrix * glm::vec4(rightPivot, 1.0f));

            drawSeat(shader, pL, worldAngle);
            drawSeat(shader, pR, worldAngle);
        }
    }

private:
    unsigned int sofaTexture;
    unsigned int platTexture;
    unsigned int seatVAO, seatVBO;
    unsigned int platformVAO, platformVBO, platformVertexCount;
    Cylinder cylinder;
    Sphere sphere;

    void setupPlatformVAO() {
        std::vector<float> vertices;
        float k = 0.5522847498f;
        glm::vec2 P[4][4] = {
            {{1, 0}, {1, k}, {k, 1}, {0, 1}},
            {{0, 1}, {-k, 1}, {-1, k}, {-1, 0}},
            {{-1, 0}, {-1, -k}, {-k, -1}, {0, -1}},
            {{0, -1}, {k, -1}, {1, -k}, {1, 0}}
        };
        
        int segments = 20; 
        std::vector<glm::vec2> boundary;
        for(int i=0; i<4; ++i) {
            for(int j=0; j<segments; ++j) {
                float t = (float)j / segments;
                float u = 1.0f - t;
                glm::vec2 p = u*u*u * P[i][0] + 3.0f*u*u*t * P[i][1] + 3.0f*u*t*t * P[i][2] + t*t*t * P[i][3];
                boundary.push_back(p);
            }
        }
        
        float h = 0.1f;
        
        auto addVert = [&](glm::vec3 pos, glm::vec3 norm, glm::vec2 uv) {
            vertices.push_back(pos.x); vertices.push_back(pos.y); vertices.push_back(pos.z);
            vertices.push_back(norm.x); vertices.push_back(norm.y); vertices.push_back(norm.z);
            vertices.push_back(uv.x); vertices.push_back(uv.y);
        };

        int n = boundary.size();
        
        // 1. Top Cap
        glm::vec3 topNorm(0, 1, 0);
        glm::vec3 centerTop(0, h, 0);
        for(int i=0; i<n; ++i) {
            glm::vec2 b1 = boundary[i];
            glm::vec2 b2 = boundary[(i+1)%n];
            addVert(centerTop, topNorm, glm::vec2(0.5f, 0.5f));
            addVert(glm::vec3(b1.x, h, b1.y), topNorm, b1 * 0.5f + glm::vec2(0.5f));
            addVert(glm::vec3(b2.x, h, b2.y), topNorm, b2 * 0.5f + glm::vec2(0.5f));
        }

        // 2. Bottom Cap
        glm::vec3 botNorm(0, -1, 0);
        glm::vec3 centerBot(0, -h, 0);
        for(int i=0; i<n; ++i) {
            glm::vec2 b1 = boundary[i];
            glm::vec2 b2 = boundary[(i+1)%n];
            addVert(centerBot, botNorm, glm::vec2(0.5f, 0.5f));
            addVert(glm::vec3(b2.x, -h, b2.y), botNorm, b2 * 0.5f + glm::vec2(0.5f));
            addVert(glm::vec3(b1.x, -h, b1.y), botNorm, b1 * 0.5f + glm::vec2(0.5f));
        }

        // 3. Side Walls
        for(int i=0; i<n; ++i) {
            glm::vec2 b1 = boundary[i];
            glm::vec2 b2 = boundary[(i+1)%n];
            glm::vec3 n1 = glm::normalize(glm::vec3(b1.x, 0, b1.y));
            glm::vec3 n2 = glm::normalize(glm::vec3(b2.x, 0, b2.y));

            glm::vec3 top1(b1.x, h, b1.y);
            glm::vec3 bot1(b1.x, -h, b1.y);
            glm::vec3 top2(b2.x, h, b2.y);
            glm::vec3 bot2(b2.x, -h, b2.y);

            float u1 = (float)i / n;
            float u2 = (float)(i+1) / n;

            addVert(top1, n1, glm::vec2(u1, 1.0f));
            addVert(bot1, n1, glm::vec2(u1, 0.0f));
            addVert(top2, n2, glm::vec2(u2, 1.0f));

            addVert(bot1, n1, glm::vec2(u1, 0.0f));
            addVert(bot2, n2, glm::vec2(u2, 0.0f));
            addVert(top2, n2, glm::vec2(u2, 1.0f));
        }
        
        platformVertexCount = vertices.size() / 8;
        
        glGenVertexArrays(1, &platformVAO);
        glGenBuffers(1, &platformVBO);
        glBindVertexArray(platformVAO);
        glBindBuffer(GL_ARRAY_BUFFER, platformVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glBindVertexArray(0);
    }

    void drawSeat(Shader& shader, glm::vec3 seatWorldPos, float facingAngle) {
        glm::mat4 seatMat = glm::mat4(1.0f);
        seatMat = glm::translate(seatMat, seatWorldPos);
        
        // Face tangential (velocity direction)
        seatMat = glm::rotate(seatMat, -facingAngle + glm::pi<float>()/2.0f, glm::vec3(0, 1, 0));
        
        // Removed centrifugal tilt completely to keep the chairs completely flat and steady.
        
        // Hang down from the red bracket slightly
        seatMat = glm::translate(seatMat, glm::vec3(0, -0.8f, 0));
        
        // Scale the entire seat up significantly so it is large and visible
        seatMat = glm::scale(seatMat, glm::vec3(1.5f)); 
        
        shader.setBool("useTexture", true);
        shader.setInt("texture_diffuse", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sofaTexture);
        shader.setVec3("objectColor", glm::vec3(0.85f, 0.85f, 0.85f)); 
        
        glBindVertexArray(seatVAO);

        // Bucket Base Cushion (Much thicker and wider so the sitting area is clearly visible)
        glm::mat4 bottom = glm::translate(seatMat, glm::vec3(0.0f, 0.15f, 0.0f));
        bottom = glm::scale(bottom, glm::vec3(1.1f, 0.3f, 0.9f));
        shader.setMat4("model", bottom);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Bucket Backrest (Much taller and thicker)
        glm::mat4 back = glm::translate(seatMat, glm::vec3(0.0f, 0.65f, -0.35f));
        back = glm::rotate(back, glm::radians(-5.0f), glm::vec3(1, 0, 0));
        back = glm::scale(back, glm::vec3(1.1f, 0.9f, 0.2f));
        shader.setMat4("model", back);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Tub Style Left Armrest/Guard (matches the backrest height nicely)
        glm::mat4 left = glm::translate(seatMat, glm::vec3(-0.45f, 0.45f, 0.0f));
        left = glm::scale(left, glm::vec3(0.2f, 0.5f, 0.9f));
        shader.setMat4("model", left);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        // Tub Style Right Armrest/Guard
        glm::mat4 right = glm::translate(seatMat, glm::vec3(0.45f, 0.45f, 0.0f));
        right = glm::scale(right, glm::vec3(0.2f, 0.5f, 0.9f));
        shader.setMat4("model", right);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glBindVertexArray(0);
        shader.setBool("useTexture", false);
    }

    void setupTexturedCubeVAO() {
        float vertices[] = {
            // Pos              // Normal           // UV
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
             0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
             0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
             0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
             0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
             0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
        };
        glGenVertexArrays(1, &seatVAO);
        glGenBuffers(1, &seatVBO);
        glBindVertexArray(seatVAO);
        glBindBuffer(GL_ARRAY_BUFFER, seatVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glBindVertexArray(0);
    }
};

#endif
