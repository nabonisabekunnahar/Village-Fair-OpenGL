#ifndef FERRIS_WHEEL_H
#define FERRIS_WHEEL_H

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"
#include "cylinder.h"
#include "sphere.h"

class FerrisWheel {
public:
    FerrisWheel() : cylinder(16), sphere(1.0f, 36, 18) {
    }

    void draw(Shader& shader, glm::vec3 pos, float time) {
        shader.use();
        shader.setBool("isInstanced", false);
        shader.setBool("useTexture", false);

        float rotationSpeed = 0.8f; // Increased speed
        float wheelAngle = time * rotationSpeed;
        float radius = 5.5f;
        float centerY = 7.8f; // Increased height to prevent gondola bottom-clipping
        
        // --- Geometry Helper ---
        // Draws a cylinder between any two 3D points
        auto drawCyl = [&](glm::vec3 p1, glm::vec3 p2, float thickness, glm::vec3 color) {
            glm::vec3 dir = p2 - p1;
            float len = glm::length(dir);
            if (len < 0.001f) return;
            dir /= len;
            glm::vec3 up(0, 1, 0);
            glm::mat4 m(1.0f);
            m = glm::translate(m, p1);
            
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
            cylinder.draw();
        };

        // Helper to draw a cylinder locally within a parent matrix
        auto drawLocalCyl = [&](glm::mat4 baseMat, glm::vec3 localP1, glm::vec3 localP2, float thickness, glm::vec3 color) {
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
            cylinder.draw();
        };

        // --- 1. Main Support Structure ---
        glm::vec3 supportColor(0.05f, 0.15f, 0.35f); // Dark industrial blue
        
        for (float zOff : {-1.4f, 1.4f}) {
            // Main vertical mast
            drawCyl(pos + glm::vec3(0, 0, zOff), pos + glm::vec3(0, centerY, zOff), 0.3f, supportColor);
            
            // A-frame legs
            for (float xOff : {-3.5f, 3.5f}) {
                drawCyl(pos + glm::vec3(xOff, 0, zOff), pos + glm::vec3(0, centerY, zOff), 0.2f, supportColor);
            }
        }

        // --- 2. Central Hub/Axle ---
        // Axle passing through the center spanning across Z
        drawCyl(pos + glm::vec3(0, centerY, 1.6f), pos + glm::vec3(0, centerY, -1.6f), 0.4f, glm::vec3(0.2f, 0.2f, 0.25f));
        
        // Hub Decorative Star Plates (Front and Back)
        for (float zOff : {-1.4f, 1.4f}) {
            glm::mat4 plate = glm::mat4(1.0f);
            plate = glm::translate(plate, pos + glm::vec3(0, centerY, zOff));
            // Rotate the plates with the wheel (around Z axis)
            plate = glm::rotate(plate, wheelAngle, glm::vec3(0, 0, 1)); 
            plate = glm::scale(plate, glm::vec3(1.2f, 1.2f, 0.05f)); // Flat disc on XY plane
            shader.setMat4("model", plate);
            shader.setVec3("objectColor", glm::vec3(0.85f, 0.85f, 0.9f)); // Off-white
            sphere.drawSphere(shader);
            
            // Inner blue star
            glm::mat4 innerPlate = glm::scale(plate, glm::vec3(0.4f, 0.4f, 1.2f));
            shader.setMat4("model", innerPlate);
            shader.setVec3("objectColor", glm::vec3(0.1f, 0.2f, 0.5f)); 
            sphere.drawSphere(shader);
        }

        // --- 3. Rotating Wheel Structure (Hierarchical) ---
        // Create the master rotation matrix for the wheel to prevent inter-frame floating point jitter
        glm::mat4 wheelMatrix = glm::mat4(1.0f);
        wheelMatrix = glm::translate(wheelMatrix, pos + glm::vec3(0, centerY, 0));
        wheelMatrix = glm::rotate(wheelMatrix, wheelAngle, glm::vec3(0, 0, 1));

        int numSpokes = 8;
        for (int i = 0; i < numSpokes; ++i) {
            // Local angles (STATIC relative to wheelMatrix)
            float localAngle1 = (float)i / numSpokes * 2.0f * glm::pi<float>();
            float localAngle2 = (float)(i+1) / numSpokes * 2.0f * glm::pi<float>();
            
            // Local tips (Z = 0 relative to wheel center)
            glm::vec3 localTip1(cos(localAngle1) * radius, sin(localAngle1) * radius, 0);
            glm::vec3 localTip2(cos(localAngle2) * radius, sin(localAngle2) * radius, 0);
            
            // Draw Spoke Rings (Front and Back)
            for (float zOff : {-1.2f, 1.2f}) {
                glm::vec3 center(0, 0, zOff);
                glm::vec3 r1 = localTip1 + glm::vec3(0, 0, zOff);
                glm::vec3 r2 = localTip2 + glm::vec3(0, 0, zOff);
                
                // Spoke (Gold/Yellow)
                drawLocalCyl(wheelMatrix, center, r1, 0.1f, glm::vec3(0.85f, 0.75f, 0.2f));
                
                // Tension Cables (Outer ring)
                drawLocalCyl(wheelMatrix, r1, r2, 0.04f, glm::vec3(0.2f, 0.2f, 0.2f));
                
                // Tension Cables (Inner star)
                glm::vec3 mid1 = center + (r1 - center) * 0.4f;
                glm::vec3 mid2 = center + (r2 - center) * 0.4f;
                drawLocalCyl(wheelMatrix, mid1, mid2, 0.03f, glm::vec3(0.2f, 0.2f, 0.2f));
            }

            // Draw Cage Gondola
            // The gondolas MUST NOT rotate with the wheelMatrix (they stay upright).
            // So we calculate their world position based on wheelAngle!
            float worldAngle = wheelAngle + localAngle1;
            glm::vec3 seatWorldPos = pos + glm::vec3(cos(worldAngle) * radius, centerY + sin(worldAngle) * radius, 0);
            
            drawCageGondola(shader, seatWorldPos, 0, i, drawCyl);
        }

    }

private:
    Cylinder cylinder;
    Sphere sphere;

