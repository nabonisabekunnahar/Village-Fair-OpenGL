#ifndef RINGTOSS_H
#define RINGTOSS_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <iostream>
#include "shader.h"
#include "cylinder.h"
#include "sphere.h"
#include "pyramid.h"

class RingToss {
public:
    RingToss(unsigned int cubeVAO) 
        : cubeVAO(cubeVAO), cylinder(24), sphere(1.0f, 24, 12), pyramid()
    {
        score = 0;
        aimAngle = 0.0f;
        pitchAngle = 30.0f;
        isLanded = false;
        resetTimer = 0.0f;
        resetRing();
        
        // Scattered layout (no columns, full random feel)
        float row1X[] = { -2.2f, -1.1f, -0.1f,  1.2f,  2.1f };
        float row2X[] = { -1.8f, -0.6f,  0.5f,  1.5f,  2.5f };
        float row3X[] = { -2.5f, -1.4f, -0.3f,  0.8f,  1.9f };

        // Row 1 (Front) EXACTLY on table top (Y=0.57)
        for (int i = 0; i < 5; ++i) {
            bottlePositions.push_back(glm::vec3(row1X[i], 0.57f, 0.8f));
            bottleColors.push_back(getPaletteColor(i));
            bottleHit.push_back(false);
        }
        // Row 2 (Middle) EXACTLY on step 2 (Y=0.77)
        for (int i = 0; i < 5; ++i) {
            bottlePositions.push_back(glm::vec3(row2X[i], 0.77f, -0.2f));
            bottleColors.push_back(getPaletteColor(i + 1));
            bottleHit.push_back(false);
        }
        // Row 3 (Back) EXACTLY on step 3 (Y=0.99)
        for (int i = 0; i < 5; ++i) {
            bottlePositions.push_back(glm::vec3(row3X[i], 0.99f, -1.2f));
            bottleColors.push_back(getPaletteColor(i + 2));
            bottleHit.push_back(false);
        }
    }

    void fullReset() {
        score = 0;
        for (size_t i = 0; i < bottleHit.size(); ++i) bottleHit[i] = false;
        aimAngle = 0.0f;
        pitchAngle = 30.0f;
        resetRing();
    }

    void resetRing() {
        ringPos = glm::vec3(0.0f, 0.8f, 4.0f); 
        velocity = glm::vec3(0.0f);
        isThrown = false;
        isLanded = false;
        resetTimer = 0.0f;
    }

    void throwRing() {
        if (!isThrown) {
            isThrown = true;
            float speed = 7.0f;
            float yaw = glm::radians(aimAngle);
            float pitch = glm::radians(pitchAngle);
            float hSpeed = speed * cos(pitch);
            velocity = glm::vec3(sin(yaw) * hSpeed * 0.5f, speed * sin(pitch), -hSpeed);
        }
    }

    void aimLeft(float dt) { if (!isThrown) { ringPos.x -= 3.0f * dt; if (ringPos.x < -3.0f) ringPos.x = -3.0f; } }
    void aimRight(float dt) { if (!isThrown) { ringPos.x += 3.0f * dt; if (ringPos.x > 3.0f) ringPos.x = 3.0f; } }
    void aimUp(float dt) { if (!isThrown) { pitchAngle += 40.0f * dt; if (pitchAngle > 60.0f) pitchAngle = 60.0f; } }
    void aimDown(float dt) { if (!isThrown) { pitchAngle -= 40.0f * dt; if (pitchAngle < 10.0f) pitchAngle = 10.0f; } }

    void update(float dt) {
        if (isLanded) {
            resetTimer -= dt;
            if (resetTimer <= 0.0f) resetRing();
            return;
        }

        if (isThrown) {
            ringPos += velocity * dt;
            velocity.y -= 10.0f * dt;

            // Hit logic (cylindrical bounding volume)
            for (size_t i = 0; i < bottlePositions.size(); ++i) {
                if (bottleHit[i]) continue;
                glm::vec3 bPos = bottlePositions[i];
                float distXZ = glm::distance(glm::vec2(ringPos.x, ringPos.z), glm::vec2(bPos.x, bPos.z));

                if (distXZ < 0.25f && ringPos.y >= bPos.y && ringPos.y <= bPos.y + 0.8f && velocity.y < 0) {
                    bottleHit[i] = true;
                    score++;
                    std::cout << "HIT! Score: " << score << std::endl;
                    resetRing(); // Instant reset, winner ring shown permanently
                    return;
                }
            }

            // Miss logic (Ground/Table intersection)
            float floorY = 0.035f; // Grass level
            if (ringPos.z > -1.7f && ringPos.z < 1.0f) floorY = 0.57f; // Table top boundary

            if (ringPos.y <= floorY) {
                ringPos.y = floorY;
                isLanded = true;
                resetTimer = 1.2f; // Wait 1.2s to show clearly where the ring fell (missed)
            }
        }
    }

