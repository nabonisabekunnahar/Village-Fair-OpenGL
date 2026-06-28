#ifndef STALL_H
#define STALL_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <string>
#include "shader.h"
#include "cylinder.h"
#include "pyramid.h"
#include "sphere.h"
#include "BezierPlate.h"

class Stall {
public:
    Stall(unsigned int cubeVAO) : cubeVAO(cubeVAO), cylinder(16), sphere(0.4f, 36, 18), bezierPlate(), sareePriceTex(0) {}

    void setSareeTextures(const std::vector<unsigned int>& texs, unsigned int priceTex) {
        sareeTextures = texs;
        sareePriceTex = priceTex;
    }

    void setupPointLights(Shader& shader, glm::vec3 pos, float rotationY, int startIndex, bool mainLanternsOn) {
        glm::mat4 baseModel = glm::mat4(1.0f);
        baseModel = glm::translate(baseModel, pos);
        baseModel = glm::rotate(baseModel, glm::radians(rotationY), glm::vec3(0, 1, 0));

        float counterZ = 0.5f;
        float pillarX = 1.6f;
        float beamY = 2.45f;
        
        std::vector<glm::vec3> lanternPositions = {
            {-pillarX + 0.4f, beamY - 0.15f, counterZ},
            { 0.0f,           beamY - 0.15f, counterZ},
            { pillarX - 0.4f, beamY - 0.15f, counterZ}
        };

        for (int i = 0; i < 3; ++i) {
            std::string prefix = "pointLights[" + std::to_string(startIndex + i) + "].";
            glm::vec3 worldLightPos = glm::vec3(baseModel * glm::vec4(lanternPositions[i], 1.0f));
            shader.setVec3(prefix + "position", worldLightPos);
            shader.setVec3(prefix + "color", mainLanternsOn ? glm::vec3(0.30f, 0.18f, 0.06f) : glm::vec3(0.0f));
            shader.setFloat(prefix + "constant", 1.0f);
            shader.setFloat(prefix + "linear", 0.7f);
            shader.setFloat(prefix + "quadratic", 1.8f);
            shader.setBool(prefix + "isEnabled", mainLanternsOn);
        }
    }

