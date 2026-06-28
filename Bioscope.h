#ifndef BIOSCOPE_H
#define BIOSCOPE_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "shader.h"
#include "cylinder.h"
#include "pyramid.h"
#include "sphere.h"
#include <vector>
#include <cmath>

struct BSplineHorn {
    unsigned int VAO, VBO, EBO;
    int indexCount;

    struct BSVertex {
        glm::vec3 Pos;
        glm::vec3 Normal;
    };

    float CoxDeBoor(int i, int p, float t, const std::vector<float>& knots) {
        if (p == 0) {
            if (knots[i] <= t && t < knots[i + 1]) return 1.0f;
            return 0.0f;
        }

        float denom1 = knots[i + p] - knots[i];
        float term1 = 0.0f;
        if (denom1 > 0.0f) {
            term1 = ((t - knots[i]) / denom1) * CoxDeBoor(i, p - 1, t, knots);
        }

        float denom2 = knots[i + p + 1] - knots[i + 1];
        float term2 = 0.0f;
        if (denom2 > 0.0f) {
            term2 = ((knots[i + p + 1] - t) / denom2) * CoxDeBoor(i + 1, p - 1, t, knots);
        }

        return term1 + term2;
    }

    BSplineHorn() {
        std::vector<glm::vec2> controlPoints = {
            glm::vec2(0.08f, 0.0f),
            glm::vec2(0.08f, 0.4f),
            glm::vec2(0.2f, 0.8f),
            glm::vec2(0.5f, 1.1f),
            glm::vec2(1.2f, 1.25f),
            glm::vec2(1.4f, 1.3f)
        };
        int n = controlPoints.size() - 1; // 5
        int p = 3; // cubic

        std::vector<float> knots;
        for (int i = 0; i <= p; i++) knots.push_back(0.0f);
        for (int i = 1; i <= n - p; i++) knots.push_back((float)i / (n - p + 1));
        for (int i = 0; i <= p; i++) knots.push_back(1.0f);

        int resolution = 30; 
        std::vector<glm::vec2> profileCurve;
        for (int i = 0; i <= resolution; i++) {
            float t = (float)i / resolution;
            if (t == 1.0f) t = 0.9999f; 

            glm::vec2 pt(0.0f, 0.0f);
            for (int j = 0; j <= n; j++) {
                float basis = CoxDeBoor(j, p, t, knots);
                pt += basis * controlPoints[j];
            }
            profileCurve.push_back(pt);
        }

        int numSlices = 40;
        std::vector<BSVertex> vertices;
        
        // 1. Outer Shell
        for (int i = 0; i <= resolution; i++) {
            float r = profileCurve[i].x;
            float y = profileCurve[i].y;
            for (int j = 0; j <= numSlices; j++) {
                float theta = j * (2.0f * 3.14159265f / numSlices);
                BSVertex v;
                v.Pos = glm::vec3(r * cos(theta), y, r * sin(theta));
                v.Normal = glm::vec3(0.0f);
                vertices.push_back(v);
            }
        }
        
        // 2. Inner Shell (slightly smaller radius)
        int shellVertices = vertices.size();
        for (int i = 0; i <= resolution; i++) {
            float r = profileCurve[i].x - 0.02f; // Provide thickness
            if (r < 0.01f) r = 0.01f;
            float y = profileCurve[i].y;
            for (int j = 0; j <= numSlices; j++) {
                float theta = j * (2.0f * 3.14159265f / numSlices);
                BSVertex v;
                v.Pos = glm::vec3(r * cos(theta), y, r * sin(theta));
                v.Normal = glm::vec3(0.0f);
                vertices.push_back(v);
            }
        }

        std::vector<unsigned int> indices;
        
        // Helper to build triangulated faces and compute per-face normals
        auto buildFaces = [&](int offset, bool invertNormal) {
            for (int i = 0; i < resolution; i++) {
                for (int j = 0; j < numSlices; j++) {
                    int v0 = offset + i * (numSlices + 1) + j;
                    int v1 = offset + (i + 1) * (numSlices + 1) + j;
                    int v2 = offset + (i + 1) * (numSlices + 1) + (j + 1);
                    int v3 = offset + i * (numSlices + 1) + (j + 1);

                    if (invertNormal) {
                        // Invert winding order so faces are CCW from the inside
                        indices.push_back(v0); indices.push_back(v2); indices.push_back(v1);
                        indices.push_back(v0); indices.push_back(v3); indices.push_back(v2);
                    } else {
                        indices.push_back(v0); indices.push_back(v1); indices.push_back(v2);
                        indices.push_back(v0); indices.push_back(v2); indices.push_back(v3);
                    }

                    glm::vec3 e1 = vertices[v1].Pos - vertices[v0].Pos;
                    glm::vec3 e2 = vertices[v2].Pos - vertices[v0].Pos;
                    glm::vec3 n1 = glm::normalize(glm::cross(e1, e2));
                    if (invertNormal) n1 = -n1;
                    
                    vertices[v0].Normal += n1;
                    vertices[v1].Normal += n1;
                    vertices[v2].Normal += n1;

                    glm::vec3 e3 = vertices[v2].Pos - vertices[v0].Pos;
                    glm::vec3 e4 = vertices[v3].Pos - vertices[v0].Pos;
                    glm::vec3 n2 = glm::normalize(glm::cross(e3, e4));
                    if (invertNormal) n2 = -n2;
                    
                    vertices[v0].Normal += n2;
                    vertices[v2].Normal += n2;
                    vertices[v3].Normal += n2;
                }
            }
        };

        buildFaces(0, false); // Generate Outer Shell (Normals Outward)
        buildFaces(shellVertices, true); // Generate Inner Shell (Normals Inward)
        
        // 3. Connect the Front Rim (connects top row of outer and inner shell)
        int lastRowOuter = resolution * (numSlices + 1);
        int lastRowInner = shellVertices + resolution * (numSlices + 1);
        for (int j = 0; j < numSlices; j++) {
            int v0 = lastRowOuter + j;
            int v1 = lastRowOuter + (j + 1);
            int v2 = lastRowInner + (j + 1);
            int v3 = lastRowInner + j;
            
            indices.push_back(v0); indices.push_back(v1); indices.push_back(v2);
            indices.push_back(v0); indices.push_back(v2); indices.push_back(v3);
            
            glm::vec3 e1 = vertices[v1].Pos - vertices[v0].Pos;
            glm::vec3 e2 = vertices[v2].Pos - vertices[v0].Pos;
            glm::vec3 n = glm::normalize(glm::cross(e1, e2));
            vertices[v0].Normal += n; vertices[v1].Normal += n; vertices[v2].Normal += n;
            
            glm::vec3 e3 = vertices[v2].Pos - vertices[v0].Pos;
            glm::vec3 e4 = vertices[v3].Pos - vertices[v0].Pos;
            glm::vec3 n2 = glm::normalize(glm::cross(e3, e4));
            vertices[v0].Normal += n2; vertices[v2].Normal += n2; vertices[v3].Normal += n2;
        }

        for (size_t i = 0; i < vertices.size(); i++) {
            vertices[i].Normal = glm::normalize(vertices[i].Normal);
        }

        indexCount = indices.size();

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(BSVertex), &vertices[0], GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BSVertex), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(BSVertex), (void*)reinterpret_cast<void*>(offsetof(BSVertex, Normal)));

        glBindVertexArray(0);
    }

    void draw(Shader& shader, glm::mat4 model) {
        shader.setMat4("model", model);
        
        // Since we explicitly constructed a two-sided thick wall, back-face culling stays as-is natively!
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
};