    void draw(Shader& shader, glm::vec3 pos, unsigned int fabricTex, unsigned int woodTex) {
        glm::mat4 baseModel = glm::mat4(1.0f);
        baseModel = glm::translate(baseModel, pos);

        drawBackboard(shader, baseModel, fabricTex);
        drawTable(shader, baseModel, woodTex);
        drawTallyStars(shader, baseModel);

        for (size_t i = 0; i < bottlePositions.size(); ++i) {
            drawBottle(shader, baseModel, bottlePositions[i], bottleColors[i], bottleHit[i]);
            if (bottleHit[i]) {
                glm::mat4 hitRm = glm::translate(baseModel, bottlePositions[i] + glm::vec3(0, 0.08f, 0));
                drawRing(shader, hitRm, glm::vec3(1, 0, 0));
            }
        }

        drawGround(shader, baseModel);

        // Active ring
        glm::mat4 rm = glm::translate(baseModel, ringPos);
        if (!isThrown) {
            rm = glm::rotate(rm, glm::radians(aimAngle), glm::vec3(0, 1, 0));
            rm = glm::rotate(rm, glm::radians(pitchAngle), glm::vec3(1, 0, 0)); // visual pitch tilt
        } else if (isLanded) {
            // Lying flat when missed
        } else {
            rm = glm::rotate(rm, glm::radians(aimAngle), glm::vec3(0, 1, 0));
            rm = glm::rotate(rm, (float)glfwGetTime() * 10.0f, glm::vec3(1, 0.4f, 0));
        }
        drawRing(shader, rm, glm::vec3(1, 0, 0));
    }

private:
    unsigned int cubeVAO;
    Cylinder cylinder;
    Sphere sphere;
    Pyramid pyramid;

    glm::vec3 ringPos;
    glm::vec3 velocity;
    bool isThrown;
    float aimAngle;
    float pitchAngle;
    bool isLanded;
    float resetTimer;
    int score;

    std::vector<glm::vec3> bottlePositions;
    std::vector<glm::vec3> bottleColors;
    std::vector<bool> bottleHit;

    glm::vec3 getPaletteColor(int i) {
        std::vector<glm::vec3> palette = {
            glm::vec3(0.1f, 0.35f, 0.9f),  // blue
            glm::vec3(0.95f, 0.1f, 0.1f),   // red
            glm::vec3(0.1f, 0.7f, 0.15f),   // green
            glm::vec3(0.6f, 0.15f, 0.85f),  // purple
            glm::vec3(0.95f, 0.85f, 0.1f)   // yellow
        };
        return palette[i % 5];
    }

    void drawTallyStars(Shader& shader, glm::mat4 base) {
        if (score == 0) return;
        shader.setBool("useTexture", false);
        shader.setVec3("objectColor", glm::vec3(1.0f, 0.84f, 0.0f)); 
        float spacing = 3.6f / 15.0f; 
        float totalWidth = (score - 1) * spacing;
        float startX = -totalWidth / 2.0f;
        for (int i = 0; i < score; ++i) {
            glm::mat4 s = glm::translate(base, glm::vec3(startX + i * spacing, 3.1f, -1.5f)); // Vertically centered on banner at Y=3.1
            s = glm::scale(s, glm::vec3(0.18f));
            shader.setMat4("model", s);
            pyramid.draw();
        }
    }

    void drawCappedCylinder(Shader& shader, glm::mat4 model, glm::vec3 col) {
        shader.setVec3("objectColor", col);
        shader.setMat4("model", model);
        cylinder.draw();
        // Top cap (sphere scaled flat)
        glm::mat4 topCap = glm::translate(model, glm::vec3(0, 1.0f, 0));
        topCap = glm::scale(topCap, glm::vec3(1.0f, 0.015f, 1.0f));
        shader.setMat4("model", topCap);
        sphere.drawSphere(shader);
    }