    template<typename F>
    void drawCageGondola(Shader& shader, glm::vec3 pos, float counterRotation, int index, F& drawCyl) {
        glm::vec3 primaryColors[] = {
            {0.9f, 0.2f, 0.3f}, // Red
            {0.1f, 0.8f, 0.2f}, // Green
            {0.9f, 0.5f, 0.1f}, // Orange
            {0.2f, 0.4f, 0.9f}, // Blue
            {0.9f, 0.1f, 0.6f}, // Pink
            {0.9f, 0.8f, 0.1f}, // Yellow
            {0.1f, 0.8f, 0.8f}, // Cyan
            {0.8f, 0.2f, 0.1f}  // Dark Orange
        };
        
        glm::vec3 mainColor = primaryColors[index % 8];
        glm::vec3 trimColor = glm::vec3(0.9f, 0.9f, 0.9f); 
        glm::vec3 darkMetal = glm::vec3(0.2f, 0.2f, 0.2f);
        
        float cageWidth = 0.75f;  // X radius (Increased)
        float cageDepth = 1.05f;  // Z radius (Increased)
        float cageHeight = 1.6f;  // Total hanging depth (Increased)
        float tubHeight = 0.8f;   // Solid bucket height
        
        // 0. Pivot Pin directly from spoke to spoke holding the roof
        drawCyl(pos + glm::vec3(0, 0, -1.2f), pos + glm::vec3(0, 0, 1.2f), 0.05f, darkMetal);

        // 1. Roof Dome
        glm::mat4 roof = glm::mat4(1.0f);
        roof = glm::translate(roof, pos);
        roof = glm::scale(roof, glm::vec3(cageWidth, 0.35f, cageDepth));
        shader.setMat4("model", roof);
        shader.setVec3("objectColor", mainColor);
        sphere.drawSphere(shader);
        
        // Roof tip ornament
        drawCyl(pos + glm::vec3(0, 0.3f, 0), pos + glm::vec3(0, 0.6f, 0), 0.06f, glm::vec3(0.85f, 0.75f, 0.2f));

        // 2. Lower Tub (Bucket)
        glm::mat4 tub = glm::mat4(1.0f);
        tub = glm::translate(tub, pos + glm::vec3(0, -cageHeight, 0));
        tub = glm::scale(tub, glm::vec3(cageWidth * 0.95f, tubHeight, cageDepth * 0.95f));
        shader.setMat4("model", tub);
        shader.setVec3("objectColor", mainColor);
        cylinder.draw();
        
        // 3. Floor
        glm::mat4 floor = glm::mat4(1.0f);
        floor = glm::translate(floor, pos + glm::vec3(0, -cageHeight, 0));
        floor = glm::scale(floor, glm::vec3(cageWidth, 0.05f, cageDepth));
        shader.setMat4("model", floor);
        shader.setVec3("objectColor", darkMetal);
        sphere.drawSphere(shader);
        
        // Inner Seat Block
        glm::mat4 seat = glm::mat4(1.0f);
        seat = glm::translate(seat, pos + glm::vec3(0, -cageHeight + 0.15f, 0));
        seat = glm::scale(seat, glm::vec3(cageWidth * 0.7f, 0.1f, cageDepth * 0.7f));
        shader.setMat4("model", seat);
        shader.setVec3("objectColor", trimColor);
        cylinder.draw();

        // 4. Vertical Cage Bars
        int numBars = 8;
        for (int i = 0; i < numBars; ++i) {
            float angle = (float)i / numBars * 2.0f * glm::pi<float>();
            float barX = cos(angle) * cageWidth * 0.95f;
            float barZ = sin(angle) * cageDepth * 0.95f;
            
            drawCyl(pos + glm::vec3(barX, -cageHeight, barZ), pos + glm::vec3(barX, 0.0f, barZ), 0.025f, trimColor);
        }
    }
};

#endif