class Bioscope {
private:
    unsigned int cubeVAO;
    Cylinder cylinder;
    Sphere sphere;
    Pyramid pyramid;
    BSplineHorn bHorn;
public:
    Bioscope(unsigned int cubeVAO) : cubeVAO(cubeVAO), cylinder(36), pyramid(), sphere(1.0f, 24, 12), bHorn() {}

    void draw(Shader& shader, glm::vec3 pos, unsigned int woodTex, unsigned int nokshaTex, unsigned int whiteFabricTex, bool isPlaying, unsigned int currentFrameTex) {
        glm::mat4 baseModel = glm::mat4(1.0f);
        baseModel = glm::translate(baseModel, pos);

        if (!isPlaying) {
            drawTable(shader, baseModel, woodTex, whiteFabricTex);
            drawBioscopeBox(shader, baseModel, nokshaTex);
            drawProps(shader, baseModel);
        }
        
        drawLenses(shader, baseModel, isPlaying);
        drawGramophone(shader, baseModel);
        drawScreen(shader, baseModel, isPlaying, currentFrameTex);
    }

private:

    void drawTable(Shader& shader, glm::mat4 base, unsigned int woodTex, unsigned int whiteFabricTex) {
        // Table legs
        shader.setBool("useTexture", true);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTex);
        shader.setVec3("objectColor", glm::vec3(1.0f));