    // ========================================================
    //  BOTTLE: capped cylinder body + sphere shoulder + capped neck
    //  Total height ~0.8 units. Sits ON the table/step surface.
    // ========================================================
    void drawBottle(Shader& shader, glm::mat4 base, glm::vec3 pos, glm::vec3 col, bool hit) {
        shader.setBool("useTexture", false);
        glm::vec3 c = hit ? col * 0.3f : col;

        // Circular wooden base pad (placed exactly on pos.y=0 via scaling up from pos)
        // Cylinder is drawn from 0 to 1, so pos + 0 translation means it sits exactly on the surface!
        glm::mat4 pad = glm::translate(base, pos);
        pad = glm::scale(pad, glm::vec3(0.3f, 0.06f, 0.3f));
        drawCappedCylinder(shader, pad, glm::vec3(0.3f, 0.18f, 0.08f));

        // Body (cylinder)
        glm::mat4 body = glm::translate(base, pos + glm::vec3(0, 0.06f, 0));
        body = glm::scale(body, glm::vec3(0.18f, 0.35f, 0.18f));
        shader.setVec3("objectColor", c);
        shader.setMat4("model", body);
        cylinder.draw(); // Bottom part of body is hidden by pad, top goes into shoulder

        // Shoulder (sphere for smooth taper)
        // Body ends at 0.06 + 0.35 = 0.41. Sphere center at 0.41.
        glm::mat4 sh = glm::translate(base, pos + glm::vec3(0, 0.41f, 0));
        sh = glm::scale(sh, glm::vec3(0.18f, 0.16f, 0.18f));
        shader.setMat4("model", sh);
        sphere.drawSphere(shader);

        // Neck (capped cylinder)
        // Starts around 0.41 + 0.10 = 0.51
        glm::mat4 neck = glm::translate(base, pos + glm::vec3(0, 0.51f, 0));
        neck = glm::scale(neck, glm::vec3(0.08f, 0.28f, 0.08f));
        drawCappedCylinder(shader, neck, c);
    }

    // ========================================================
    //  BACKBOARD: Yellow panel + red border + side wings
    // ========================================================
    void drawBackboard(Shader& shader, glm::mat4 base, unsigned int fabricTex) {
        glBindVertexArray(cubeVAO);
        glm::vec3 yellow(1.0f, 0.92f, 0.12f);
        glm::vec3 red(0.88f, 0.08f, 0.08f);

        // Main panel (starts from ground, goes up behind table)
        shader.setBool("useTexture", false);
        shader.setVec3("objectColor", glm::vec3(0.95f, 0.95f, 0.92f)); // Realistic Warm White
        glm::mat4 bp = glm::translate(base, glm::vec3(0, 1.8f, -1.6f));
        bp = glm::scale(bp, glm::vec3(6.0f, 3.6f, 0.12f));
        shader.setMat4("model", bp);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Red border around backboard (4 strips)
        shader.setBool("useTexture", false);
        // Top
        glm::mat4 bt = glm::translate(base, glm::vec3(0, 3.65f, -1.58f));
        bt = glm::scale(bt, glm::vec3(6.2f, 0.12f, 0.14f));
        shader.setMat4("model", bt); shader.setVec3("objectColor", red);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        // Bottom
        glm::mat4 bb = glm::translate(base, glm::vec3(0, 0.0f, -1.58f));
        bb = glm::scale(bb, glm::vec3(6.2f, 0.08f, 0.14f));
        shader.setMat4("model", bb);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        // Left
        glm::mat4 bl = glm::translate(base, glm::vec3(-3.05f, 1.8f, -1.58f));
        bl = glm::scale(bl, glm::vec3(0.12f, 3.7f, 0.14f));
        shader.setMat4("model", bl);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        // Right
        glm::mat4 br = glm::translate(base, glm::vec3(3.05f, 1.8f, -1.58f));
        br = glm::scale(br, glm::vec3(0.12f, 3.7f, 0.14f));
        shader.setMat4("model", br);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Side wings 
        auto drawWing = [&](float xPos) {
            // Realistic white interior panel
            shader.setBool("useTexture", false);
            shader.setVec3("objectColor", glm::vec3(0.95f, 0.95f, 0.92f));
            glm::mat4 w = glm::translate(base, glm::vec3(xPos, 1.2f, -0.5f));
            w = glm::scale(w, glm::vec3(0.1f, 2.4f, 2.4f));
            shader.setMat4("model", w); 
            glDrawArrays(GL_TRIANGLES, 0, 36);
            
            // Red exterior trim
            shader.setBool("useTexture", false);
            shader.setVec3("objectColor", red);
            // Red exterior trim
            float extOff = (xPos > 0) ? 0.06f : -0.06f;
            glm::mat4 we = glm::translate(base, glm::vec3(xPos + extOff, 1.2f, -0.5f));
            we = glm::scale(we, glm::vec3(0.03f, 2.5f, 2.5f));
            shader.setMat4("model", we); shader.setVec3("objectColor", red);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        };
        drawWing(-3.0f);
        drawWing(3.0f);

        // "RING TOSS" banner area (red rectangle at top of backboard)
        glm::mat4 ban = glm::translate(base, glm::vec3(0, 3.1f, -1.54f));
        ban = glm::scale(ban, glm::vec3(4.0f, 0.7f, 0.04f));
        shader.setMat4("model", ban); shader.setVec3("objectColor", red);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Stars (small pyramids for decoration)
        glm::vec3 starCols[] = { {0,0.5f,0.1f}, {0.1f,0.6f,0.1f}, {0.9f,0,0}, {0.5f,0,0.8f} };
        float starX[] = { -2.4f, -2.4f, 2.4f, 2.4f };
        float starY[] = { 3.3f, 2.6f, 3.3f, 2.6f };
        for (int i = 0; i < 4; ++i) {
            glm::mat4 s = glm::translate(base, glm::vec3(starX[i], starY[i], -1.53f));
            s = glm::scale(s, glm::vec3(0.2f));
            shader.setMat4("model", s);
            shader.setVec3("objectColor", starCols[i]);
            pyramid.draw();
        }
    }

