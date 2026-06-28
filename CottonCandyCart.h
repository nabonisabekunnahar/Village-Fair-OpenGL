#ifndef COTTON_CANDY_CART_H
#define COTTON_CANDY_CART_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <cmath>
#include "shader.h"
#include "cylinder.h"
#include "Sphere.h"
#include "sphereWithTexture.h"

class CottonCandyCart {
public:
    CottonCandyCart(unsigned int cubeVAO)
        : cubeVAO(cubeVAO), cyl(36), sph(0.5f, 24, 16),
          candySph(0.5f, 24, 16, {1,1,1},{1,1,1},{1,1,1},32.0f, 0,0, 0,0,1,1)
    {}

    void draw(Shader& shader, glm::mat4 base, glm::vec3 pos, float rotY,
              unsigned int signTex, unsigned int panelTex, float time = 0.0f,
              unsigned int cottonTex1 = 0, unsigned int cottonTex2 = 0,
              unsigned int cottonTex3 = 0, unsigned int cstandTex = 0)
    {
        glm::mat4 M = glm::rotate(
            glm::translate(base, pos),
            glm::radians(rotY), glm::vec3(0,1,0));

        drawPillars   (shader, M);
        drawBody      (shader, M, panelTex);
        drawCounter   (shader, M);
        drawAwning    (shader, M);
        drawSign      (shader, M, signTex);
        drawWheel     (shader, M);
        drawBackLegs  (shader, M);
        drawHandle    (shader, M);
        drawMachine   (shader, M);
        drawCandies   (shader, M, time, cottonTex1, cottonTex2, cottonTex3);
        drawJar       (shader, M);
        drawSmallSign (shader, M, cstandTex);
    }

private:
    unsigned int cubeVAO;
    Cylinder cyl;
    Sphere   sph;
    SphereWithTexture candySph;

    // ── palette ─────────────────────────────────────────────────
    const glm::vec3 PINK   = {0.92f, 0.32f, 0.52f};
    const glm::vec3 DPINK  = {0.75f, 0.18f, 0.38f};  // darker pink for shaded faces
    const glm::vec3 BLUE   = {0.18f, 0.46f, 0.80f};
    const glm::vec3 DBLUE  = {0.12f, 0.32f, 0.60f};
    const glm::vec3 CREAM  = {0.98f, 0.95f, 0.82f};
    const glm::vec3 SILVER = {0.72f, 0.74f, 0.78f};
    const glm::vec3 WOOD   = {0.60f, 0.42f, 0.22f};

    // ── layout — scaled up ───────────────────────────────────────
    const float CW  = 5.0f;    // cart body width
    const float CH  = 2.0f;    // cart body height
    const float CD  = 2.4f;    // cart body depth
    const float CTY = 2.14f;   // counter-top Y
    const float AWY = 6.6f;    // awning top Y

    // ── helpers ──────────────────────────────────────────────────
    void matte(Shader& s, glm::vec3 c) {
        s.setBool("useTexture", false);
        s.setVec3("objectColor", c);
        s.setVec3("emission", {0,0,0});
        s.setVec3("lightSpecular", {0,0,0});
        s.setFloat("shininess", 32.0f);
    }
    void metal(Shader& s, glm::vec3 c, float shine = 80.0f) {
        s.setBool("useTexture", false);
        s.setVec3("objectColor", c);
        s.setVec3("emission", {0,0,0});
        s.setVec3("lightSpecular", {0.35f,0.35f,0.35f});
        s.setFloat("shininess", shine);
    }
    void restoreSpec(Shader& s) { s.setVec3("lightSpecular", {1,1,1}); }