        glBindVertexArray(cubeVAO);
        float legX[] = { -2.8f, 2.8f, -2.8f, 2.8f };
        float legZ[] = { -1.8f, -1.8f, 1.8f, 1.8f };
        for (int i = 0; i < 4; i++) {
            glm::mat4 m = glm::translate(base, glm::vec3(legX[i], 0.4f, legZ[i]));
            m = glm::scale(m, glm::vec3(0.2f, 0.8f, 0.2f));
            shader.setMat4("model", m);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // Tablecloth (textured cube over the whole table)
        shader.setBool("useTexture", true);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, whiteFabricTex);
        shader.setVec3("objectColor", glm::vec3(1.0f)); 
        glm::mat4 top = glm::translate(base, glm::vec3(0.0f, 0.83f, 0.0f));
        top = glm::scale(top, glm::vec3(6.4f, 0.06f, 4.4f));
        shader.setMat4("model", top);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    void drawBioscopeBox(Shader& shader, glm::mat4 base, unsigned int decoTex) {
        glBindVertexArray(cubeVAO);

        // Main colored block (red and yellow mix)
        // Red outline/frame
        shader.setBool("useTexture", false);
        shader.setVec3("objectColor", glm::vec3(0.88f, 0.15f, 0.15f)); // Bright red base
        glm::mat4 mainBox = glm::translate(base, glm::vec3(0.0f, 1.86f, 0.0f));
        mainBox = glm::scale(mainBox, glm::vec3(3.8f, 2.0f, 2.0f));
        shader.setMat4("model", mainBox);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Yellow side trim overlay
        shader.setVec3("objectColor", glm::vec3(1.0f, 0.85f, 0.1f));
        glm::mat4 yellowTrim = glm::translate(base, glm::vec3(0.0f, 1.86f, 0.0f));
        yellowTrim = glm::scale(yellowTrim, glm::vec3(3.82f, 1.8f, 1.8f));
        shader.setMat4("model", yellowTrim);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Decorated front panel (with noksha flowers)
        shader.setBool("useTexture", true);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, decoTex);
        shader.setVec3("objectColor", glm::vec3(1.0f));
        glm::mat4 fp = glm::translate(base, glm::vec3(0.0f, 1.86f, 1.01f)); // Just slightly out in Z
        fp = glm::scale(fp, glm::vec3(3.6f, 1.8f, 0.02f));
        shader.setMat4("model", fp);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        // Yellow top angle/roof
        shader.setBool("useTexture", false);
        shader.setVec3("objectColor", glm::vec3(1.0f, 0.85f, 0.1f));
        glm::mat4 roof = glm::translate(base, glm::vec3(0.0f, 2.9f, 0.0f));
        roof = glm::scale(roof, glm::vec3(3.8f, 0.2f, 1.2f));
        shader.setMat4("model", roof);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        // Red top trim
        shader.setVec3("objectColor", glm::vec3(0.88f, 0.15f, 0.15f));
        glm::mat4 rt = glm::translate(base, glm::vec3(0.0f, 3.0f, 0.0f));
        rt = glm::scale(rt, glm::vec3(3.5f, 0.1f, 1.0f));
        shader.setMat4("model", rt);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    void drawLenses(Shader& shader, glm::mat4 base, bool isPlaying) {
        if (isPlaying) return; // Hide lenses to avoid clipping camera
        shader.setBool("useTexture", false);
        
        // 3 front viewing lenses
        float lensX[] = { -1.1f, 0.0f, 1.1f };
        for (int i = 0; i < 3; i++) {
            // First draw thick silver circular base (Solid flat sphere)
            glm::mat4 bg = glm::translate(base, glm::vec3(lensX[i], 1.45f, 1.05f));
            bg = glm::scale(bg, glm::vec3(0.35f, 0.35f, 0.08f)); // Flat on Z
            shader.setVec3("objectColor", glm::vec3(0.7f, 0.75f, 0.75f)); // Silver housing
            shader.setMat4("model", bg);
            sphere.drawSphere(shader);

            // Then draw black solid glass on top (slightly smaller, placed forward)
            glm::mat4 m = glm::translate(base, glm::vec3(lensX[i], 1.45f, 1.13f)); 
            m = glm::scale(m, glm::vec3(0.28f, 0.28f, 0.03f)); 
            shader.setVec3("objectColor", glm::vec3(0.05f, 0.05f, 0.1f)); // Black glass
            shader.setMat4("model", m);
            sphere.drawSphere(shader);
        }

        // Side film winding knobs via solid flat spheres
        float sideX[] = { -1.91f, 1.91f };
        for (int i = 0; i < 2; i++) {
            // Silver base disk
            glm::mat4 mb = glm::translate(base, glm::vec3(sideX[i], 1.3f, 0.0f));
            mb = glm::scale(mb, glm::vec3(0.04f, 0.4f, 0.4f)); // flat on X
            shader.setVec3("objectColor", glm::vec3(0.85f, 0.85f, 0.85f));
            shader.setMat4("model", mb);
            sphere.drawSphere(shader);

            // Red center inner disk
            glm::mat4 mc = glm::translate(base, glm::vec3(sideX[i] + (i==0?-0.04f:0.04f), 1.3f, 0.0f));
            mc = glm::scale(mc, glm::vec3(0.02f, 0.25f, 0.25f));
            shader.setVec3("objectColor", glm::vec3(0.88f, 0.15f, 0.15f));
            shader.setMat4("model", mc);
            sphere.drawSphere(shader);
        }
    }