    void draw(Shader& shader, glm::vec3 pos, float rotationY, unsigned int fabricTex, unsigned int bambooTex, int lightIdx, float time, bool fairyLightsOn, bool mainLanternsOn, int stallType, unsigned int ceramicTex, unsigned int tA=0, unsigned int tB=0, unsigned int tC=0, unsigned int tD=0) {
        shader.setVec3("objectColor", glm::vec3(1.0f, 1.0f, 1.0f)); 
        glm::mat4 baseModel = glm::mat4(1.0f);
        baseModel = glm::translate(baseModel, pos);
        baseModel = glm::rotate(baseModel, glm::radians(rotationY), glm::vec3(0, 1, 0));
 
        float px = 2.8f; float pz = 2.1f;
        float pillarHeight = 4.5f;

        // 1. Table with Skirt
        glBindVertexArray(cubeVAO);
        shader.setBool("useTexture", true);
        shader.setInt("texture_diffuse", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fabricTex);

        float totalDepth = pz * 2.0f;
        float counterDepth = totalDepth * 0.4f;
        float counterZ = pz - (counterDepth / 2.0f);

        glm::mat4 skirtModel = glm::translate(baseModel, glm::vec3(0, 0.6f, counterZ)); 
        skirtModel = glm::scale(skirtModel, glm::vec3(5.6f, 1.2f, counterDepth)); 
        shader.setMat4("model", skirtModel);
        shader.setVec3("objectColor", glm::vec3(1.0f, 1.0f, 1.0f));
        glDrawArrays(GL_TRIANGLES, 0, 36);
 
        shader.setBool("useTexture", false);
        glm::mat4 counterTop = glm::translate(baseModel, glm::vec3(0, 1.2f, counterZ));
        counterTop = glm::scale(counterTop, glm::vec3(5.8f, 0.1f, counterDepth + 0.1f));
        shader.setMat4("model", counterTop);
        shader.setVec3("objectColor", glm::vec3(0.95f, 0.95f, 0.95f)); 
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // 2. Pillars
        shader.setVec3("objectColor", glm::vec3(1.0f, 1.0f, 1.0f));
        shader.setBool("useTexture", true);
        glBindTexture(GL_TEXTURE_2D, bambooTex);
        drawPillar(shader, baseModel, glm::vec3(-px, 0, -pz), pillarHeight);
        drawPillar(shader, baseModel, glm::vec3( px, 0, -pz), pillarHeight);
        drawPillar(shader, baseModel, glm::vec3(-px, 0,  pz), pillarHeight);
        drawPillar(shader, baseModel, glm::vec3( px, 0,  pz), pillarHeight);

        // 3. Walls
        shader.setVec3("objectColor", glm::vec3(1.0f, 1.0f, 1.0f));
        glBindVertexArray(cubeVAO);
        glBindTexture(GL_TEXTURE_2D, fabricTex);
        glm::mat4 backWall = glm::translate(baseModel, glm::vec3(0.0f, pillarHeight/2.0f + 0.6f, -pz));
        backWall = glm::scale(backWall, glm::vec3(5.7f, pillarHeight + 1.2f, 0.1f));
        shader.setMat4("model", backWall);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glm::mat4 leftWall = glm::translate(baseModel, glm::vec3(-px, pillarHeight/2.0f + 0.6f, 0.0f));
        leftWall = glm::scale(leftWall, glm::vec3(0.1f, pillarHeight + 1.2f, 4.2f));
        shader.setMat4("model", leftWall);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glm::mat4 rightWall = glm::translate(baseModel, glm::vec3(px, pillarHeight/2.0f + 0.6f, 0.0f));
        rightWall = glm::scale(rightWall, glm::vec3(0.1f, pillarHeight + 1.2f, 4.2f));
        shader.setMat4("model", rightWall);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // 4. Bunting
        shader.setBool("useTexture", false);
        for(int i=-4; i<=4; ++i) {
            glm::mat4 flag = glm::translate(baseModel, glm::vec3(i * 0.65f, pillarHeight + 0.1f, pz));
            flag = glm::scale(flag, glm::vec3(0.4f, 0.4f, 0.1f));
            shader.setMat4("model", flag);
            shader.setVec3("objectColor", (i%2==0) ? glm::vec3(1,1,0) : glm::vec3(0,1,1));
            pyramid.draw();
        }

        // 4.1 Fairy Lights
        if (fairyLightsOn) {
            float blink = (static_cast<float>(sin(time * 8.0f)) * 0.5f + 0.5f);
            for (int i = -8; i <= 8; ++i) {
                glm::mat4 lightOrb = glm::translate(baseModel, glm::vec3(i * 0.35f, pillarHeight + 0.05f, pz + 0.05f));
                lightOrb = glm::scale(lightOrb, glm::vec3(0.16f));
                shader.setMat4("model", lightOrb);
                glm::vec3 lightColor = (i % 2 == 0) ? glm::vec3(1.0f, 0.0f, 0.0f) : glm::vec3(1.0f, 1.0f, 1.0f);
                shader.setVec3("objectColor", lightColor);
                shader.setVec3("emission", lightColor * blink * 0.7f);
                sphere.drawSphere(shader);
            }
            shader.setVec3("emission", glm::vec3(0.0f));
        }

        // 5. Roof
        shader.setVec3("objectColor", glm::vec3(1.0f, 1.0f, 1.0f));
        shader.setBool("useTexture", true);
        glBindTexture(GL_TEXTURE_2D, fabricTex);
        glm::mat4 roofModel = glm::translate(baseModel, glm::vec3(0, pillarHeight + 1.2f, 0));
        roofModel = glm::scale(roofModel, glm::vec3(6.5f, 3.5f, 5.0f));
        shader.setMat4("model", roofModel);
        pyramid.draw();

        // 6. Lanterns
        shader.setBool("useTexture", false);        // --- Lanterns ---
        std::vector<glm::vec3> lanternOffsets = {
            glm::vec3(-1.8f, pillarHeight - 1.2f, pz - 0.5f),
            glm::vec3( 0.0f, pillarHeight - 1.2f, pz - 0.5f),
            glm::vec3( 1.8f, pillarHeight - 1.2f, pz - 0.5f)
        };

        for (size_t i = 0; i < lanternOffsets.size(); ++i) {
            glm::vec3 lPos = lanternOffsets[i];
            
            // 1. Static Geometry (Always Visible)
            float socketTopY = 0.50f;
            float socketBaseY = 0.36f;

            // Wire from ceiling (Recalculated to connect ceiling to socket top)
            float wireTop = pillarHeight - lPos.y + 0.1f;
            float wireLen = wireTop - socketTopY;
            glm::mat4 wireMat = glm::translate(baseModel, lPos + glm::vec3(0, socketTopY, 0));
            wireMat = glm::scale(wireMat, glm::vec3(0.015f, wireLen, 0.015f));
            shader.setMat4("model", wireMat);
            shader.setVec3("objectColor", glm::vec3(0.08f));
            shader.setVec3("emission", glm::vec3(0.0f));
            cylinder.draw();

            // Top Socket/Cap
            glm::mat4 capMat = glm::translate(baseModel, lPos + glm::vec3(0, socketBaseY, 0));
            capMat = glm::scale(capMat, glm::vec3(0.12f, socketTopY - socketBaseY, 0.12f));
            shader.setMat4("model", capMat);
            shader.setVec3("objectColor", glm::vec3(0.06f));
            cylinder.draw();

            // Cage calculation
            float topRingY = 0.34f, midRingY = -0.05f, botRingY = -0.35f;
            float topRingR = 0.10f, midRingR = 0.25f, botRingR = 0.06f;
            
            glm::vec3 cageColor(0.12f, 0.08f, 0.05f);
            glm::vec3 activeCageGlow = mainLanternsOn ? glm::vec3(0.28f, 0.16f, 0.03f) : glm::vec3(0.0f);

            // Rings
            shader.setVec3("objectColor", cageColor);
            shader.setVec3("emission", activeCageGlow);
            
            glm::mat4 tr = glm::translate(baseModel, lPos + glm::vec3(0, topRingY, 0));
            tr = glm::scale(tr, glm::vec3(topRingR * 2.0f, 0.03f, topRingR * 2.0f));
            shader.setMat4("model", tr);
            cylinder.draw();

            glm::mat4 mr = glm::translate(baseModel, lPos + glm::vec3(0, midRingY, 0));
            mr = glm::scale(mr, glm::vec3(midRingR * 2.0f, 0.03f, midRingR * 2.0f));
            shader.setMat4("model", mr);
            cylinder.draw();

            glm::mat4 br = glm::translate(baseModel, lPos + glm::vec3(0, botRingY, 0));
            br = glm::scale(br, glm::vec3(botRingR * 2.0f, 0.03f, botRingR * 2.0f));
            shader.setMat4("model", br);
            cylinder.draw();

            // Bars
            glBindVertexArray(cubeVAO);
            glm::vec3 upDir(0, 1, 0);
            for (int b = 0; b < 6; ++b) {
                float ang = b * 60.0f * 3.14159f / 180.0f;
                float cs = cos(ang), sn = sin(ang);

                glm::vec3 p1(topRingR * cs, topRingY, topRingR * sn);
                glm::vec3 p2(midRingR * cs, midRingY, midRingR * sn);
                glm::vec3 mid1 = (p1 + p2) * 0.5f;
                float len1 = glm::length(p2 - p1);
                glm::vec3 dir1 = glm::normalize(p2 - p1);
                glm::vec3 ax1 = glm::cross(upDir, dir1);
                float agl1 = acos(glm::clamp(glm::dot(upDir, dir1), -1.0f, 1.0f));

                glm::mat4 bar1 = glm::translate(baseModel, lPos + mid1);
                if (glm::length(ax1) > 0.001f) bar1 = glm::rotate(bar1, agl1, glm::normalize(ax1));
                bar1 = glm::scale(bar1, glm::vec3(0.035f, len1, 0.035f)); 
                shader.setMat4("model", bar1);
                glDrawArrays(GL_TRIANGLES, 0, 36);

                glm::vec3 p3(botRingR * cs, botRingY, botRingR * sn);
                glm::vec3 mid2 = (p2 + p3) * 0.5f;
                float len2 = glm::length(p3 - p2);
                glm::vec3 dir2 = glm::normalize(p3 - p2);
                glm::vec3 ax2 = glm::cross(upDir, dir2);
                float agl2 = acos(glm::clamp(glm::dot(upDir, dir2), -1.0f, 1.0f));

                glm::mat4 bar2 = glm::translate(baseModel, lPos + mid2);
                if (glm::length(ax2) > 0.001f) bar2 = glm::rotate(bar2, agl2, glm::normalize(ax2));
                bar2 = glm::scale(bar2, glm::vec3(0.035f, len2, 0.035f));
                shader.setMat4("model", bar2);
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }

            // 2. Light Effects (Toggled)
            glm::mat4 bulbMat = glm::translate(baseModel, lPos + glm::vec3(0, -0.02f, 0));
            bulbMat = glm::scale(bulbMat, glm::vec3(0.20f, 0.35f, 0.20f)); // Slightly larger bulb
            shader.setMat4("model", bulbMat);
            
            if (mainLanternsOn) {
                shader.setVec3("objectColor", glm::vec3(1.0f, 0.85f, 0.45f));
                shader.setVec3("emission", glm::vec3(0.65f, 0.45f, 0.15f));
            } else {
                shader.setVec3("objectColor", glm::vec3(0.3f, 0.25f, 0.2f)); // Turned off look
                shader.setVec3("emission", glm::vec3(0.0f));
            }
            sphere.drawSphere(shader);
            shader.setVec3("emission", glm::vec3(0.0f));
        }

        // 7. Decorative Items
        if (stallType == 0) drawFoodItems(shader, baseModel, counterZ, ceramicTex);
        else if (stallType == 1) drawToyItems(shader, baseModel, counterZ, tA, tB, tC, tD);
        else if (stallType == 2) drawPotteryItems(shader, baseModel, counterZ);
        else if (stallType == 3) drawTeaItems(shader, baseModel, counterZ, tA, tB, tC);
        else if (stallType == 4) drawSareeItems(shader, baseModel, counterZ);
    }

private:
    unsigned int cubeVAO;
    Cylinder cylinder;
    Sphere sphere;
    std::vector<unsigned int> sareeTextures;
    unsigned int sareePriceTex;
    Pyramid pyramid;
    BezierPlate bezierPlate;