    // ========================================================
    //  TABLE: solid wooden table + 3 staircase shelf layers on top
    //  Table legs ~0.5 high, table top at Y=0.55
    //  Steps are SUBTLE: each only 0.18 higher than the last
    // ========================================================
    void drawTable(Shader& shader, glm::mat4 base, unsigned int woodTex) {
        shader.setBool("useTexture", true);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTex);
        shader.setVec3("objectColor", glm::vec3(1.0f));
        glBindVertexArray(cubeVAO);

        // ---- MAIN TABLE TOP (large, visible slab) ----
        glm::mat4 top = glm::translate(base, glm::vec3(0, 0.52f, 0.0f));
        top = glm::scale(top, glm::vec3(5.8f, 0.1f, 3.2f));
        shader.setMat4("model", top);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // ---- 4 TABLE LEGS ----
        shader.setBool("useTexture", false);
        shader.setVec3("objectColor", glm::vec3(0.35f, 0.2f, 0.1f));
        float lx = 2.5f, lz1 = -1.2f, lz2 = 1.2f;
        glm::vec3 legs[] = { {-lx,0.25f,lz1}, {lx,0.25f,lz1}, {-lx,0.25f,lz2}, {lx,0.25f,lz2} };
        for (auto& lp : legs) {
            glm::mat4 leg = glm::translate(base, lp);
            leg = glm::scale(leg, glm::vec3(0.25f, 0.5f, 0.25f));
            shader.setMat4("model", leg);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // ---- STAIRCASE SHELVES (on top of the table) ----
        shader.setBool("useTexture", true);
        shader.setVec3("objectColor", glm::vec3(0.85f));

        // Step 2 (middle shelf - clearly raised block)
        glm::mat4 s2 = glm::translate(base, glm::vec3(0, 0.66f, -0.2f));
        s2 = glm::scale(s2, glm::vec3(5.8f, 0.22f, 2.0f));
        shader.setMat4("model", s2);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Step 3 (back shelf - highest block)
        glm::mat4 s3 = glm::translate(base, glm::vec3(0, 0.88f, -1.2f));
        s3 = glm::scale(s3, glm::vec3(5.8f, 0.22f, 1.2f));
        shader.setMat4("model", s3);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // ========================================================
    //  GROUND: foul line + 5 rings laid flat on the ground
    // ========================================================
    void drawGround(Shader& shader, glm::mat4 base) {
        shader.setBool("useTexture", false);
        glBindVertexArray(cubeVAO);

        // Foul line (white stripe)
        shader.setVec3("objectColor", glm::vec3(1.0f));
        glm::mat4 line = glm::translate(base, glm::vec3(0, 0.01f, 2.5f));
        line = glm::scale(line, glm::vec3(8.0f, 0.01f, 0.15f));
        shader.setMat4("model", line);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // 5 individual rings laid flat on the ground (like the reference)
        glm::vec3 ringCols[] = { {0.9f,0.1f,0.1f}, {1,0.85f,0.05f}, {0.1f,0.4f,0.9f}, {0.1f,0.7f,0.15f}, {0.9f,0.1f,0.1f} };
        float ringX[] = { -1.8f, -0.9f, 0.0f, 0.9f, 1.8f };
        for (int i = 0; i < 5; ++i) {
            glm::mat4 rm = glm::translate(base, glm::vec3(ringX[i], 0.035f, 3.5f));
            drawRing(shader, rm, ringCols[i]);
        }
    }

    // ========================================================
    //  RING: smooth torus using Bezier-style curve sampling
    //  Generates a proper circular torus by placing many small
    //  sphere-like segments along a smooth circular path.
    //  Uses 48 sample points for a perfectly smooth circle.
    // ========================================================
    void drawRing(Shader& shader, glm::mat4 model, glm::vec3 col) {
        shader.setBool("useTexture", false);
        shader.setVec3("objectColor", col);

        float torusRadius = 0.35f;    // Major radius (center of ring to center of tube)
        float tubeRadius  = 0.045f;   // Minor radius (thickness of tube)
        int majorSegments  = 48;      // Smooth circle sampling (like Bezier curve resolution)
        int minorSegments  = 8;       // Cross-section detail

        // Build torus VAO on first call (cached as static)
        static unsigned int torusVAO = 0, torusVBO = 0;
        static int torusVertCount = 0;
        if (torusVAO == 0) {
            std::vector<float> verts;
            for (int i = 0; i < majorSegments; ++i) {
                float t0 = (float)i / majorSegments * 2.0f * 3.14159265f;
                float t1 = (float)(i + 1) / majorSegments * 2.0f * 3.14159265f;
                for (int j = 0; j < minorSegments; ++j) {
                    float p0 = (float)j / minorSegments * 2.0f * 3.14159265f;
                    float p1 = (float)(j + 1) / minorSegments * 2.0f * 3.14159265f;

                    // 4 corners of this quad on the torus surface
                    auto torusPoint = [&](float theta, float phi, float& ox, float& oy, float& oz, float& nx, float& ny, float& nz) {
                        ox = cos(theta) * (torusRadius + tubeRadius * cos(phi));
                        oy = tubeRadius * sin(phi);
                        oz = sin(theta) * (torusRadius + tubeRadius * cos(phi));
                        nx = cos(theta) * cos(phi);
                        ny = sin(phi);
                        nz = sin(theta) * cos(phi);
                    };

                    float x0, y0, z0, nx0, ny0, nz0;
                    float x1, y1, z1, nx1, ny1, nz1;
                    float x2, y2, z2, nx2, ny2, nz2;
                    float x3, y3, z3, nx3, ny3, nz3;

                    torusPoint(t0, p0, x0, y0, z0, nx0, ny0, nz0);
                    torusPoint(t1, p0, x1, y1, z1, nx1, ny1, nz1);
                    torusPoint(t1, p1, x2, y2, z2, nx2, ny2, nz2);
                    torusPoint(t0, p1, x3, y3, z3, nx3, ny3, nz3);

                    // Triangle 1
                    auto push = [&](float px, float py, float pz, float pnx, float pny, float pnz) {
                        verts.push_back(px); verts.push_back(py); verts.push_back(pz);
                        verts.push_back(pnx); verts.push_back(pny); verts.push_back(pnz);
                        verts.push_back(0); verts.push_back(0); // UV placeholder
                    };
                    push(x0,y0,z0,nx0,ny0,nz0); push(x1,y1,z1,nx1,ny1,nz1); push(x2,y2,z2,nx2,ny2,nz2);
                    push(x0,y0,z0,nx0,ny0,nz0); push(x2,y2,z2,nx2,ny2,nz2); push(x3,y3,z3,nx3,ny3,nz3);
                }
            }
            torusVertCount = (int)verts.size() / 8;
            glGenVertexArrays(1, &torusVAO);
            glGenBuffers(1, &torusVBO);
            glBindVertexArray(torusVAO);
            glBindBuffer(GL_ARRAY_BUFFER, torusVBO);
            glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
            glEnableVertexAttribArray(2);
            glBindVertexArray(0);
        }

        shader.setMat4("model", model);
        glBindVertexArray(torusVAO);
        glDrawArrays(GL_TRIANGLES, 0, torusVertCount);
        glBindVertexArray(0);
    }
};

#endif