    void box(Shader& s, glm::mat4 m, glm::vec3 p, glm::vec3 sc) {
        s.setMat4("model", glm::scale(glm::translate(m,p), sc));
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    void sp(Shader& s, glm::mat4 m, glm::vec3 p, glm::vec3 sc) {
        s.setMat4("model", glm::scale(glm::translate(m,p), sc));
        sph.drawSphere(s);
    }
    void cy(Shader& s, glm::mat4 m, glm::vec3 p, glm::vec3 sc) {
        s.setMat4("model", glm::scale(glm::translate(m,p), sc));
        cyl.draw();
    }
    void tsp(Shader& s, glm::mat4 m, glm::vec3 p, glm::vec3 sc) {
        s.setMat4("model", glm::scale(glm::translate(m, p), sc));
        glBindVertexArray(candySph.sphereTexVAO);
        glDrawElements(GL_TRIANGLES, candySph.getIndexCount(), GL_UNSIGNED_INT, 0);
    }

    // ═══════════════════════════════════════════════════════════
    // 1. PILLARS — square steel, ground to AWY
    // ═══════════════════════════════════════════════════════════
    void drawPillars(Shader& s, glm::mat4 M) {
        float px = CW/2.0f + 0.08f;
        float pz = CD/2.0f + 0.08f;

        // Main square pillars
        metal(s, BLUE, 40.0f);
        for (float x : {-px, px})
            for (float z : {-pz, pz})
                box(s, M, {x, AWY/2.0f, z}, {0.20f, AWY, 0.20f});

        // Base plate at each pillar foot
        matte(s, DBLUE);
        for (float x : {-px, px})
            for (float z : {-pz, pz})
                box(s, M, {x, 0.06f, z}, {0.36f, 0.12f, 0.36f});

        // Horizontal cross-bars — sides only (front/back omitted so interior stays open)
        matte(s, DBLUE);
        for (float barY : {CTY + 0.08f, AWY * 0.62f}) {
            // left & right side rails only
            box(s, M, {-px, barY, 0}, {0.12f, 0.12f, CD + 0.30f});
            box(s, M, { px, barY, 0}, {0.12f, 0.12f, CD + 0.30f});
        }
    }

    // ═══════════════════════════════════════════════════════════
    // 2. BODY
    // ═══════════════════════════════════════════════════════════
    void drawBody(Shader& s, glm::mat4 M, unsigned int panelTex) {
        // Main pink body
        matte(s, PINK);
        box(s, M, {0, CH/2.0f, 0}, {CW, CH, CD});

        // Decorative horizontal trim band
        matte(s, DPINK);
        box(s, M, {0, CH*0.55f, 0}, {CW + 0.02f, 0.12f, CD + 0.02f});

        // Corner rounded looks — vertical strips
        matte(s, DPINK);
        for (float x : {-(CW/2.0f - 0.12f), CW/2.0f - 0.12f})
            box(s, M, {x, CH/2.0f, CD/2.0f + 0.01f}, {0.20f, CH, 0.02f});

        // Textured front & back panels
        if (panelTex) {
            s.setBool("useTexture", true);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, panelTex);
            s.setVec3("objectColor", {1,1,1});
            s.setVec3("emission",    {0,0,0});
            box(s, M, {0, CH/2.0f,  CD/2.0f + 0.016f}, {CW - 0.42f, CH - 0.06f, 0.02f});
            box(s, M, {0, CH/2.0f, -CD/2.0f - 0.016f}, {CW - 0.42f, CH - 0.06f, 0.02f});
            s.setBool("useTexture", false);
        }
    }

    // ═══════════════════════════════════════════════════════════
    // 3. COUNTER
    // ═══════════════════════════════════════════════════════════
    void drawCounter(Shader& s, glm::mat4 M) {
        // Blue counter slab
        matte(s, BLUE);
        box(s, M, {0, CTY - 0.08f, 0}, {CW + 0.40f, 0.18f, CD + 0.40f});
        // White top surface
        matte(s, {0.96f, 0.96f, 0.94f});
        box(s, M, {0, CTY + 0.01f, 0}, {CW + 0.36f, 0.04f, CD + 0.36f});
        // Blue edge lip
        matte(s, DBLUE);
        box(s, M, {0, CTY - 0.18f, 0}, {CW + 0.44f, 0.06f, CD + 0.44f});
    }