    void drawFoodItems(Shader& shader, glm::mat4 baseModel, float cz, unsigned int ceramicTex) {
        // 1. Beautiful Round Bezier Plates (5 plates)
        shader.use();
        shader.setBool("useTexture", true);
        shader.setInt("texture_diffuse", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ceramicTex);
        for (int p = -2; p <= 2; ++p) {
            float plateX = p * 1.1f; 
            float plateZ = cz + 0.45f;
            
            // Draw Bezier Plate
            glm::mat4 plate = glm::translate(baseModel, glm::vec3(plateX, 1.25f, plateZ));
            plate = glm::scale(plate, glm::vec3(0.35f, 0.4f, 0.35f)); 
            shader.setMat4("model", plate);
            shader.setVec3("objectColor", glm::vec3(1.0f)); 
            bezierPlate.draw();

            // Dense Pyramid on each plate
            shader.setBool("useTexture", false);
            float sweetScale = 0.16f; float spacing = 0.11f;
            glm::vec3 sweetColor = (abs(p) % 2 == 0) ? glm::vec3(1.0f, 0.55f, 0.0f) : glm::vec3(1.0f, 0.8f, 0.1f);
            shader.setVec3("objectColor", sweetColor);
            shader.setVec3("emission", sweetColor * 0.1f);
            for (int ly = 0; ly < 4; ++ly) {
                int count = 4 - ly;
                for (int i = 0; i < count; ++i) {
                    for (int j = 0; j < count; ++j) {
                        glm::mat4 sweet = glm::translate(baseModel, glm::vec3(plateX + (i-(count-1)*0.5f)*spacing, 1.35f + ly*0.1f, plateZ + (j-(count-1)*0.5f)*spacing));
                        sweet = glm::scale(sweet, glm::vec3(sweetScale));
                        shader.setMat4("model", sweet);
                        sphere.drawSphere(shader);
                    }
                }
            }
            shader.setBool("useTexture", true); 
            glBindTexture(GL_TEXTURE_2D, ceramicTex);
        }
        shader.setVec3("emission", glm::vec3(0.0f));
    }

