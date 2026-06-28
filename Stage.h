#ifndef STAGE_H
#define STAGE_H

#include <glad/glad.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "shader.h"
#include "cylinder.h"
#include "Sphere.h"
#include "sphereWithTexture.h"

class Stage {
public:
    Stage(unsigned int cubeVAO) 
        : cubeVAO(cubeVAO), cylinder(16), 
          sphere(0.5f, 18, 9),
          potSphere(0.5f, 20, 10, {1,1,1}, {1,1,1}, {1,1,1}, 32.0f, 0, 0, 0,0, 1,1)
    {}

    void draw(Shader& shader, glm::vec3 position, unsigned int stageTex, unsigned int flowerTex, unsigned int woodTex, const std::vector<unsigned int>& fabricTexs, unsigned int nokshaTex, float time = 0.0f) {
        shader.use();
        glm::mat4 base = glm::mat4(1.0f);
        base = glm::translate(base, position);
        
        float W = 12.0f;       // Stage width
        float D = 6.0f;        // Stage depth
        float H = 1.2f;        // Platform height
        float PH = 5.0f;       // Pole height (taller backdrop)
        float backZ = -D/2.0f + 0.2f;
        float frontZ = D/2.0f;
        
        // ============================================================
        // 1. STAGE PLATFORM
        // ============================================================
        shader.setBool("useTexture", true);
        shader.setInt("texture_diffuse", 0);
        glActiveTexture(GL_TEXTURE0);
        if (woodTex != 0) glBindTexture(GL_TEXTURE_2D, woodTex);
        glBindVertexArray(cubeVAO);
        
        shader.setVec3("objectColor", glm::vec3(0.55f, 0.35f, 0.18f));
        
        // Main platform
        drawCube(shader, base, glm::vec3(0, H/2, 0), glm::vec3(W, H, D));
        
        // Front step (full width, lower)
        drawCube(shader, base, glm::vec3(0, H*0.25f, frontZ+0.5f), glm::vec3(W+1.0f, H*0.5f, 1.0f));
        
        // Side stairs
        drawCube(shader, base, glm::vec3(-(W/2+0.6f), H*0.25f, 0), glm::vec3(1.2f, H*0.5f, 2.5f));
        drawCube(shader, base, glm::vec3( (W/2+0.6f), H*0.25f, 0), glm::vec3(1.2f, H*0.5f, 2.5f));
        
        // Front decorative panel/skirt (darker wood strip under the front edge)
        shader.setVec3("objectColor", glm::vec3(0.4f, 0.25f, 0.1f));
        drawCube(shader, base, glm::vec3(0, H-0.02f, frontZ+0.02f), glm::vec3(W+0.1f, 0.15f, 0.06f));
        
        // ============================================================
        // 2. BACKDROP FRAME (Thick wooden pillars + back wall)
        // ============================================================
        shader.setVec3("objectColor", glm::vec3(0.5f, 0.32f, 0.15f));
        
        // Left Pole (thick)
        drawCube(shader, base, glm::vec3(-W/2+0.3f, H+PH/2, backZ), glm::vec3(0.35f, PH, 0.35f));
        // Right Pole (thick)
        drawCube(shader, base, glm::vec3( W/2-0.3f, H+PH/2, backZ), glm::vec3(0.35f, PH, 0.35f));
        // Top beam (thick)
        drawCube(shader, base, glm::vec3(0, H+PH+0.1f, backZ), glm::vec3(W, 0.35f, 0.35f));
        
        // Solid back wall
        drawCube(shader, base, glm::vec3(0, H+PH/2, backZ-0.08f), glm::vec3(W, PH, 0.1f));

        // ============================================================
        // 3. BANNER (stage.png)
        // ============================================================
        if (stageTex != 0) {
            shader.setVec3("emission", glm::vec3(0.18f));
            glBindTexture(GL_TEXTURE_2D, stageTex);
            shader.setVec3("objectColor", glm::vec3(1.0f));
            drawCube(shader, base, glm::vec3(0, H+PH/2+0.3f, backZ+0.12f), glm::vec3(W-2.5f, PH-1.5f, 0.05f));
            shader.setVec3("emission", glm::vec3(0.0f));
        }
        // Rebind wood texture for subsequent drawing
        if (woodTex != 0) glBindTexture(GL_TEXTURE_2D, woodTex);

        // ============================================================
        // 4. FLOOR MATS (Textured with Fabric images - Matte finish)
        // ============================================================
        shader.setBool("useTexture", false);
        glBindVertexArray(cubeVAO);
        float mz = frontZ + 3.5f;
        
        // Apply very low shininess for matte fabric look
        shader.setVec3("material.specular", glm::vec3(0.05f, 0.05f, 0.05f));
        shader.setFloat("material.shininess", 4.0f);
        
        struct MatDef { float x, z, w, d; glm::vec3 border, inner; };
        MatDef mats[] = {
            {-4.5f, mz,      3.2f, 2.8f, glm::vec3(0.55f,0.12f,0.1f),  glm::vec3(0.65f,0.2f,0.15f)},
            { 0.0f, mz,      3.2f, 2.8f, glm::vec3(0.12f,0.3f,0.55f),  glm::vec3(0.2f,0.4f,0.6f)},
            { 4.5f, mz,      3.2f, 2.8f, glm::vec3(0.12f,0.42f,0.15f), glm::vec3(0.2f,0.52f,0.22f)},
            {-2.5f, mz+3.0f, 2.8f, 2.4f, glm::vec3(0.55f,0.38f,0.1f),  glm::vec3(0.65f,0.48f,0.18f)},
            { 2.5f, mz+3.0f, 2.8f, 2.4f, glm::vec3(0.42f,0.12f,0.38f), glm::vec3(0.52f,0.2f,0.45f)},
        };
        for (int i = 0; i < 5; ++i) {
            auto& mt = mats[i];
            
            // Border (Solid color)
            shader.setBool("useTexture", false);
            shader.setVec3("objectColor", mt.border);
            drawCube(shader, base, glm::vec3(mt.x, 0.02f, mt.z), glm::vec3(mt.w, 0.04f, mt.d));
            
            // Inner Mat (Textured with fabric)
            if (i < fabricTexs.size() && fabricTexs[i] != 0) {
                shader.setBool("useTexture", true);
                glBindTexture(GL_TEXTURE_2D, fabricTexs[i]);
                shader.setVec3("objectColor", glm::vec3(1.0f));
            } else {
                shader.setBool("useTexture", false);
                shader.setVec3("objectColor", mt.inner);
            }
            drawCube(shader, base, glm::vec3(mt.x, 0.035f, mt.z), glm::vec3(mt.w-0.35f, 0.04f, mt.d-0.35f));
        }
        
        // Restore defaults
        shader.setVec3("material.specular", glm::vec3(0.5f, 0.5f, 0.5f));
        shader.setFloat("material.shininess", 32.0f);
        shader.setBool("useTexture", false); // Default for next sections

        // ============================================================
        // 5. THICK MARIGOLD GARLANDS — The main attraction!
        // ============================================================
        shader.setBool("useTexture", false);
        shader.setVec3("emission", glm::vec3(0.12f, 0.06f, 0.0f));

        float fz = frontZ + 0.15f;
        float topY = H + PH + 0.25f;
        float bz = backZ + 0.3f;

        // ---- FRONT STAGE garlands (drooping from the platform edge) ----
        drawGarland(shader, base, glm::vec3(-W/2,     H+0.1f, fz), glm::vec3(-W/4,     H+0.1f, fz), 0.55f, 40);
        drawGarland(shader, base, glm::vec3(-W/4,     H+0.1f, fz), glm::vec3(  0,      H+0.1f, fz), 0.55f, 40);
        drawGarland(shader, base, glm::vec3(  0,      H+0.1f, fz), glm::vec3( W/4,     H+0.1f, fz), 0.55f, 40);
        drawGarland(shader, base, glm::vec3( W/4,     H+0.1f, fz), glm::vec3( W/2,     H+0.1f, fz), 0.55f, 40);

        // ---- TOP FRAME garlands (drooping from the top beam) ----
        drawGarland(shader, base, glm::vec3(-W/2+0.3f, topY, bz), glm::vec3(-W/4,      topY, bz), 0.65f, 40);
        drawGarland(shader, base, glm::vec3(-W/4,      topY, bz), glm::vec3(  0,       topY, bz), 0.65f, 40);
        drawGarland(shader, base, glm::vec3(  0,       topY, bz), glm::vec3( W/4,      topY, bz), 0.65f, 40);
        drawGarland(shader, base, glm::vec3( W/4,      topY, bz), glm::vec3( W/2-0.3f, topY, bz), 0.65f, 40);

        // ---- SIDE VERTICAL garlands hanging down from the top beam ----
        drawGarland(shader, base, glm::vec3(-W/2+0.4f, topY, bz), glm::vec3(-W/2+0.4f, H+0.8f, bz), 0.0f, 30);
        drawGarland(shader, base, glm::vec3( W/2-0.4f, topY, bz), glm::vec3( W/2-0.4f, H+0.8f, bz), 0.0f, 30);

        // ---- SIDE garlands on the front face of the stage (left & right edges) ----
        drawGarland(shader, base, glm::vec3(-W/2+0.15f, H+0.1f, fz), glm::vec3(-W/2+0.15f, 0.1f, fz), 0.0f, 18);
        drawGarland(shader, base, glm::vec3( W/2-0.15f, H+0.1f, fz), glm::vec3( W/2-0.15f, 0.1f, fz), 0.0f, 18);
        
        // ---- FRONT STEP garland (one continuous garland along the lower step) ----
        drawGarland(shader, base, glm::vec3(-W/2-0.3f, H*0.5f+0.05f, frontZ+1.0f), glm::vec3(-W/4, H*0.5f+0.05f, frontZ+1.0f), 0.3f, 30);
        drawGarland(shader, base, glm::vec3(-W/4,      H*0.5f+0.05f, frontZ+1.0f), glm::vec3(0,    H*0.5f+0.05f, frontZ+1.0f), 0.3f, 30);
        drawGarland(shader, base, glm::vec3(0,          H*0.5f+0.05f, frontZ+1.0f), glm::vec3(W/4,  H*0.5f+0.05f, frontZ+1.0f), 0.3f, 30);
        drawGarland(shader, base, glm::vec3(W/4,        H*0.5f+0.05f, frontZ+1.0f), glm::vec3(W/2+0.3f, H*0.5f+0.05f, frontZ+1.0f), 0.3f, 30);

        shader.setVec3("emission", glm::vec3(0.0f));

        // ============================================================
        // 6. COLORFUL HANGING STARS (Matte finish, Slow sway)
        // ============================================================
        glBindVertexArray(cubeVAO);
        
        // Matte finish for paper/fabric stars
        shader.setVec3("material.specular", glm::vec3(0.08f, 0.08f, 0.08f));
        shader.setFloat("material.shininess", 8.0f);
        
        glm::vec3 starColors[] = {
            glm::vec3(0.85f, 0.1f, 0.15f),   // Red
            glm::vec3(0.1f, 0.65f, 0.15f),   // Green
            glm::vec3(0.9f, 0.7f, 0.0f),     // Gold
            glm::vec3(0.55f, 0.1f, 0.7f),    // Purple
            glm::vec3(0.1f, 0.45f, 0.85f),   // Blue
            glm::vec3(0.9f, 0.4f, 0.1f),     // Orange
            glm::vec3(0.85f, 0.1f, 0.5f),    // Pink
        };
        
        float starX[] = {-4.8f, -3.0f, -1.2f, 0.6f, 2.4f, 4.2f, 5.2f};
        float starDrops[] = {1.2f, 2.0f, 1.5f, 2.3f, 1.8f, 1.3f, 2.1f};
        for (int i = 0; i < 7; ++i) {
            float sy = H + PH - starDrops[i];
            float sz = backZ + 0.35f;
            float topY = H + PH;
            
            // Slower, more rhythmic swaying (matches booth flags feel)
            float swayX = sinf(time * 0.8f + i * 0.6f) * 3.5f;
            float swayZ = cosf(time * 0.7f + i * 1.1f) * 2.5f;
            
            // String (now pivots from top with sway)
            shader.setVec3("objectColor", glm::vec3(0.15f));
            shader.setVec3("emission", glm::vec3(0.0f));
            
            glm::mat4 stringMat = glm::translate(base, glm::vec3(starX[i], topY, sz));
            stringMat = glm::rotate(stringMat, glm::radians(swayX), glm::vec3(1, 0, 0));
            stringMat = glm::rotate(stringMat, glm::radians(swayZ), glm::vec3(0, 0, 1));
            
            // Draw string hanging down
            drawCube(shader, stringMat, glm::vec3(0, -starDrops[i] / 2.0f, 0), glm::vec3(0.02f, starDrops[i], 0.02f));
            
            // Star body = 2 overlapping rotated cubes at the bottom of the swaying string
            shader.setVec3("objectColor", starColors[i]);
            shader.setVec3("emission", starColors[i] * 0.2f);
            float ss = 0.35f + (i % 3) * 0.1f;
            
            glm::mat4 starBase = glm::translate(stringMat, glm::vec3(0, -starDrops[i], 0));
            
            glm::mat4 s1 = glm::rotate(starBase, glm::radians(45.0f), glm::vec3(0, 0, 1));
            s1 = glm::rotate(s1, glm::radians(time * 8.0f + i * 15.0f), glm::vec3(0, 1, 0)); // Much slower spin
            s1 = glm::scale(s1, glm::vec3(ss, ss, 0.06f));
            shader.setMat4("model", s1);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            
            glm::mat4 s2 = glm::rotate(starBase, glm::radians(22.5f), glm::vec3(0, 0, 1));
            s2 = glm::rotate(s2, glm::radians(time * 8.0f + i * 15.0f), glm::vec3(0, 1, 0)); // Much slower spin
            s2 = glm::scale(s2, glm::vec3(ss*0.85f, ss*0.85f, 0.06f));
            shader.setMat4("model", s2);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            
            shader.setVec3("emission", glm::vec3(0.0f));
        }

        // ============================================================ 
        // 7. TRIANGULAR BUNTING along front edge
        // ============================================================
        glBindVertexArray(cubeVAO);
        glm::vec3 buntCols[] = {
            glm::vec3(1.0f, 0.15f, 0.15f),  // Bright Red
            glm::vec3(0.15f, 0.85f, 0.15f), // Bright Green
            glm::vec3(1.0f, 0.85f, 0.05f),  // Bright Yellow
            glm::vec3(0.15f, 0.4f, 1.0f),   // Bright Blue
            glm::vec3(0.9f, 0.2f, 0.75f),   // Bright Pink
            glm::vec3(1.0f, 0.6f, 0.05f),   // Bright Orange
        };
        
        float buntY = H - 0.05f;
        float buntZ = frontZ + 0.06f;
        int nBunt = 14;
        float buntSpc = (W - 0.2f) / (float)nBunt;
        for (int i = 0; i < nBunt; ++i) {
            float bx = -W/2 + 0.1f + i * buntSpc + buntSpc/2.0f;
            shader.setVec3("objectColor", buntCols[i % 6]);
            shader.setVec3("emission", buntCols[i % 6] * 0.15f); // Slight glow for vibrancy
            
            glm::mat4 tri = glm::translate(base, glm::vec3(bx, buntY - 0.3f, buntZ));
            tri = glm::rotate(tri, glm::radians(45.0f), glm::vec3(0, 0, 1));
            tri = glm::scale(tri, glm::vec3(0.38f, 0.38f, 0.02f));
            shader.setMat4("model", tri);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        shader.setVec3("emission", glm::vec3(0.0f));
        // Bunting string
        shader.setVec3("objectColor", glm::vec3(0.25f, 0.15f, 0.08f));
        drawCube(shader, base, glm::vec3(0, buntY, buntZ), glm::vec3(W, 0.03f, 0.03f));

        // ============================================================
        // 8. DECORATIVE CLAY POTS (larger, more visible, matte finish)
        // ============================================================
        shader.setBool("useTexture", false);
        shader.setVec3("emission", glm::vec3(0.0f));
        
        // Set zero specular/shininess for a truly matte, realistic clay look
        shader.setVec3("material.specular", glm::vec3(0.0f, 0.0f, 0.0f));
        shader.setVec3("lightSpecular", glm::vec3(0.0f, 0.0f, 0.0f));
        shader.setFloat("material.shininess", 1.0f);
        shader.setFloat("shininess", 1.0f);
        
        float potX[] = {-W/2+1.0f, W/2-1.0f, -W/2+1.0f, W/2-1.0f};
        float potZ[] = {backZ+1.0f, backZ+1.0f, frontZ-0.5f, frontZ-0.5f};
        glm::vec3 potCols[] = {
            glm::vec3(0.9f, 0.1f, 0.1f),   // Red
            glm::vec3(0.9f, 0.9f, 0.9f),   // White
            glm::vec3(0.9f, 0.8f, 0.1f),   // Yellow
            glm::vec3(0.6f, 0.1f, 0.7f)    // Purple
        };
        
        for (int i = 0; i < 4; ++i) {
            float x = potX[i];
            float z = potZ[i];
            float yBase = H + 0.05f; // Sitting directly on the stage surface
            float scale = 0.85f; // Big visible pots!
            
            // --- Exact logic from Stall.h "drawPot" with textures ---
            float bodyH = scale * 0.85f;
            
            // Body Sphere (Textured with noksha.png)
            if (nokshaTex != 0) {
                shader.setBool("useTexture", true);
                glBindTexture(GL_TEXTURE_2D, nokshaTex);
                shader.setVec3("objectColor", {1.0f, 1.0f, 1.0f});
            } else {
                shader.setBool("useTexture", false);
                shader.setVec3("objectColor", potCols[i]);
            }
            
            glm::mat4 m = glm::translate(base, glm::vec3(x, yBase + bodyH * 0.45f, z));
            m = glm::scale(m, glm::vec3(scale, bodyH, scale));
            shader.setMat4("model", m);
            // Use textured sphere VAO if texture is enabled
            if (nokshaTex != 0) {
                glBindVertexArray(potSphere.sphereTexVAO);
                glDrawElements(GL_TRIANGLES, (GLsizei)potSphere.getIndexCount(), GL_UNSIGNED_INT, 0);
            } else {
                sphere.drawSphere(shader);
            }
            
            shader.setBool("useTexture", false);
            shader.setVec3("objectColor", potCols[i]);
            
            // Neck - Deeply fused into the body
            float neckR = scale * 0.16f; 
            float neckH = scale * 0.35f;
            glm::mat4 n = glm::translate(base, glm::vec3(x, yBase + bodyH * 0.65f, z)); 
            n = glm::scale(n, glm::vec3(neckR, neckH, neckR));
            shader.setMat4("model", n);
            cylinder.draw();
            
            // Solid Mouth/Rim (Using flattened sphere for solid cap, NO floating cylinder rings!)
            shader.setVec3("objectColor", potCols[i] * 0.9f); // Slightly darker rim for depth
            glm::mat4 r = glm::translate(base, glm::vec3(x, yBase + bodyH * 0.65f + neckH - 0.02f, z));
            r = glm::scale(r, glm::vec3(scale * 0.35f, 0.06f, scale * 0.35f));
            shader.setMat4("model", r);
            sphere.drawSphere(shader); 
        }
        
        // Restore default specular/shininess
        shader.setVec3("material.specular", glm::vec3(0.5f, 0.5f, 0.5f));
        shader.setFloat("material.shininess", 32.0f);

        // ============================================================
        // 9. LANTERNS on both side poles
        // ============================================================
        shader.setVec3("emission", glm::vec3(0.35f, 0.25f, 0.05f));
        
        float lx[] = {-W/2+0.3f, W/2-0.3f};
        for (int i = 0; i < 2; ++i) {
            float ly = H + PH - 0.6f;
            float lz = backZ + 0.3f;
            
            // Lantern body (golden glow)
            shader.setVec3("objectColor", glm::vec3(0.85f, 0.65f, 0.1f));
            glm::mat4 lb = glm::translate(base, glm::vec3(lx[i], ly, lz));
            lb = glm::scale(lb, glm::vec3(0.22f, 0.35f, 0.22f));
            shader.setMat4("model", lb);
            sphere.drawSphere(shader);
            
            // Lantern cap (dark metal)
            shader.setVec3("objectColor", glm::vec3(0.12f));
            glm::mat4 lt = glm::translate(base, glm::vec3(lx[i], ly+0.22f, lz));
            lt = glm::scale(lt, glm::vec3(0.18f, 0.08f, 0.18f));
            shader.setMat4("model", lt);
            cylinder.draw();
            
            // Lantern hook
            shader.setVec3("objectColor", glm::vec3(0.12f));
            drawCube(shader, base, glm::vec3(lx[i], ly+0.35f, lz), glm::vec3(0.03f, 0.2f, 0.03f));
        }
        shader.setVec3("emission", glm::vec3(0.0f));
        
        // ============================================================
        // 10. STAGE FLOOR BORDER (wood trim around the edges)
        // ============================================================
        shader.setBool("useTexture", true);
        if (woodTex != 0) glBindTexture(GL_TEXTURE_2D, woodTex);
        glBindVertexArray(cubeVAO);
        shader.setVec3("objectColor", glm::vec3(0.45f, 0.28f, 0.12f));
        
        // Front lip
        drawCube(shader, base, glm::vec3(0, H+0.02f, frontZ), glm::vec3(W+0.1f, 0.06f, 0.15f));
        // Back lip
        drawCube(shader, base, glm::vec3(0, H+0.02f, -D/2), glm::vec3(W+0.1f, 0.06f, 0.15f));
        // Left lip
        drawCube(shader, base, glm::vec3(-W/2, H+0.02f, 0), glm::vec3(0.15f, 0.06f, D+0.1f));
        // Right lip
        drawCube(shader, base, glm::vec3(W/2, H+0.02f, 0), glm::vec3(0.15f, 0.06f, D+0.1f));
        
        shader.setBool("useTexture", true); // Restore

        // ============================================================
        // 11. LED BOBO BALLOONS — 3 per corner, all from same base (bouquet)
        //   Lean angles large enough so balloons never overlap:
        //   arm=3.28, minSep needed=1.56, actual≈1.9+ at all times
        // ============================================================
        shader.setBool("useTexture", false);
        // Left cluster
        glm::vec3 LB(-W/2, H, frontZ);
        drawBalloon(shader, base, LB, time, 0.00f, 0.80f,   5.0f,  3.0f);  // center-slight-fwd
        drawBalloon(shader, base, LB, time, 2.09f, 0.74f, -33.0f,-14.0f);  // left-back
        drawBalloon(shader, base, LB, time, 4.19f, 0.76f,  22.0f,-26.0f);  // right-back
        // Right cluster (leanX mirrored)
        glm::vec3 RB( W/2, H, frontZ);
        drawBalloon(shader, base, RB, time, 1.05f, 0.80f,  -5.0f,  3.0f);
        drawBalloon(shader, base, RB, time, 3.14f, 0.74f,  33.0f,-14.0f);
        drawBalloon(shader, base, RB, time, 5.24f, 0.76f, -22.0f,-26.0f);
    }

private:
    unsigned int cubeVAO;
    Cylinder cylinder;
    Sphere sphere;
    SphereWithTexture potSphere;

    // Helper: draw a cube at position with size
    void drawCube(Shader& shader, glm::mat4 base, glm::vec3 pos, glm::vec3 size) {
        glm::mat4 m = glm::translate(base, pos);
        m = glm::scale(m, size);
        shader.setMat4("model", m);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // Draw one LED bobo balloon. staticLeanX/Z set the resting angle (bouquet fan-out).
    void drawBalloon(Shader& shader, glm::mat4 base, glm::vec3 cornerBase,
                     float time, float phase, float balloonR = 0.85f,
                     float staticLeanX = 0.0f, float staticLeanZ = 0.0f) {
        shader.setBool("useTexture", false);
        const float stickH = 2.5f;

        // ---- Pendulum: static lean + gentle sway (small amplitude so bouquet stays spread) ----
        float angleX = staticLeanX + 2.5f * sinf(time * 1.15f + phase);
        float angleZ = staticLeanZ + 2.0f * sinf(time * 0.88f + phase + 1.3f);
        glm::mat4 pivBase = glm::translate(base, cornerBase);
        pivBase = glm::rotate(pivBase, glm::radians(angleZ), glm::vec3(0.0f, 0.0f, 1.0f));
        pivBase = glm::rotate(pivBase, glm::radians(angleX), glm::vec3(1.0f, 0.0f, 0.0f));

        glm::vec3 balloonLocal = glm::vec3(0.0f, stickH + balloonR, 0.0f);

        // ---- Stick ----
        glBindVertexArray(cubeVAO);
        shader.setVec3("objectColor", glm::vec3(0.88f, 0.90f, 0.95f));
        shader.setVec3("emission",    glm::vec3(0.02f, 0.02f, 0.03f));
        drawCube(shader, pivBase, glm::vec3(0.0f, stickH * 0.5f, 0.0f), glm::vec3(0.04f, stickH, 0.04f));

        // ---- LED string along stick — sharp blink ----
        glm::vec3 stickCols[] = {
            {1.0f,0.08f,0.08f},{0.2f,0.5f,1.0f},{0.08f,1.0f,0.18f},
            {1.0f,0.85f,0.08f},{0.6f,0.1f,1.0f},{1.0f,0.45f,0.0f},
            {0.1f,0.9f,0.9f},  {1.0f,0.25f,0.7f},
        };
        for (int i = 0; i < 8; ++i) {
            float blinkFreq = 3.0f + (i * 0.5f);
            float bp = sinf(time * blinkFreq + i * 1.5f + phase);
            float tw = bp > 0.0f ? (0.6f + 0.4f * bp) : 0.0f; // Sharp blink
            
            shader.setVec3("objectColor", stickCols[i] * 0.15f);
            shader.setVec3("emission",    stickCols[i] * tw * 1.5f);
            float t = (float)(i + 1) / 9.0f;
            glm::mat4 lm = glm::translate(pivBase, glm::vec3(0.0f, t * stickH, 0.0f));
            lm = glm::scale(lm, glm::vec3(0.085f));
            shader.setMat4("model", lm);
            sphere.drawSphere(shader);
        }

        // ---- Balloon shell (Clear plastic look with subtle, realistic highlights) ----
        shader.setVec3("material.specular", glm::vec3(0.35f, 0.35f, 0.4f));
        shader.setFloat("material.shininess", 64.0f);
        shader.setVec3("objectColor", glm::vec3(0.62f, 0.78f, 1.00f));
        shader.setVec3("emission",    glm::vec3(0.06f, 0.09f, 0.22f));
        glm::mat4 bm = glm::translate(pivBase, balloonLocal);
        bm = glm::scale(bm, glm::vec3(balloonR * 2.0f, balloonR * 2.3f, balloonR * 2.0f));
        shader.setMat4("model", bm);
        sphere.drawSphere(shader);

        // ---- Surface LEDs — golden spiral, 45 dots, true on/off blink ----
        // LEDs sit just OUTSIDE the shell (r×1.06): front ones visible,
        // back ones depth-occluded by balloon — no depth-test hacks needed.
        const int   N      = 45;
        const float gAngle = 2.399963f;  // golden ratio angle

        // Palette: 40 % blue, 20 % purple, 40 % mixed
        glm::vec3 pal[] = {
            {0.20f,0.45f,1.00f},  // 0 blue
            {0.30f,0.10f,1.00f},  // 1 deep blue
            {0.55f,0.08f,0.95f},  // 2 purple
            {0.75f,0.08f,0.85f},  // 3 violet
            {1.00f,0.08f,0.12f},  // 4 red
            {1.00f,0.48f,0.00f},  // 5 orange
            {0.08f,0.95f,0.18f},  // 6 green
            {0.08f,0.92f,0.95f},  // 7 cyan
            {1.00f,1.00f,1.00f},  // 8 white
            {1.00f,0.88f,0.10f},  // 9 yellow
        };

        for (int i = 0; i < N; ++i) {
            // Even spherical coverage
            float cosLat = 1.0f - 2.0f * (float)i / (float)(N - 1);
            float sinLat = sqrtf(fmaxf(0.0f, 1.0f - cosLat * cosLat));
            float az     = gAngle * i;
            glm::vec3 surfN = glm::normalize(glm::vec3(sinLat * cosf(az), cosLat / 1.15f, sinLat * sinf(az)));
            glm::vec3 lp    = balloonLocal + surfN * (balloonR * 1.06f);

            // Realistic blink: 2.0–4.0 Hz range, each LED independent
            float sp  = 2.0f + (i % 7) * 0.35f;
            float bp  = sinf(time * sp + i * 1.41f + phase);
            float tw  = bp > 0.1f ? (0.5f + 0.5f * bp) : 0.0f;

            // Color: 40 % blue/deep-blue, 20 % purple/violet, 40 % others
            int ci;
            int r = i % 10;
            if      (r < 2) ci = 0;
            else if (r < 4) ci = 1;
            else if (r < 5) ci = 2;
            else if (r < 6) ci = 3;
            else            ci = 4 + ((i / 10) % 6);
            if (ci > 9) ci = 9;

            glm::vec3 col = pal[ci];
            shader.setVec3("objectColor", col * 0.15f);
            shader.setVec3("emission",    col * tw * 1.8f); // Bright LEDs
            glm::mat4 lm = glm::translate(pivBase, lp);
            lm = glm::scale(lm, glm::vec3(0.075f));
            shader.setMat4("model", lm);
            sphere.drawSphere(shader);
        }

        // ---- Restore defaults ----
        shader.setVec3("emission", glm::vec3(0.0f));
        shader.setVec3("material.specular", glm::vec3(0.5f, 0.5f, 0.5f));
        shader.setFloat("material.shininess", 32.0f);
    }

    // Draw a dense chain of marigold flowers from p1 to p2 with parabolic droop
    void drawGarland(Shader& shader, glm::mat4 base, glm::vec3 p1, glm::vec3 p2, float droop, int count) {
        // Warm orange/yellow palette for natural marigold look
        glm::vec3 cols[] = {
            glm::vec3(0.92f, 0.52f, 0.04f),  // Deep orange
            glm::vec3(0.98f, 0.72f, 0.08f),  // Golden yellow
            glm::vec3(0.88f, 0.42f, 0.06f),  // Dark marigold
            glm::vec3(0.95f, 0.62f, 0.05f),  // Bright orange
            glm::vec3(1.0f,  0.78f, 0.12f),  // Light gold
        };
        
        for (int i = 0; i <= count; ++i) {
            float t = (float)i / (float)count;
            glm::vec3 pos = p1 * (1.0f - t) + p2 * t;
            pos.y -= 4.0f * droop * t * (1.0f - t);
            
            shader.setVec3("objectColor", cols[i % 5]);
            
            // Each flower = squashed sphere (flat marigold shape, overlapping neighbours)
            glm::mat4 m = glm::translate(base, pos);
            m = glm::scale(m, glm::vec3(0.16f, 0.12f, 0.16f));
            shader.setMat4("model", m);
            sphere.drawSphere(shader);
        }
    }
};

#endif