    // ═══════════════════════════════════════════════════════════
    // 4. AWNING — flat striped roof + valance flush to edge
    // ═══════════════════════════════════════════════════════════
    void drawAwning(Shader& s, glm::mat4 M) {
        float px = CW/2.0f + 0.08f;
        float pz = CD/2.0f + 0.08f;
        float aw = px * 2.0f + 0.50f;   // awning width (X)
        float ad = pz * 2.0f + 0.50f;   // awning depth (Z)

        // ── flat striped roof — exactly between the four pillar tops ──
        int   nStripe = 20;
        float sw = aw / nStripe;
        for (int i = 0; i < nStripe; ++i) {
            matte(s, i % 2 == 0 ? PINK : CREAM);
            box(s, M, {-aw/2.0f + sw/2.0f + i*sw, AWY, 0}, {sw, 0.14f, ad});
        }

        // ── top ridge cap (sits on roof) ──
        matte(s, DBLUE);
        box(s, M, {0, AWY + 0.12f, 0}, {aw + 0.12f, 0.12f, ad + 0.12f});

        // ── valance parameters ──
        // aBot = exact bottom surface of the awning roof
        float aBot = AWY - 0.07f;
        float vH   = 0.65f;   // how far the valance hangs down
        int   nVal = 10;
        float vw   = aw / nVal;
        float fz   = ad / 2.0f;   // front/back edge of awning (no overhang)

        // ── FRONT valance — top flush with awning bottom edge ──
        for (int i = 0; i < nVal; ++i) {
            float vx = -aw/2.0f + vw/2.0f + i * vw;
            // box center: top at aBot, hanging straight down
            matte(s, i % 2 == 0 ? PINK : CREAM);
            box(s, M, {vx, aBot - vH*0.5f, fz + 0.03f}, {vw*0.92f, vH, 0.07f});
            // scallop bump at bottom
            matte(s, i%2==0 ? DPINK : glm::vec3(0.85f,0.80f,0.62f));
            sp(s, M, {vx, aBot - vH - 0.07f, fz + 0.03f}, {vw*0.46f, 0.24f, 0.20f});
        }

        // ── BACK valance ──
        for (int i = 0; i < nVal; ++i) {
            float vx = -aw/2.0f + vw/2.0f + i * vw;
            matte(s, i % 2 == 0 ? PINK : CREAM);
            box(s, M, {vx, aBot - vH*0.5f, -fz - 0.03f}, {vw*0.92f, vH, 0.07f});
            matte(s, i%2==0 ? DPINK : glm::vec3(0.85f,0.80f,0.62f));
            sp(s, M, {vx, aBot - vH - 0.07f, -fz - 0.03f}, {vw*0.46f, 0.24f, 0.20f});
        }

        // ── SIDE valances ──
        int   nVs = 6;
        float vsw = ad / nVs;
        float exL = -(aw/2.0f + 0.03f);
        float exR =  (aw/2.0f + 0.03f);
        for (int i = 0; i < nVs; ++i) {
            float vz = -ad/2.0f + vsw/2.0f + i * vsw;
            for (float ex : {exL, exR}) {
                matte(s, i % 2 == 0 ? PINK : CREAM);
                box(s, M, {ex, aBot - vH*0.5f, vz}, {0.07f, vH, vsw*0.92f});
                matte(s, i%2==0 ? DPINK : glm::vec3(0.85f,0.80f,0.62f));
                sp(s, M, {ex, aBot - vH - 0.07f, vz}, {0.20f, 0.24f, vsw*0.46f});
            }
        }
    }