    void drawToyItems(Shader& shader, glm::mat4 baseModel, float cz, unsigned int t1, unsigned int t2, unsigned int t3, unsigned int t4) {
        shader.use();
        glActiveTexture(GL_TEXTURE0);
        shader.setInt("texture_diffuse", 0);
        unsigned int texs[] = { t1, t2, t3, t4 };
        
        float tableTop = 1.25f;
        
        // ======================================================
        // 1. Densely Packed Gift Boxes (like reference image)
        // ======================================================
        glBindVertexArray(cubeVAO);
        
        glm::vec3 fc[] = {
            glm::vec3(1.0f,0.2f,0.2f), glm::vec3(0.2f,0.6f,1.0f),
            glm::vec3(1.0f,0.8f,0.0f), glm::vec3(0.8f,0.2f,0.9f),
            glm::vec3(0.0f,0.9f,0.4f), glm::vec3(1.0f,0.5f,0.0f),
            glm::vec3(0.3f,0.3f,0.9f), glm::vec3(0.9f,0.4f,0.6f)
        };
        
        // Helper: draw one box
        struct ToyBox { float x, y, z, sx, sy, sz; int type; }; // type: 0=cube, 1=pyramid
        
        // Row 1 (front): dense base layer - slightly lowered for contact
        ToyBox layer1[] = {
            {-2.2f, tableTop+0.13f, cz+0.5f, 0.50f, 0.30f, 0.45f, 0},
            {-1.5f, tableTop+0.13f, cz+0.5f, 0.55f, 0.30f, 0.50f, 0},
            {-0.8f, tableTop+0.13f, cz+0.5f, 0.45f, 0.30f, 0.40f, 0},
            {-0.1f, tableTop+0.13f, cz+0.5f, 0.50f, 0.30f, 0.50f, 0},
            { 0.6f, tableTop+0.13f, cz+0.5f, 0.48f, 0.30f, 0.42f, 0},
            { 1.3f, tableTop+0.13f, cz+0.5f, 0.55f, 0.30f, 0.45f, 0},
            { 2.0f, tableTop+0.13f, cz+0.5f, 0.50f, 0.30f, 0.50f, 0},
        };
        // Row 2 (back): slightly behind - slightly lowered
        ToyBox layer1b[] = {
            {-1.8f, tableTop+0.13f, cz+0.05f, 0.45f, 0.30f, 0.42f, 0},
            {-1.1f, tableTop+0.13f, cz+0.05f, 0.50f, 0.30f, 0.48f, 0},
            {-0.4f, tableTop+0.13f, cz+0.05f, 0.42f, 0.30f, 0.40f, 0},
            { 0.3f, tableTop+0.13f, cz+0.05f, 0.55f, 0.30f, 0.45f, 0},
            { 1.0f, tableTop+0.13f, cz+0.05f, 0.48f, 0.30f, 0.42f, 0},
            { 1.7f, tableTop+0.13f, cz+0.05f, 0.50f, 0.30f, 0.50f, 0},
        };
        // Layer 2: stacked on top of layer 1
        ToyBox layer2[] = {
            {-1.9f, tableTop+0.42f, cz+0.3f, 0.40f, 0.30f, 0.38f, 0},
            {-1.0f, tableTop+0.42f, cz+0.3f, 0.38f, 0.28f, 0.36f, 1},
            {-0.2f, tableTop+0.42f, cz+0.3f, 0.42f, 0.30f, 0.40f, 0},
            { 0.6f, tableTop+0.42f, cz+0.3f, 0.36f, 0.28f, 0.34f, 1},
            { 1.4f, tableTop+0.42f, cz+0.3f, 0.40f, 0.30f, 0.38f, 0},
        };
        // Layer 3: peak items
        ToyBox layer3[] = {
            {-1.5f, tableTop+0.72f, cz+0.3f, 0.32f, 0.24f, 0.30f, 1},
            {-0.3f, tableTop+0.72f, cz+0.3f, 0.30f, 0.22f, 0.28f, 0},
            { 0.9f, tableTop+0.72f, cz+0.3f, 0.34f, 0.24f, 0.32f, 1},
        };
        
        auto drawBox = [&](ToyBox& b, int idx) {
            unsigned int myTex = texs[idx % 4];
            if (myTex != 0) {
                shader.setBool("useTexture", true);
                glBindTexture(GL_TEXTURE_2D, myTex);
                shader.setVec3("objectColor", glm::vec3(1.0f));
            } else {
                shader.setBool("useTexture", false);
                shader.setVec3("objectColor", fc[idx % 8]);
            }
            
            glm::mat4 m = glm::translate(baseModel, glm::vec3(b.x, b.y, b.z));
            m = glm::scale(m, glm::vec3(b.sx, b.sy, b.sz));
            shader.setMat4("model", m);
            if (b.type == 0) {
                glBindVertexArray(cubeVAO);
                glDrawArrays(GL_TRIANGLES, 0, 36);
            } else {
                pyramid.draw();
            }
        };
        
        for (int i = 0; i < 7; ++i) drawBox(layer1[i], i);
        for (int i = 0; i < 6; ++i) drawBox(layer1b[i], i + 3);
        for (int i = 0; i < 5; ++i) drawBox(layer2[i], i + 1);
        for (int i = 0; i < 3; ++i) drawBox(layer3[i], i + 2);

        // ======================================================
        // 2. Balloon Bundle on Floor (inside stall, behind counter)
        // ======================================================
        shader.setBool("useTexture", false);
        
        float poleX = -1.5f;
        float poleZ = -0.5f;
        float poleHeight = 1.3f;  // Lowered to keep balloons below ceiling
        float knotY = poleHeight;
        
        // Wooden pole
        glm::mat4 poleMat = glm::translate(baseModel, glm::vec3(poleX, poleHeight * 0.5f, poleZ));
        poleMat = glm::scale(poleMat, glm::vec3(0.04f, poleHeight, 0.04f));
        shader.setMat4("model", poleMat);
        shader.setVec3("objectColor", glm::vec3(0.45f, 0.25f, 0.1f));
        cylinder.draw();
        
        // Balloon cluster - wide bouquet like reference image
        // Natural colors: red, green, blue, orange, yellow
        glm::vec3 bColors[] = {
            glm::vec3(0.9f, 0.1f, 0.1f),   // red
            glm::vec3(0.1f, 0.75f, 0.1f),   // green
            glm::vec3(0.15f, 0.45f, 0.95f), // blue
            glm::vec3(0.95f, 0.55f, 0.05f), // orange
            glm::vec3(0.95f, 0.85f, 0.1f),  // yellow
            glm::vec3(0.9f, 0.15f, 0.15f),  // red2
            glm::vec3(0.1f, 0.7f, 0.15f),   // green2
            glm::vec3(0.2f, 0.5f, 0.9f),    // blue2
        };
        
        float bW = 0.7f; // balloon width
        float bH = 1.0f; // balloon height (elongated)
        
        // Wide bouquet: balloons spread outward, converging at bottom
        // Front visible layer
        glm::vec3 bOff[] = {
            // Top center - tallest
            glm::vec3( 0.0f,  1.6f,  0.0f),
            // Upper ring - wide spread
            glm::vec3( 0.65f, 1.3f,  0.15f),
            glm::vec3(-0.65f, 1.3f,  0.15f),
            glm::vec3( 0.3f,  1.4f,  0.5f),
            glm::vec3(-0.3f,  1.4f, -0.5f),
            glm::vec3( 0.0f,  1.35f, -0.55f),
            // Middle ring - widest spread
            glm::vec3( 0.85f, 0.95f,  0.0f),
            glm::vec3(-0.85f, 0.95f,  0.0f),
            glm::vec3( 0.0f,  1.0f,   0.7f),
            glm::vec3( 0.0f,  0.95f, -0.7f),
            glm::vec3( 0.6f,  1.0f,   0.5f),
            glm::vec3(-0.6f,  1.0f,   0.5f),
            glm::vec3( 0.6f,  1.0f,  -0.5f),
            glm::vec3(-0.6f,  1.0f,  -0.5f),
            // Lower ring - taper inward  
            glm::vec3( 0.45f, 0.65f,  0.3f),
            glm::vec3(-0.45f, 0.65f, -0.3f),
            glm::vec3( 0.3f,  0.7f,  -0.4f),
            glm::vec3(-0.3f,  0.7f,   0.4f),
        };
        int nBal = 18;
        
        for (int i = 0; i < nBal; ++i) {
            glm::vec3 bPos(poleX + bOff[i].x, knotY + bOff[i].y, poleZ + bOff[i].z);
            glm::mat4 bMat = glm::translate(baseModel, bPos);
            bMat = glm::scale(bMat, glm::vec3(bW, bH, bW));
            shader.setMat4("model", bMat);
            glm::vec3 col = bColors[i % 8];
            shader.setVec3("objectColor", col);
            shader.setVec3("emission", col * 0.15f);
            sphere.drawSphere(shader);
        }
        shader.setVec3("emission", glm::vec3(0.0f));
    }