    void drawGramophone(Shader& shader, glm::mat4 base) {
        shader.setBool("useTexture", false);
        glBindVertexArray(cubeVAO);
        
        // Black music box player
        shader.setVec3("objectColor", glm::vec3(0.1f, 0.1f, 0.1f));
        glm::mat4 bbox = glm::translate(base, glm::vec3(0.0f, 3.2f, 0.0f));
        bbox = glm::scale(bbox, glm::vec3(1.2f, 0.4f, 0.8f));
        shader.setMat4("model", bbox);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Buttons on black box
        shader.setVec3("objectColor", glm::vec3(0.8f, 0.1f, 0.1f));
        glm::mat4 btn1 = glm::translate(base, glm::vec3(-0.4f, 3.2f, 0.41f));
        btn1 = glm::scale(btn1, glm::vec3(0.1f, 0.1f, 0.02f));
        shader.setMat4("model", btn1);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Pipe leading to horn (adjusting for proper connection)
        shader.setVec3("objectColor", glm::vec3(0.4f, 0.4f, 0.4f));
        glm::mat4 pipe1 = glm::translate(base, glm::vec3(0.0f, 3.6f, -0.2f));
        pipe1 = glm::scale(pipe1, glm::vec3(0.06f, 0.4f, 0.06f));
        shader.setMat4("model", pipe1);
        cylinder.draw();
        
        glm::mat4 pipe2 = glm::translate(base, glm::vec3(0.0f, 4.0f, -0.2f));
        pipe2 = glm::rotate(pipe2, glm::radians(60.0f), glm::vec3(1, 0, 0));
        pipe2 = glm::scale(pipe2, glm::vec3(0.08f, 0.5f, 0.08f));
        shader.setMat4("model", pipe2);
        cylinder.draw();

        // Giant Golden Horn (B-Spline Native Surface of Revolution)
        shader.setVec3("objectColor", glm::vec3(0.95f, 0.8f, 0.15f)); // Glossy Gold
        
        // Base coordinate pointing mostly forward (-Z rotation aligns Y axis forward)
        glm::mat4 hornCenter = glm::translate(base, glm::vec3(0.0f, 4.3f, 0.23f));
        hornCenter = glm::rotate(hornCenter, glm::radians(75.0f), glm::vec3(1, 0, 0)); 

        // 1. Render the Mathematically Flawless Curved Shell using custom VAO logic
        bHorn.draw(shader, hornCenter);
        
        // 2. Deep Hollow Throat (Black disc recessed safely inside to fake endless dark depth)
        glm::mat4 voidGlass = glm::translate(hornCenter, glm::vec3(0.0f, 0.6f, 0.0f)); 
        voidGlass = glm::scale(voidGlass, glm::vec3(0.35f, 0.05f, 0.35f)); 
        shader.setVec3("objectColor", glm::vec3(0.1f, 0.05f, 0.0f)); 
        shader.setMat4("model", voidGlass);
        sphere.drawSphere(shader);

        // 3. Central Acoustics Cone natively resting in the throat
        glm::mat4 nip = glm::translate(hornCenter, glm::vec3(0.0f, 0.61f, 0.0f));
        nip = glm::scale(nip, glm::vec3(0.2f, 0.4f, 0.2f));
        shader.setVec3("objectColor", glm::vec3(0.95f, 0.8f, 0.1f)); 
        shader.setMat4("model", nip);
        pyramid.draw();
    }