    // ═══════════════════════════════════════════════════════════
    // 5. CLOUD SIGN — attached to top ridge bar
    // ═══════════════════════════════════════════════════════════
    void drawSign(Shader& s, glm::mat4 M, unsigned int signTex) {
        float barTop = AWY + 0.18f;
        float signCY = barTop + 0.65f;

        // connector post
        matte(s, DBLUE);
        box(s, M, {0, barTop + 0.18f, 0}, {0.14f, 0.36f, 0.14f});

        // Blue cloud back
        matte(s, BLUE);
        struct { float x, dy, rx, ry; } b[] = {
            {-1.05f, 0.00f, 0.62f, 0.55f},
            {-0.50f, 0.25f, 0.70f, 0.65f},
            { 0.00f, 0.38f, 0.80f, 0.72f},
            { 0.50f, 0.25f, 0.70f, 0.65f},
            { 1.05f, 0.00f, 0.62f, 0.55f},
            { 0.00f,-0.18f, 1.38f, 0.42f},
        };
        for (auto& bb : b)
            sp(s, M, {bb.x, signCY + bb.dy, -0.08f}, {bb.rx, bb.ry, 0.24f});

        // Pink inner panel
        matte(s, PINK);
        sp(s, M, {0, signCY + 0.08f, 0.08f}, {1.24f, 0.74f, 0.18f});

        // Sign texture
        if (signTex) {
            s.setBool("useTexture", true);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, signTex);
            s.setVec3("objectColor", {1,1,1});
            s.setVec3("emission", {0,0,0});
            box(s, M, {0, signCY + 0.10f, 0.20f}, {2.12f, 1.10f, 0.01f});
            s.setBool("useTexture", false);
        }
    }

    // ═══════════════════════════════════════════════════════════
    // 6. WHEEL — both sides, realistic spoked wheel
    // ═══════════════════════════════════════════════════════════
    void drawWheel(Shader& s, glm::mat4 M) {
        float wr = 1.05f;
        float wx = -(CW/2.0f + 0.10f);

        for (float wz : {CD/2.0f + 0.14f, -(CD/2.0f + 0.14f)}) {
            glm::vec3 wc = {wx, wr, wz};

            // Tire (dark grey rubber)
            matte(s, {0.25f, 0.25f, 0.26f});
            for (int i = 0; i < 24; ++i) {
                float a  = glm::radians(i * 15.0f);
                float py = wc.y + wr * sinf(a);
                float pz = wc.z + wr * cosf(a);
                glm::mat4 seg = glm::scale(
                    glm::rotate(glm::translate(M, {wc.x, py, pz}), a, {1,0,0}),
                    {0.18f, 0.30f, 0.18f});
                s.setMat4("model", seg);
                glBindVertexArray(cubeVAO); glDrawArrays(GL_TRIANGLES,0,36);
            }
            // Rim (bright blue)
            metal(s, BLUE, 60.0f);
            for (int i = 0; i < 24; ++i) {
                float a  = glm::radians(i * 15.0f);
                float py = wc.y + (wr-0.13f) * sinf(a);
                float pz = wc.z + (wr-0.13f) * cosf(a);
                glm::mat4 seg = glm::scale(
                    glm::rotate(glm::translate(M, {wc.x, py, pz}), a, {1,0,0}),
                    {0.12f, 0.22f, 0.12f});
                s.setMat4("model", seg);
                glBindVertexArray(cubeVAO); glDrawArrays(GL_TRIANGLES,0,36);
            }
            // Spokes (golden-cream, 8 of them)
            matte(s, {0.88f, 0.78f, 0.38f});
            for (int i = 0; i < 8; ++i) {
                float a = glm::radians(i * 45.0f);
                glm::mat4 sp2 = glm::scale(
                    glm::rotate(glm::translate(M, {wc.x, wc.y, wc.z}), a, {1,0,0}),
                    {0.055f, (wr - 0.12f) * 1.96f, 0.055f});
                s.setMat4("model", sp2);
                glBindVertexArray(cubeVAO); glDrawArrays(GL_TRIANGLES,0,36);
            }
            // Hub cap (pink disc)
            matte(s, PINK);
            cy(s, M, wc, {0.20f, 0.12f, 0.20f});
            metal(s, SILVER, 96.0f);
            sp(s, M, wc, {0.14f, 0.14f, 0.14f});
        }
    }

    // ═══════════════════════════════════════════════════════════
    // 7. BACK LEGS (right side stabilisers)
    // ═══════════════════════════════════════════════════════════
    void drawBackLegs(Shader& s, glm::mat4 M) {
        metal(s, BLUE, 40.0f);
        float lx = CW/2.0f - 0.14f;
        for (float z : {-CD/3.5f, CD/3.5f}) {
            box(s, M, {lx, 0.34f, z}, {0.20f, 0.68f, 0.20f});
            // foot plate
            matte(s, DBLUE);
            box(s, M, {lx, 0.06f, z}, {0.30f, 0.12f, 0.30f});
        }
    }

    // ═══════════════════════════════════════════════════════════
    // 8. HANDLE
    // ═══════════════════════════════════════════════════════════
    void drawHandle(Shader& s, glm::mat4 M) {
        float hx = -(CW/2.0f + 0.55f);
        metal(s, BLUE, 50.0f);
        // Horizontal arms
        for (float z : {-0.55f, 0.55f}) {
            box(s, M, {hx + 0.22f, CTY + 0.05f, z}, {0.70f, 0.10f, 0.10f});
            // Vertical uprights
            box(s, M, {hx - 0.10f, CTY + 0.55f, z}, {0.10f, 1.00f, 0.10f});
        }
        // Cross bar connecting the two uprights
        box(s, M, {hx - 0.10f, CTY + 1.05f, 0}, {0.10f, 0.10f, 1.10f + 0.10f});
        // Handle grip (pink)
        matte(s, PINK);
        cy(s, M, {hx - 0.10f, CTY + 1.06f, 0}, {0.13f, 0.12f, 0.62f});
        // End knobs
        sp(s, M, {hx - 0.10f, CTY + 1.06f, -0.55f}, {0.14f, 0.14f, 0.14f});
        sp(s, M, {hx - 0.10f, CTY + 1.06f,  0.55f}, {0.14f, 0.14f, 0.14f});
    }

    // ═══════════════════════════════════════════════════════════
    // 9. MACHINE — cotton candy spinner, realistic
    // ═══════════════════════════════════════════════════════════
    void drawMachine(Shader& s, glm::mat4 M) {
        float mx  = 0.10f;
        float mz  = CD/2.0f - 0.52f;
        float topY = CTY + 0.06f;

        // ── pink machine body ──
        matte(s, PINK);
        box(s, M, {mx, topY + 0.28f, mz}, {1.60f, 0.56f, 1.60f});

        // ── front face panel (darker) ──
        matte(s, DPINK);
        box(s, M, {mx, topY + 0.28f, mz + 0.81f}, {1.60f, 0.56f, 0.02f});

        // ── blue base strip ──
        matte(s, BLUE);
        box(s, M, {mx, topY + 0.04f, mz}, {1.54f, 0.08f, 1.54f});

        // ── blue collar (fills gap cabinet top → bowl bottom) ──
        matte(s, BLUE);
        cy(s, M, {mx, topY + 0.56f, mz}, {1.58f, 0.22f, 1.58f});

        // ── outer silver bowl wall ──
        metal(s, SILVER, 90.0f);
        cy(s, M, {mx, topY + 0.78f, mz}, {1.58f, 0.42f, 1.58f});

        // ── inner bowl rim (darker ring) ──
        metal(s, {0.58f, 0.60f, 0.65f}, 60.0f);
        cy(s, M, {mx, topY + 0.98f, mz}, {1.30f, 0.08f, 1.30f});

        // ── bowl floor disc ──
        metal(s, {0.68f, 0.70f, 0.74f}, 40.0f);
        {
            glm::mat4 d = glm::scale(glm::translate(M, {mx, topY + 0.64f, mz}),
                                     {1.28f, 0.05f, 1.28f});
            s.setMat4("model", d); cyl.draw();
        }

        // ── bowl top rim ring ──
        metal(s, {0.65f, 0.67f, 0.72f}, 60.0f);
        cy(s, M, {mx, topY + 1.02f, mz}, {1.60f, 0.06f, 1.60f});

        // ── spinning centre spindle ──
        matte(s, PINK);
        cy(s, M, {mx, topY + 0.78f, mz}, {0.18f, 0.36f, 0.18f});
        metal(s, SILVER, 120.0f);
        sp(s, M, {mx, topY + 1.00f, mz}, {0.16f, 0.13f, 0.16f});

        // ── front panel controls ──
        // Left dial (blue knob)
        matte(s, BLUE);
        sp(s, M, {mx - 0.38f, topY + 0.28f, mz + 0.82f}, {0.095f, 0.095f, 0.06f});
        matte(s, {0.90f,0.90f,0.90f});
        box(s, M, {mx - 0.38f, topY + 0.33f, mz + 0.85f}, {0.016f, 0.055f, 0.016f});

        // Power button (red)
        matte(s, {0.90f, 0.12f, 0.16f});
        sp(s, M, {mx + 0.02f, topY + 0.28f, mz + 0.83f}, {0.075f, 0.075f, 0.05f});
        matte(s, {0.40f, 0.08f, 0.10f});
        cy(s, M, {mx + 0.02f, topY + 0.28f, mz + 0.82f}, {0.115f, 0.030f, 0.115f});

        // Right speed dial (blue)
        matte(s, BLUE);
        sp(s, M, {mx + 0.40f, topY + 0.28f, mz + 0.82f}, {0.085f, 0.085f, 0.05f});

        // LED row
        glm::vec3 ledC[] = {{0.12f,0.88f,0.28f},{0.90f,0.78f,0.08f},{0.88f,0.18f,0.18f}};
        for (int i = 0; i < 3; ++i) {
            matte(s, ledC[i]);
            s.setVec3("emission", ledC[i] * 0.55f);
            sp(s, M, {mx - 0.18f + i*0.18f, topY + 0.44f, mz + 0.83f},
               {0.038f, 0.038f, 0.025f});
        }
        s.setVec3("emission", {0,0,0});
        s.setVec3("lightSpecular", {0,0,0});
    }

    // ═══════════════════════════════════════════════════════════
    // 10. COTTON CANDY — 5 fluffy cloud clusters on sticks
    // ═══════════════════════════════════════════════════════════
    void drawCandies(Shader& s, glm::mat4 M, float time,
                     unsigned int tex1, unsigned int tex2, unsigned int tex3) {
        struct CC { float x, stickH; glm::vec3 col; };
        CC c[] = {
            {-1.80f, 1.10f, {0.98f, 0.94f, 0.42f}},  // yellow (outer)
            {-0.90f, 1.40f, {0.98f, 0.48f, 0.68f}},  // pink
            { 0.00f, 1.60f, {0.70f, 0.52f, 0.94f}},  // purple (center, tallest)
            { 0.90f, 1.40f, {0.45f, 0.78f, 0.96f}},  // blue
            { 1.80f, 1.10f, {0.98f, 0.68f, 0.76f}},  // peach (outer)
        };
        unsigned int texArr[5] = {tex1, tex2, tex3, tex1, tex2};
        float bz = 0.0f;  // centre of counter depth

        for (int i = 0; i < 5; ++i) {
            float sw = sinf(time * 1.6f + i * 0.75f) * 0.030f;
            float tx = c[i].x + sw;
            float sh = c[i].stickH;
            unsigned int tex = texArr[i];

            // Stick
            matte(s, {0.85f, 0.76f, 0.58f});
            box(s, M, {tx, CTY + sh * 0.5f, bz}, {0.032f, sh, 0.032f});

            // Cotton cloud — textured SphereWithTexture
            s.setBool("useTexture", tex ? true : false);
            if (tex) {
                s.setInt("texture_diffuse", 0);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, tex);
            }
            s.setVec3("objectColor", c[i].col);
            s.setVec3("emission",    c[i].col * 0.05f);

            float cy0 = CTY + sh + 0.35f;

            tsp(s, M, {tx,        cy0,        bz       }, {0.52f, 0.62f, 0.48f}); // centre
            tsp(s, M, {tx+0.30f,  cy0-0.05f,  bz       }, {0.36f, 0.44f, 0.32f}); // L lobe
            tsp(s, M, {tx-0.30f,  cy0-0.05f,  bz       }, {0.36f, 0.44f, 0.32f}); // R lobe
            tsp(s, M, {tx,        cy0+0.40f,  bz       }, {0.32f, 0.36f, 0.30f}); // top
            tsp(s, M, {tx+0.18f,  cy0+0.30f,  bz       }, {0.28f, 0.32f, 0.26f});
            tsp(s, M, {tx-0.18f,  cy0+0.30f,  bz       }, {0.28f, 0.32f, 0.26f});
            tsp(s, M, {tx+0.38f,  cy0+0.16f,  bz       }, {0.24f, 0.28f, 0.22f});
            tsp(s, M, {tx-0.38f,  cy0+0.16f,  bz       }, {0.24f, 0.28f, 0.22f});
            tsp(s, M, {tx,        cy0-0.34f,  bz       }, {0.26f, 0.30f, 0.24f}); // lower taper
            tsp(s, M, {tx+0.14f,  cy0-0.24f,  bz       }, {0.20f, 0.24f, 0.18f});
            tsp(s, M, {tx-0.14f,  cy0-0.24f,  bz       }, {0.20f, 0.24f, 0.18f});
            tsp(s, M, {tx,        cy0+0.10f,  bz+0.22f }, {0.28f, 0.32f, 0.26f}); // depth
            tsp(s, M, {tx,        cy0+0.10f,  bz-0.22f }, {0.28f, 0.32f, 0.26f});

            s.setBool("useTexture", false);
            s.setVec3("emission", {0,0,0});
        }
    }

    // ═══════════════════════════════════════════════════════════
    // 11. JAR + STICK HOLDER
    // ═══════════════════════════════════════════════════════════
    void drawJar(Shader& s, glm::mat4 M) {
        // Sugar jar — far left of counter
        // cy() places the cylinder centre at the given Y, height = scale.y
        // so bottom = Y - scale.y/2.  We want bottom = CTY + 0.05 (on counter top)
        float jx   = -(CW/2.0f - 0.55f);
        float jz   =  (CD/2.0f - 0.38f);
        float base = CTY + 0.05f;   // counter surface

        float jarH = 0.72f;
        float jarR = 0.36f;
        float jarCY = base + jarH / 2.0f;   // centre Y so bottom = base

        // Glass body
        matte(s, {0.82f, 0.88f, 0.96f});
        cy(s, M, {jx, jarCY, jz}, {jarR, jarH, jarR});
        // Pink sugar fill (lower half)
        matte(s, PINK);
        cy(s, M, {jx, base + 0.22f, jz}, {0.28f, 0.44f, 0.28f});
        // Blue lid (sits on top)
        float jarTop = base + jarH;
        matte(s, BLUE);
        cy(s, M, {jx, jarTop + 0.03f, jz}, {jarR + 0.02f, 0.06f, jarR + 0.02f});
        cy(s, M, {jx, jarTop + 0.09f, jz}, {jarR - 0.02f, 0.06f, jarR - 0.02f});

        // Stick holder — right of jar, same base
        float hx = jx + 0.58f;
        float cupH = 0.32f;
        float cupCY = base + cupH / 2.0f;
        matte(s, PINK);
        cy(s, M, {hx, cupCY, jz}, {0.24f, cupH, 0.24f});
        matte(s, DPINK);
        cy(s, M, {hx, base + cupH, jz}, {0.26f, 0.04f, 0.26f});
        // wooden sticks
        matte(s, {0.85f, 0.78f, 0.58f});
        for (int i = 0; i < 8; ++i) {
            float ang = i * 45.0f;
            float ox  = 0.07f * cosf(glm::radians(ang));
            float oz  = 0.07f * sinf(glm::radians(ang));
            float stickCY = base + cupH + 0.17f;
            box(s, M, {hx+ox, stickCY, jz+oz}, {0.020f, 0.34f, 0.020f});
        }
        box(s, M, {hx, base + cupH + 0.19f, jz}, {0.020f, 0.38f, 0.020f});
    }

    // ═══════════════════════════════════════════════════════════
    // 12. SMALL "SWEET & FLUFFY" SIGN
    // ═══════════════════════════════════════════════════════════
    void drawSmallSign(Shader& s, glm::mat4 M, unsigned int cstandTex = 0) {
        float base = CTY + 0.05f;
        float sx = CW/2.0f - 0.62f;
        float sz = CD/2.0f - 0.30f;

        // Pink board frame
        matte(s, PINK);
        box(s, M, {sx, base + 0.40f, sz}, {0.90f, 0.65f, 0.06f});

        if (cstandTex) {
            // textured face
            s.setBool("useTexture", true);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, cstandTex);
            s.setVec3("objectColor", {1,1,1});
            s.setVec3("emission", {0.04f,0.0f,0.02f});
            box(s, M, {sx, base + 0.40f, sz + 0.04f}, {0.82f, 0.57f, 0.01f});
            s.setBool("useTexture", false);
            s.setVec3("emission", {0,0,0});
        } else {
            matte(s, {1,1,1});
            box(s, M, {sx, base + 0.40f, sz + 0.04f}, {0.80f, 0.54f, 0.01f});
            matte(s, DPINK);
            for (int i = 0; i < 3; ++i)
                box(s, M, {sx, base + 0.54f - i*0.18f, sz + 0.05f}, {0.58f, 0.08f, 0.01f});
        }

        // A-frame legs — bottom sits on counter
        matte(s, PINK);
        box(s, M, {sx - 0.09f, base + 0.10f, sz - 0.20f}, {0.045f, 0.20f, 0.42f});
        box(s, M, {sx + 0.09f, base + 0.10f, sz - 0.20f}, {0.045f, 0.20f, 0.42f});

        restoreSpec(s);
    }
};

#endif