    void drawPotteryItems(Shader& shader, glm::mat4 baseModel, float cz) {
        shader.setBool("useTexture", false);
        float tableTop = 1.25f;

        // Pottery color palette (Terracotta shades)
        glm::vec3 cLight(0.72f, 0.45f, 0.28f); 
        glm::vec3 cMed(0.55f, 0.27f, 0.07f);   
        glm::vec3 cDark(0.38f, 0.18f, 0.08f);  

        // --- Helper: Hari/Kalshi (Traditional Pot) ---
        auto drawPot = [&](float x, float z, float scale, glm::vec3 col) {
            float bodyH = scale * 0.85f;
            // Body Sphere
            glm::mat4 m = glm::translate(baseModel, glm::vec3(x, tableTop + bodyH * 0.45f, z));
            m = glm::scale(m, glm::vec3(scale, bodyH, scale));
            shader.setMat4("model", m);
            shader.setVec3("objectColor", col);
            sphere.drawSphere(shader);
            
            // Neck - Deeply fused
            float neckR = scale * 0.16f; // Even slightly thinner
            float neckH = scale * 0.35f;
            glm::mat4 n = glm::translate(baseModel, glm::vec3(x, tableTop + bodyH * 0.65f, z)); // Deep overlap into sphere
            n = glm::scale(n, glm::vec3(neckR, neckH, neckR));
            shader.setMat4("model", n);
            cylinder.draw();
            
            // Solid Mouth/Rim (Using flattened sphere for solid cap)
            glm::mat4 r = glm::translate(baseModel, glm::vec3(x, tableTop + bodyH * 0.65f + neckH - 0.02f, z));
            r = glm::scale(r, glm::vec3(scale * 0.35f, 0.06f, scale * 0.35f));
            shader.setMat4("model", r);
            sphere.drawSphere(shader); // Using sphere ensures no "ring" look
        };

        // --- Helper: Surahi (Tall Pitcher) ---
        auto drawPitcher = [&](float x, float z, float scale, glm::vec3 col) {
            float baseH = scale * 0.7f;
            // Rounded Base
            glm::mat4 m = glm::translate(baseModel, glm::vec3(x, tableTop + baseH * 0.45f, z));
            m = glm::scale(m, glm::vec3(scale * 0.85f, baseH, scale * 0.85f));
            shader.setMat4("model", m);
            shader.setVec3("objectColor", col);
            sphere.drawSphere(shader);
            
            // Thinner Elegant Neck
            float neckH = scale * 1.0f;
            float neckR = scale * 0.11f; // Slimmer
            glm::mat4 n = glm::translate(baseModel, glm::vec3(x, tableTop + baseH * 0.55f, z)); // Fused base
            n = glm::scale(n, glm::vec3(neckR, neckH, neckR));
            shader.setMat4("model", n);
            cylinder.draw();
            
            // Solid Top Lip (Using sphere)
            glm::mat4 lip = glm::translate(baseModel, glm::vec3(x, tableTop + baseH * 0.55f + neckH - 0.02f, z));
            lip = glm::scale(lip, glm::vec3(neckR * 1.8f, 0.08f, neckR * 1.8f));
            shader.setMat4("model", lip);
            sphere.drawSphere(shader); // Guaranteed solid
        };

        // --- Helper: Sanki/Bowl Stack with depth (Gorto) ---
        auto drawBowlStack = [&](float x, float z, float scale, glm::vec3 col) {
            int stackSize = 6;
            float plateH = 0.035f;
            for(int i = 0; i < stackSize; ++i) {
                float yPos = tableTop + i * plateH + 0.05f;
                // Outer Plate Body
                glm::mat4 m = glm::translate(baseModel, glm::vec3(x, yPos, z));
                m = glm::scale(m, glm::vec3(scale, plateH * 2.5f, scale)); 
                shader.setMat4("model", m);
                shader.setVec3("objectColor", col);
                sphere.drawSphere(shader);
                
                // Inner "Pit" (Gorto)
                glm::mat4 gorto = glm::translate(baseModel, glm::vec3(x, yPos + 0.012f, z));
                gorto = glm::scale(gorto, glm::vec3(scale * 0.8f, plateH * 2.0f, scale * 0.8f));
                shader.setMat4("model", gorto);
                shader.setVec3("objectColor", col * 0.8f); // Slightly darker
                sphere.drawSphere(shader);
            }
        };

        // --- Arrangement ---
        drawPitcher(-2.2f, cz + 0.1f, 0.85f, cMed);
        drawPot(-1.1f, cz + 0.1f, 0.95f, cDark);
        drawPitcher(0.1f, cz + 0.1f, 1.0f, cLight);
        drawPot(1.3f, cz + 0.1f, 0.9f, cMed);
        drawPitcher(2.3f, cz + 0.1f, 0.85f, cDark);

        drawPot(-1.8f, cz + 0.45f, 0.7f, cLight);
        drawPot(-0.7f, cz + 0.45f, 0.75f, cMed);
        drawPot(0.5f, cz + 0.45f, 0.7f, cDark);
        drawPot(1.6f, cz + 0.45f, 0.75f, cLight);

        drawBowlStack(-2.1f, cz + 0.8f, 0.55f, cDark);
        drawBowlStack(-1.2f, cz + 0.8f, 0.5f, cLight);
        drawBowlStack(-0.3f, cz + 0.8f, 0.6f, cMed);
        drawBowlStack(0.6f, cz + 0.8f, 0.55f, cDark);
        drawBowlStack(1.5f, cz + 0.8f, 0.5f, cLight);
        drawBowlStack(2.4f, cz + 0.8f, 0.6f, cMed);
    }
    void drawTeaItems(Shader& shader, glm::mat4 baseModel, float cz, unsigned int t1, unsigned int t2, unsigned int t3) {
        float tableTop = 1.25f;

        // 1. Menu Boards
        shader.setBool("useTexture", true);
        shader.setVec3("objectColor", glm::vec3(1.0f, 1.0f, 1.0f));
        glActiveTexture(GL_TEXTURE0);

        // Board 1: Tall board on the right - User wants cha3 here
        if (t3 != 0) {
            glBindTexture(GL_TEXTURE_2D, t3);
            glm::mat4 b1 = glm::translate(baseModel, glm::vec3(2.5f, 1.45f, cz + 1.2f));
            b1 = glm::rotate(b1, glm::radians(-15.0f), glm::vec3(0, 1, 0));
            b1 = glm::scale(b1, glm::vec3(1.4f, 2.2f, 0.05f)); // Adjusted proportion for Bengali text
            shader.setMat4("model", b1);
            shader.setFloat("specularStrength", 0.0f);
            glBindVertexArray(cubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // Board 2: Ground board leaning in middle - Use cha2
        if (t2 != 0) {
            glBindTexture(GL_TEXTURE_2D, t2);
            glm::mat4 b2 = glm::translate(baseModel, glm::vec3(-0.1f, 0.72f, cz + 1.25f));
            b2 = glm::rotate(b2, glm::radians(-12.0f), glm::vec3(1, 0, 0));
            b2 = glm::scale(b2, glm::vec3(1.45f, 1.35f, 0.05f)); // Avoid squashing text
            shader.setMat4("model", b2);
            shader.setFloat("specularStrength", 0.0f);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // Board 3: Board ON TOP OF THE TABLE - Use cha1
        if (t1 != 0) {
            glBindTexture(GL_TEXTURE_2D, t1);
            glm::mat4 b3 = glm::translate(baseModel, glm::vec3(-2.1f, tableTop + 0.6f, cz + 0.5f));
            b3 = glm::rotate(b3, glm::radians(10.0f), glm::vec3(0, 1, 0)); // Slight angle on table
            b3 = glm::scale(b3, glm::vec3(1.1f, 1.2f, 0.05f));
            shader.setMat4("model", b3);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        shader.setVec3("emission", glm::vec3(0.0f));

        // 2. Kettles (Ketli) - Final Solid Reconstruction
        shader.setBool("useTexture", false);
        auto drawKettle = [&](float x, float z) {
            glm::vec3 gunmetal(0.30f, 0.30f, 0.32f); 
            float bodyYScale = 0.45f;
            float bodyYRad = bodyYScale * 0.5f;
            
            // 2.1 Squat Body (Wide and Low)
            glm::mat4 body = glm::translate(baseModel, glm::vec3(x, tableTop + bodyYRad, z));
            body = glm::scale(body, glm::vec3(0.75f, bodyYScale, 0.75f));
            shader.setMat4("model", body);
            shader.setVec3("objectColor", gunmetal);
            sphere.drawSphere(shader);
            
            // 2.2 Solid Rim/Boltu (Using Spheres to ensure it's SOLID - no ring gaps)
            float rimYBase = tableTop + bodyYScale - 0.1f;
            // Neck filler (fills gap between rim and body top)
            glm::mat4 neck = glm::translate(baseModel, glm::vec3(x, rimYBase, z));
            neck = glm::scale(neck, glm::vec3(0.35f, 0.12f, 0.35f));
            shader.setMat4("model", neck);
            sphere.drawSphere(shader);
            
            // Visible Rim
            glm::mat4 rim = glm::translate(baseModel, glm::vec3(x, rimYBase + 0.08f, z));
            rim = glm::scale(rim, glm::vec3(0.32f, 0.14f, 0.32f));
            shader.setMat4("model", rim);
            sphere.drawSphere(shader);
            
            // Small solid knob on top
            glm::mat4 knob = glm::translate(baseModel, glm::vec3(x, rimYBase + 0.16f, z));
            knob = glm::scale(knob, glm::vec3(0.08f, 0.1f, 0.08f));
            shader.setMat4("model", knob);
            sphere.drawSphere(shader);

            // 2.3 Larger & Solid Spout
            glm::mat4 spout = glm::translate(baseModel, glm::vec3(x + 0.26f, tableTop + 0.15f, z)); 
            spout = glm::rotate(spout, glm::radians(75.0f), glm::vec3(0, 0, 1));
            spout = glm::scale(spout, glm::vec3(0.15f, 0.35f, 0.15f)); 
            shader.setMat4("model", spout);
            cylinder.draw();

            // 2.4 Arched Overhead Handle (5-SEGMENT POLYGONAL ARCH)
            float hW = 0.58f; // Narrowed to ensure it's deep inside
            float hH = 0.50f;
            float hY = tableTop + 0.20f;
            float sideX = hW * 0.5f;
            float diagW = 0.15f; // for the bend

            // Segment 1 & 5: Vertical Sides
            auto drawHBar = [&](glm::vec3 pos, glm::vec3 rotAxis, float rotAng, glm::vec3 vScale) {
                glm::mat4 h = glm::translate(baseModel, pos);
                if (rotAng != 0) h = glm::rotate(h, glm::radians(rotAng), rotAxis);
                h = glm::scale(h, vScale);
                shader.setMat4("model", h);
                cylinder.draw();
            };

            // Sides
            drawHBar(glm::vec3(x - sideX, hY, z), glm::vec3(0,0,1), 0,    glm::vec3(0.04f, 0.35f, 0.04f));
            drawHBar(glm::vec3(x + sideX, hY, z), glm::vec3(0,0,1), 0,    glm::vec3(0.04f, 0.35f, 0.04f));
            // Shoulders (Angle)
            drawHBar(glm::vec3(x - sideX, hY+0.35f, z), glm::vec3(0,0,1), -45,  glm::vec3(0.04f, 0.22f, 0.04f));
            drawHBar(glm::vec3(x + sideX, hY+0.35f, z), glm::vec3(0,0,1), 45,   glm::vec3(0.04f, 0.22f, 0.04f));
            // Top Bar (Centered)
            float topBarW = hW - diagW*2.0f;
            drawHBar(glm::vec3(x - sideX + 0.15f, hY+0.50f, z), glm::vec3(0,0,1), -90, glm::vec3(0.04f, topBarW+0.05f, 0.04f));
        };

        drawKettle(-0.7f, cz + 0.35f);
        drawKettle( 0.45f, cz + 0.35f);

        // 3. Matir Bhar
        shader.setVec3("objectColor", glm::vec3(0.55f, 0.32f, 0.15f)); 
        for (int i = 0; i < 4; ++i) {
            for (int k = 0; k < 2; ++k) {
                float cupX = 1.35f + i*0.42f;
                float cupZ = cz + 0.25f + k*0.38f;
                glm::mat4 cBase = glm::translate(baseModel, glm::vec3(cupX, tableTop + 0.06f, cupZ));
                cBase = glm::scale(cBase, glm::vec3(0.18f, 0.12f, 0.18f));
                shader.setMat4("model", cBase);
                sphere.drawSphere(shader);
                glm::mat4 cNeck = glm::translate(baseModel, glm::vec3(cupX, tableTop + 0.12f, cupZ));
                cNeck = glm::scale(cNeck, glm::vec3(0.16f, 0.18f, 0.16f));
                shader.setMat4("model", cNeck);
                cylinder.draw();
            }
        }
    }

    // ========== SAREE STALL (stallType == 4) ==========
    void drawSareeItems(Shader& shader, glm::mat4 baseModel, float cz) {
        if (sareeTextures.empty()) return; // Safety: no textures loaded
        float tableTop = 1.25f;

        // 1. Hanging Rod (bamboo-colored horizontal bar along the back wall)
        shader.setBool("useTexture", false);
        shader.setVec3("objectColor", glm::vec3(0.45f, 0.35f, 0.2f));
        glm::mat4 rod = glm::translate(baseModel, glm::vec3(-2.6f, 3.45f, -2.05f)); // Start from the left
        rod = glm::rotate(rod, glm::radians(-90.0f), glm::vec3(0, 0, 1)); // Extend to the right
        rod = glm::scale(rod, glm::vec3(0.05f, 5.2f, 0.05f));
        shader.setMat4("model", rod);
        cylinder.draw();

        // 2. Hanging Sarees on back wall (textured quads)
        shader.setBool("useTexture", true);
        shader.setVec3("objectColor", glm::vec3(1.0f, 1.0f, 1.0f));
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(cubeVAO);

        int numHanging = (sareeTextures.size() >= 6) ? 6 : (int)sareeTextures.size();
        float hangStartX = -2.4f;
        float hangSpacing = 4.8f / (float)(numHanging > 1 ? numHanging - 1 : 1);

        for (int i = 0; i < numHanging && i < (int)sareeTextures.size(); ++i) {
            unsigned int texId = sareeTextures[i];
            if (texId == 0) continue; // Skip failed textures
            glBindTexture(GL_TEXTURE_2D, texId);
            float xPos = hangStartX + i * hangSpacing;
            // Saree patterns look much better with proper width and zero specular glare
            glm::mat4 s = glm::translate(baseModel, glm::vec3(xPos, 2.2f, -2.0f));
            s = glm::scale(s, glm::vec3(1.35f, 2.3f, 0.04f)); 
            shader.setMat4("model", s);
            shader.setFloat("specularStrength", 0.02f); 
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        shader.setVec3("emission", glm::vec3(0.0f));

        // 3. Price Board (shari_price.png) - hanging from the table edge
        if (sareePriceTex != 0) {
            shader.setVec3("objectColor", glm::vec3(1.0f, 1.0f, 1.0f));
            glBindTexture(GL_TEXTURE_2D, sareePriceTex);
            // Raised slightly and adjusted scale to maintain text clarity (less horizontal stretch)
            glm::mat4 board = glm::translate(baseModel, glm::vec3(0.0f, 0.8f, 2.13f));
            board = glm::scale(board, glm::vec3(2.2f, 1.2f, 0.05f)); 
            shader.setMat4("model", board);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // 4. Folded Saree Stacks on the counter (Solid vibrant colors)
        shader.setBool("useTexture", false);
        // Define rich silk-like colors for the folded stacks (reduced brightness to prevent blowout)
        glm::vec3 sareeColors[] = {
            glm::vec3(0.65f, 0.08f, 0.25f), // Magenta darker
            glm::vec3(0.08f, 0.40f, 0.40f), // Teal darker
            glm::vec3(0.70f, 0.50f, 0.10f), // Saffron/Gold darker
            glm::vec3(0.15f, 0.25f, 0.55f), // Royal Blue darker
            glm::vec3(0.45f, 0.10f, 0.45f), // Purple darker
            glm::vec3(0.10f, 0.45f, 0.22f), // Emerald darker
            glm::vec3(0.65f, 0.18f, 0.08f), // Vermillion darker
            glm::vec3(0.55f, 0.40f, 0.15f), // Bronze darker
        };
        int numColors = 8;

        // Row 1: Front row stacks on the counter
        float stackStartX = -2.3f;
        for (int i = 0; i < 7; ++i) {
            float sx = stackStartX + i * 0.75f;
            // Each stack is 3-4 thin slabs piled up
            for (int layer = 0; layer < 4; ++layer) {
                glm::vec3 col = sareeColors[(i * 3 + layer) % numColors];
                glm::mat4 slab = glm::translate(baseModel, glm::vec3(sx, tableTop + 0.06f + layer * 0.1f, cz + 0.15f - 0.275f));
                slab = glm::rotate(slab, glm::radians(90.0f), glm::vec3(1, 0, 0));
                slab = glm::scale(slab, glm::vec3(0.325f, 0.55f, 0.04f));
                shader.setMat4("model", slab);
                shader.setVec3("objectColor", col);
                cylinder.draw(); // Rotated cylinder creates curved folded front edges
            }
        }

        // Row 2: Back row stacks (taller, behind front row)
        for (int i = 0; i < 6; ++i) {
            float sx = stackStartX + 0.35f + i * 0.82f;
            for (int layer = 0; layer < 5; ++layer) {
                glm::vec3 col = sareeColors[(i * 2 + layer + 3) % numColors];
                glm::mat4 slab = glm::translate(baseModel, glm::vec3(sx, tableTop + 0.06f + layer * 0.1f, cz + 0.7f - 0.25f));
                slab = glm::rotate(slab, glm::radians(90.0f), glm::vec3(1, 0, 0));
                slab = glm::scale(slab, glm::vec3(0.35f, 0.5f, 0.04f));
                shader.setMat4("model", slab);
                shader.setVec3("objectColor", col);
                cylinder.draw();
            }
        }
    }

    void drawPillar(Shader& shader, glm::mat4 base, glm::vec3 offset, float height) {
        glm::mat4 m = glm::translate(base, offset);
        m = glm::scale(m, glm::vec3(0.12f, height, 0.12f));
        shader.setMat4("model", m);
        cylinder.draw();
    }
};

#endif