    void drawProps(Shader& shader, glm::mat4 base) {
        // Scattered solid film reel discs on table beside bioscope
        shader.setBool("useTexture", false);
        
        float rx[] = { 2.4f, 2.7f, 2.5f, -2.6f, -2.4f };
        float rz[] = { 0.4f, 0.8f, -0.2f, 0.9f, 0.6f };
        for (int i = 0; i < 5; i++) {
            glm::mat4 m = glm::translate(base, glm::vec3(rx[i], 0.86f + (i==2?0.04f:0.0f), rz[i]));
            m = glm::scale(m, glm::vec3(0.35f, 0.03f, 0.35f));
            
            // Alternate film reel colors (silver and red)
            shader.setVec3("objectColor", (i % 2 == 0) ? glm::vec3(0.6f, 0.6f, 0.65f) : glm::vec3(0.75f, 0.15f, 0.15f)); 
            shader.setMat4("model", m);
            sphere.drawSphere(shader);
        }
    }

    void drawScreen(Shader& shader, glm::mat4 base, bool isPlaying, unsigned int currentFrameTex) {
        if (!isPlaying) return;

        // Internal Screen Quad/Box
        shader.setBool("useTexture", true);
        if (currentFrameTex == 0) shader.setBool("useTexture", false); // Fallback to color if tex missing
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, currentFrameTex);
        shader.setVec3("objectColor", glm::vec3(1.0f)); 

        glBindVertexArray(cubeVAO);
        
        // Massive Cinematic Screen
        glm::mat4 screenM = glm::translate(base, glm::vec3(0.0f, 1.45f, -1.5f)); 
        screenM = glm::scale(screenM, glm::vec3(12.0f, 7.0f, 0.01f)); 
        shader.setMat4("model", screenM);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Backdrop
        shader.setBool("useTexture", false);
        shader.setVec3("objectColor", glm::vec3(0.0f, 0.0f, 0.0f)); // Pure black backdrop
        glm::mat4 glowM = glm::translate(base, glm::vec3(0.0f, 1.45f, -1.6f));
        glowM = glm::scale(glowM, glm::vec3(20.0f, 20.0f, 0.005f));
        shader.setMat4("model", glowM);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
};

#endif
