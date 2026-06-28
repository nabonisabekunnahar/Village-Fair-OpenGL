#ifndef ICE_CREAM_CORNER_H
#define ICE_CREAM_CORNER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include "shader.h"
#include "cylinder.h"
#include "sphere.h"
#include "pyramid.h"
#include "sphereWithTexture.h"

class BezierSurface {
public:
    BezierSurface(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3,
                  int lat = 20, int lon = 36) {
        setupMesh(p0, p1, p2, p3, lat, lon);
    }
    ~BezierSurface() {
        if (VAO) glDeleteVertexArrays(1, &VAO);
        if (VBO) glDeleteBuffers(1, &VBO);
        if (EBO) glDeleteBuffers(1, &EBO);
    }
    void draw() {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
private:
    unsigned int VAO=0, VBO=0, EBO=0;
    int indicesCount=0;
    void setupMesh(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3,
                   int latSeg, int lonSeg) {
        std::vector<float> verts;
        std::vector<unsigned int> idx;
        for (int i = 0; i <= latSeg; ++i) {
            float t=float(i)/latSeg, it=1-t;
            glm::vec2 p = it*it*it*p0 + 3*it*it*t*p1 + 3*it*t*t*p2 + t*t*t*p3;
            for (int j = 0; j <= lonSeg; ++j) {
                float phi = glm::two_pi<float>()*j/lonSeg;
                float x=p.x*cosf(phi), z=p.x*sinf(phi), y=p.y;
                verts.push_back(x); verts.push_back(y); verts.push_back(z);
                glm::vec3 n = glm::normalize(glm::vec3(x,0.4f,z));
                verts.push_back(n.x); verts.push_back(n.y); verts.push_back(n.z);
                verts.push_back(float(j)/lonSeg); verts.push_back(t);
            }
        }
        for (int i=0;i<latSeg;++i) for (int j=0;j<lonSeg;++j) {
            int f=i*(lonSeg+1)+j, s=f+lonSeg+1;
            idx.push_back(f); idx.push_back(s);   idx.push_back(f+1);
            idx.push_back(s); idx.push_back(s+1); idx.push_back(f+1);
        }
        indicesCount=(int)idx.size();
        glGenVertexArrays(1,&VAO); glGenBuffers(1,&VBO); glGenBuffers(1,&EBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER,VBO);
        glBufferData(GL_ARRAY_BUFFER,verts.size()*sizeof(float),verts.data(),GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,idx.size()*sizeof(unsigned int),idx.data(),GL_STATIC_DRAW);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)0);               glEnableVertexAttribArray(0);
        glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(3*sizeof(float)));glEnableVertexAttribArray(1);
        glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(6*sizeof(float)));glEnableVertexAttribArray(2);
    }
};

class IceCreamCorner {
public:
    IceCreamCorner(unsigned int cubeVAO)
        : cubeVAO(cubeVAO), cyl(24), sph(0.5f,24,16), pyr(),
          // curvy umbrella: centre high, edges droop below the rim (real canopy shape)
          umbrellaCanopy(glm::vec2(0, 0.75f), glm::vec2(1.0f, 0.65f),
                         glm::vec2(1.85f, -0.15f), glm::vec2(2.1f, -0.45f)),
          popcornSph(0.5f, 20, 10, {1,1,1}, {1,1,1}, {1,1,1}, 32.0f, 0, 0, 0,0, 1,1)
    {}

    void draw(Shader& shader, glm::vec3 pos, float rotY,
              unsigned int stripeTex, unsigned int woodTex, unsigned int popcornTex,
              unsigned int wood2Tex, unsigned int burgerTex, unsigned int redWhiteTex,
              unsigned int ice1Tex, unsigned int ice2Tex, unsigned int ice3Tex, unsigned int iceSignTex,
              const std::vector<unsigned int>& scoopTexs) {
        glm::mat4 base = glm::translate(glm::mat4(1.0f), pos);
        base = glm::rotate(base, glm::radians(rotY), glm::vec3(0,1,0));

        drawBooth(shader, base, woodTex, stripeTex, ice1Tex, ice2Tex, ice3Tex, iceSignTex, scoopTexs);
        drawLeftSawhorseTable(shader, base, wood2Tex, scoopTexs); // Use wood2Tex as requested
        drawBurgerDomeStand(shader, base, wood2Tex, burgerTex);
        drawPopcornBucketStand(shader, base, stripeTex, popcornTex);
        drawSideUmbrella(shader, base, redWhiteTex);
        
        // Add two chairs in front of the right-side table
        drawChair(shader, base, {11.0f, 0.0f, 1.2f}, 180.0f, wood2Tex); // Chair 1
        drawChair(shader, base, {13.0f, 0.0f, 1.2f}, 180.0f, wood2Tex); // Chair 2
    }

private:
    unsigned int cubeVAO;
    Cylinder cyl;
    Sphere   sph;
    Pyramid  pyr;
    BezierSurface umbrellaCanopy;
    SphereWithTexture popcornSph;

    // ── helpers ──────────────────────────────────────────────────────
    void cube(Shader& s, glm::mat4 base, glm::vec3 p, glm::vec3 sz) {
        glm::mat4 m = glm::scale(glm::translate(base, p), sz);
        s.setMat4("model", m);
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    void useWood(Shader& s, unsigned int tex) {
        s.setBool("useTexture", true);
        s.setInt("texture_diffuse", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);
        s.setVec3("objectColor", glm::vec3(1.0f));
        s.setVec3("emission", glm::vec3(0.0f));
    }
    void flat(Shader& s, glm::vec3 col, glm::vec3 em = glm::vec3(0.0f)) {
        s.setBool("useTexture", false);
        s.setVec3("objectColor", col);
        s.setVec3("emission", em);
    }

    void drawChair(Shader& shader, glm::mat4 base, glm::vec3 pos, float rotY, unsigned int woodTex) {
        glm::mat4 m = glm::translate(base, pos);
        m = glm::rotate(m, glm::radians(rotY), {0, 1, 0});
        
        // 4 Legs
        flat(shader, {0.35f, 0.20f, 0.10f});
        float legH = 0.75f;  // Increased slightly
        float legW = 0.08f;  // Increased slightly
        float seatW = 0.85f; // Increased slightly
        for (float sx : {-seatW/2 + legW, seatW/2 - legW}) {
            for (float sz : {-seatW/2 + legW, seatW/2 - legW}) {
                cube(shader, m, {sx, legH/2, sz}, {legW, legH, legW});
            }
        }
        
        // Seat
        useWood(shader, woodTex);
        cube(shader, m, {0, legH, 0}, {seatW, 0.08f, seatW});
        
        // Backrest poles
        flat(shader, {0.35f, 0.20f, 0.10f});
        float backH = 0.95f; // Increased slightly
        for (float sx : {-seatW/2 + legW, seatW/2 - legW}) {
            cube(shader, m, {sx, legH + backH/2, -seatW/2 + legW}, {legW, backH, legW});
        }
        
        // Backrest board
        useWood(shader, woodTex);
        cube(shader, m, {0, legH + backH - 0.18f, -seatW/2 + legW}, {seatW - 0.1f, 0.32f, 0.06f});
    }

    // ═══════════════════════════════════════════════════════════════
    // 1. MAIN BOOTH — Ice Cream Shop (matches reference image)
    // ═══════════════════════════════════════════════════════════════
    void drawBooth(Shader& shader, glm::mat4 base, unsigned int woodTex, unsigned int stripeTex,
                   unsigned int ice1Tex, unsigned int ice2Tex, unsigned int ice3Tex, unsigned int iceSignTex,
                   const std::vector<unsigned int>& scoopTexs) {
        shader.use();
        const float W=7.0f, D=2.0f, CH=1.6f, WH=4.2f;
        const float fz=D*0.5f, bz=-D*0.5f;

        // ── 1. DARK BROWN FRAME ──
        flat(shader, {0.22f,0.10f,0.04f});
        cube(shader, base, {-W*0.5f+0.15f, WH*0.5f, 0}, {0.30f, WH, D+0.1f});
        cube(shader, base, { W*0.5f-0.15f, WH*0.5f, 0}, {0.30f, WH, D+0.1f});
        cube(shader, base, {0, WH-0.15f, 0}, {W+0.1f, 0.30f, D+0.1f});
        cube(shader, base, {0, 0.08f, 0}, {W+0.1f, 0.16f, D+0.1f});

        // ── 2. BACK WALL (dark brown) ──
        flat(shader, {0.28f,0.13f,0.05f});
        cube(shader, base, {0, WH*0.5f, bz+0.09f}, {W-0.30f, WH, 0.18f});

        // ── 3. COUNTER (orange-yellow with 3 white-bordered panels) ──
        flat(shader, {0.92f, 0.62f, 0.12f});
        cube(shader, base, {0, CH*0.5f, 0}, {W-0.32f, CH, D-0.1f});
        float panelW = (W-0.32f)/3.0f - 0.05f;
        float panelXs[] = {-(W-0.32f)/3.0f, 0.0f, (W-0.32f)/3.0f};
        unsigned int pTex[] = {ice1Tex, ice2Tex, ice3Tex};
        for (int i=0; i<3; ++i) {
            flat(shader, {0.98f, 0.98f, 0.98f});
            cube(shader, base, {panelXs[i], CH*0.5f, fz-0.01f}, {panelW+0.04f, CH-0.06f, 0.03f});
            
            shader.setBool("useTexture", true);
            shader.setInt("texture_diffuse", 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, pTex[i]);
            shader.setVec3("objectColor", glm::vec3(1.0f));
            shader.setVec3("emission", glm::vec3(0.08f)); // soft glow
            
            cube(shader, base, {panelXs[i], CH*0.5f, fz+0.011f}, {panelW-0.04f, CH-0.14f, 0.03f});
            
            shader.setVec3("emission", glm::vec3(0.0f));
            shader.setBool("useTexture", false);
        }
        flat(shader, {0.22f,0.10f,0.04f});
        cube(shader, base, {0, CH+0.06f, 0}, {W-0.28f, 0.12f, D+0.06f});

        // ── 4. DISPLAY CASE ──
        // Back metallic wall of case 
        flat(shader, {0.55f, 0.60f, 0.65f});
        cube(shader, base, {0, CH+0.42f, bz+0.52f}, {W-0.35f, 0.55f, 0.10f});
        // Glass panel — thinner and darker (less dominant)
        flat(shader, {0.65f, 0.75f, 0.82f});
        glm::mat4 glass = glm::translate(base, {0.0f, CH+0.32f, fz-0.30f});
        glass = glm::rotate(glass, glm::radians(32.0f), {1,0,0});
        glass = glm::scale(glass, {W-0.35f, 0.025f, 0.65f});
        shader.setMat4("model", glass);
        glBindVertexArray(cubeVAO); glDrawArrays(GL_TRIANGLES,0,36);
        // Case bottom tray
        flat(shader, {0.45f, 0.50f, 0.55f});
        cube(shader, base, {0, CH+0.12f, bz+0.58f}, {W-0.35f, 0.07f, 0.72f});
        // Ice cream tubs 
        glm::vec3 iceCreamCols[] = {
            {0.98f,0.72f,0.78f},{0.45f,0.22f,0.12f},{0.98f,0.92f,0.60f},
            {0.62f,0.88f,0.62f},{0.72f,0.48f,0.92f},{0.98f,0.78f,0.72f}
        };
        for (int i=0; i<6; ++i) {
            float tx = -W*0.5f + 0.60f + i*(W-0.8f)/6.0f;
            flat(shader, iceCreamCols[i]);
            glm::mat4 tub = glm::translate(base, {tx, CH+0.20f, bz+0.62f});
            tub = glm::scale(tub, {0.68f, 0.14f, 0.50f});
            shader.setMat4("model", tub);
            glBindVertexArray(cubeVAO); glDrawArrays(GL_TRIANGLES,0,36);
        }

        // ── 5. SHELF (one visible shelf on back wall) ──
        flat(shader, {0.55f, 0.35f, 0.12f});
        // Shelf bracket supports
        cube(shader, base, {0, CH+1.68f, bz+0.32f}, {W-0.45f, 0.07f, 0.45f});

        // ── 6. ARCH SIGN — big prominent arch like reference ──
        // Pulled forward (bz+0.28) and made very large
        float archYOffset = 0.80f;

        // Support pillars to connect the roof to the lifted arch
        flat(shader, {0.22f,0.10f,0.04f});
        cube(shader, base, {-2.0f, WH + 0.5f, bz+0.28f}, {0.20f, 1.0f, 0.20f});
        cube(shader, base, { 2.0f, WH + 0.5f, bz+0.28f}, {0.20f, 1.0f, 0.20f});

        // Outer golden frame base
        flat(shader, {0.22f,0.10f,0.04f});
        cube(shader, base, {0, WH+0.20f + archYOffset, bz+0.28f}, {5.2f, 0.26f, 0.25f}); // base bar
        // Left/right sides of arch frame 
        cube(shader, base, {-2.3f, WH+0.80f + archYOffset, bz+0.28f}, {0.26f, 1.20f, 0.25f});
        cube(shader, base, { 2.3f, WH+0.80f + archYOffset, bz+0.28f}, {0.26f, 1.20f, 0.25f});
        // Top arc (stepped blocks to simulate arch)
        cube(shader, base, {0, WH+1.42f + archYOffset, bz+0.29f},  {4.6f, 0.26f, 0.23f}); // wide
        cube(shader, base, {0, WH+1.70f + archYOffset, bz+0.28f},  {3.8f, 0.26f, 0.22f}); // middle
        cube(shader, base, {0, WH+1.96f + archYOffset, bz+0.27f},  {2.8f, 0.28f, 0.21f}); // narrow
        cube(shader, base, {0, WH+2.22f + archYOffset, bz+0.26f},  {1.6f, 0.28f, 0.20f}); // top
        // Golden fill (inside the arch frame)
        flat(shader, {0.95f, 0.75f, 0.15f});
        cube(shader, base, {0, WH+0.92f + archYOffset, bz+0.33f}, {4.52f, 1.42f, 0.16f});
        
        // Sign Board (using ice_sign.png texture)
        shader.setBool("useTexture", true);
        shader.setInt("texture_diffuse", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, iceSignTex);
        shader.setVec3("objectColor", glm::vec3(1.0f));
        shader.setVec3("emission", glm::vec3(0.1f)); 
        
        // Single billboard in front of the golden fill
        cube(shader, base, {0, WH+1.12f + archYOffset, bz+0.42f}, {4.0f, 1.6f, 0.04f}); 
        
        shader.setVec3("emission", glm::vec3(0.0f));
        shader.setBool("useTexture", false);

        // ── 7. AWNING ──
        shader.setBool("useTexture", true);
        shader.setInt("texture_diffuse", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, stripeTex);
        shader.setVec3("objectColor", glm::vec3(1.0f));
        shader.setVec3("emission", glm::vec3(0.06f, 0.0f, 0.0f));
        shader.setVec3("lightSpecular", {0.15f, 0.15f, 0.15f});
        shader.setFloat("shininess", 16.0f);
        glm::mat4 awn = glm::translate(base, {0, WH+0.12f, fz*0.35f});
        awn = glm::rotate(awn, glm::radians(-22.0f), {1,0,0});
        awn = glm::scale(awn, {W+0.15f, 0.06f, 3.2f});
        shader.setMat4("model", awn);
        glBindVertexArray(cubeVAO); glDrawArrays(GL_TRIANGLES,0,36);
        shader.setVec3("emission", glm::vec3(0.0f));
        shader.setBool("useTexture", false);
        shader.setVec3("lightSpecular", {1.0f,1.0f,1.0f});
        // Scalloped edge / bunting flags
        // Calculate perfect front lower edge of the rotated awning (-22 degrees means front pitches UP)
        float frontTipY = WH+0.12f + sinf(glm::radians(22.0f))*1.6f; // PLUS for upward pitch
        float frontTipZ = fz*0.35f + cosf(glm::radians(22.0f))*1.6f;
        
        // Let the diamond flags hang down a bit
        float awEdgeY = frontTipY - 0.22f; 
        float awEdgeZ = frontTipZ - 0.02f; 

        // Draw a thin horizontal wire/string attached under the awning lip
        flat(shader, {0.15f, 0.10f, 0.05f}); // Dark brown string
        glm::mat4 wire = glm::translate(base, {0.0f, frontTipY - 0.03f, awEdgeZ});
        wire = glm::scale(wire, {W - 0.2f, 0.015f, 0.015f});
        shader.setMat4("model", wire);
        glBindVertexArray(cubeVAO); glDrawArrays(GL_TRIANGLES,0,36);

        // Draw 2 metal brackets/rods at the two endpoints connecting the wire to the awning
        flat(shader, {0.10f, 0.10f, 0.10f}); // Dark metal
        for (int side : {-1, 1}) {
            float endX = side * (W - 0.2f) * 0.5f;
            glm::mat4 bracket = glm::translate(base, {endX, frontTipY - 0.015f, awEdgeZ});
            bracket = glm::scale(bracket, {0.03f, 0.06f, 0.03f}); // Solid thick connector
            shader.setMat4("model", bracket);
            glBindVertexArray(cubeVAO); glDrawArrays(GL_TRIANGLES,0,36);
        }

        glm::vec3 scCols[]={{0.90f,0.15f,0.15f},{0.98f,0.98f,0.98f}};
        int nScallops = 12;
        for (int i=0; i<nScallops; ++i) {
            float sx = -W*0.5f + 0.30f + i*(W-0.30f)/(nScallops-1);
            
            // Sway animation based on time, with slight phase offset per flag
            float timeSec = (float)glfwGetTime();
            float swayAngle = sinf(timeSec * 2.5f + sx * 1.5f) * 12.0f; // Max +/- 12 degrees
            swayAngle += 8.0f; // Baseline wind blow direction pushing slightly back
            
            // Pivot from the wire
            glm::mat4 pivot = glm::translate(base, {sx, frontTipY - 0.03f, awEdgeZ});
            pivot = glm::rotate(pivot, glm::radians(swayAngle), {1, 0, 0});
            
            // Draw a small vertical string connecting the main wire down to each flag
            flat(shader, {0.15f, 0.10f, 0.05f});
            glm::mat4 string = glm::translate(pivot, {0.0f, -0.09f, 0.0f});
            string = glm::scale(string, {0.012f, 0.18f, 0.012f});
            shader.setMat4("model", string);
            glBindVertexArray(cubeVAO); glDrawArrays(GL_TRIANGLES,0,36);

            // Draw the diamond flag
            flat(shader, scCols[i%2]);
            glm::mat4 sc = glm::translate(pivot, {0.0f, -0.19f, 0.0f});
            sc = glm::rotate(sc, glm::radians(45.0f), {0,0,1});
            sc = glm::scale(sc, {0.26f, 0.26f, 0.04f}); // Thinner so they look like fabric flags
            shader.setMat4("model", sc);
            glBindVertexArray(cubeVAO); glDrawArrays(GL_TRIANGLES,0,36);
        }

        // ── 8. SHELF ITEMS & COUNTER ITEMS ──
        drawShelfItems(shader, base, CH, D);
        drawCounterItems(shader, base, CH, fz, bz, scoopTexs);
    }

    void drawShelfItems(Shader& shader, glm::mat4 base, float CH, float D) {
        float bz = -D*0.5f + 0.28f; // closer to back wall, clearly visible
        float shelfY = CH + 1.75f;   // just above the shelf surface

        // Squeeze bottles: brown sauce, pink strawberry, yellow caramel
        glm::vec3 botCols[] = {{0.40f,0.20f,0.05f},{0.85f,0.35f,0.55f},{0.88f,0.78f,0.10f}};
        for (int i=0; i<3; ++i) {
            float bx = -0.8f + i*0.72f;
            flat(shader, botCols[i]);
            glm::mat4 m = glm::translate(base, {bx, shelfY, bz});
            m = glm::scale(m, {0.14f, 0.45f, 0.14f}); shader.setMat4("model",m); cyl.draw();
            // bottle neck
            flat(shader, botCols[i]*0.8f);
            glm::mat4 neck = glm::translate(base, {bx, shelfY+0.26f, bz});
            neck = glm::scale(neck, {0.07f, 0.14f, 0.07f}); shader.setMat4("model",neck); cyl.draw();
        }

        // Glass jars with candy: red sprinkles, green candy
        for (int i=0; i<2; ++i) {
            float jx = 1.05f + i*0.72f;
            flat(shader, {0.82f,0.90f,0.92f});
            glm::mat4 jar = glm::translate(base, {jx, shelfY, bz});
            jar = glm::scale(jar, {0.20f, 0.32f, 0.20f}); shader.setMat4("model",jar); cyl.draw();
            glm::vec3 candy = (i==0) ? glm::vec3(0.90f,0.20f,0.20f) : glm::vec3(0.20f,0.75f,0.30f);
            flat(shader, candy);
            glm::mat4 fill = glm::translate(base, {jx, shelfY-0.05f, bz});
            fill = glm::scale(fill, {0.15f, 0.22f, 0.15f}); shader.setMat4("model",fill); cyl.draw();
            flat(shader, {0.72f,0.78f,0.80f});
            glm::mat4 lid = glm::translate(base, {jx, shelfY+0.19f, bz});
            lid = glm::scale(lid, {0.22f, 0.05f, 0.22f}); shader.setMat4("model",lid); cyl.draw();
        }

        // Wafer roll jar on right end
        float wrX = 2.20f;
        flat(shader, {0.82f,0.90f,0.92f});
        glm::mat4 wj = glm::translate(base, {wrX, shelfY, bz});
        wj = glm::scale(wj, {0.18f, 0.30f, 0.18f}); shader.setMat4("model",wj); cyl.draw();
        for (int k=-1; k<=1; ++k) {
            flat(shader, {0.82f, 0.55f, 0.22f});
            glm::mat4 stick = glm::translate(base, {wrX + k*0.05f, shelfY+0.28f, bz});
            stick = glm::scale(stick, {0.030f, 0.22f, 0.030f}); shader.setMat4("model",stick); cyl.draw();
        }
    }

    void drawCounterItems(Shader& shader, glm::mat4 base, float CH, float fz, float bz, const std::vector<unsigned int>& scoopTexs) {
        // Cone holder box (far left of counter, like reference)
        float coneX = -W_HALF() + 0.62f;
        flat(shader, {0.78f, 0.55f, 0.18f}); // pale wooden box
        glm::mat4 box = glm::translate(base, {coneX, CH+0.30f, fz-0.22f});
        box = glm::scale(box, {0.60f, 0.44f, 0.35f}); // Made box a bit wider for 3 cones
        shader.setMat4("model",box);
        glBindVertexArray(cubeVAO); glDrawArrays(GL_TRIANGLES,0,36);
        
        // Cones fanned out from box
        glm::vec3 scoopCols[] = {{0.98f,0.72f,0.78f}, {0.98f,0.92f,0.60f}, {0.45f,0.22f,0.12f}}; // Pink, Vanilla, Choco
        for (int ci=0; ci<3; ++ci) {
            float cx = coneX - 0.18f + ci*0.18f; // Spaced evenly: -0.18, 0, 0.18
            float cy = CH+0.55f;
            float cz = fz-0.22f;

            // Draw cone (pointing down into the box)
            flat(shader, {0.82f, 0.58f, 0.24f});
            glm::mat4 cone = glm::translate(base, {cx, cy, cz});
            cone = glm::rotate(cone, glm::radians(180.0f), {1,0,0}); // rotate 180 to point down
            cone = glm::scale(cone, {0.18f, 0.45f, 0.18f}); // width 0.18, height 0.45
            shader.setMat4("model",cone); pyr.draw();

            /* The pyramid points down. 
               Its local apex is at (0,-0.5,0) and its broad base is at (0,0.5,0).
               So the base world Y is cy + 0.5 * scaleY = cy + 0.225 */

            // Draw ice cream scoop exactly on the top broad base of the cone
            shader.setBool("useTexture", true);
            glBindTexture(GL_TEXTURE_2D, scoopTexs[ci % scoopTexs.size()]);
            shader.setVec3("objectColor", {1.0f, 1.0f, 1.0f});
            shader.setVec3("emission", {0.0f, 0.0f, 0.0f}); // no emission for real look
            shader.setVec3("lightSpecular", {0.03f, 0.03f, 0.03f}); // virtually no specularity (matte)
            shader.setFloat("shininess", 2.0f); // very low shininess
            glm::mat4 scoop = glm::translate(base, {cx, cy + 0.22f, cz}); // sit perfectly on top
            scoop = glm::scale(scoop, glm::vec3(0.22f)); // diameter 0.22, slightly larger than cone width (0.18)
            shader.setMat4("model", scoop); 
            glBindVertexArray(popcornSph.sphereTexVAO);
            glDrawElements(GL_TRIANGLES, popcornSph.getIndexCount(), GL_UNSIGNED_INT, 0);
            shader.setBool("useTexture", false);
            shader.setVec3("lightSpecular", {1.0f, 1.0f, 1.0f}); // reset
            shader.setFloat("shininess", 32.0f); // reset
            shader.setVec3("emission", {0, 0, 0});
        }

        // Stacked red cups (between box and display case) — visible stack
        float cupX = -W_HALF() + 1.45f;
        for (int ci=0; ci<6; ++ci) {
            flat(shader, {0.88f, 0.15f, 0.15f});
            glm::mat4 cup = glm::translate(base, {cupX, CH+0.14f + ci*0.10f, fz-0.25f});
            float sc = 0.30f - ci*0.012f;
            cup = glm::scale(cup, {sc, 0.095f, sc}); shader.setMat4("model",cup); cyl.draw();
            // white stripe on each cup
            flat(shader, {0.98f,0.98f,0.98f});
            glm::mat4 stripe = glm::translate(base, {cupX, CH+0.17f + ci*0.10f, fz-0.25f});
            float ss = 0.31f - ci*0.013f;
            stripe = glm::scale(stripe, {ss, 0.018f, ss}); shader.setMat4("model",stripe); cyl.draw();
        }

        // ── Gelato Display Tray (Angled & Large, Right side of counter) ──
        float trayX = 1.15f; // moved further left to extend
        float trayY = CH + 0.62f; // resting solidly near the counter top
        float trayZ = fz - 0.50f; // pushed further back so it can lay flatter
        
        // Angled base block for the display (tilted backward by -50 degrees)
        flat(shader, {0.60f, 0.65f, 0.70f}); // Metallic grey case
        glm::mat4 tb = glm::translate(base, {trayX, trayY, trayZ});
        tb = glm::rotate(tb, glm::radians(-50.0f), {1, 0, 0}); 
        tb = glm::scale(tb, {4.2f, 1.1f, 0.20f}); // Wider for 8 tubs
        shader.setMat4("model", tb);
        glBindVertexArray(cubeVAO); glDrawArrays(GL_TRIANGLES,0,36);
        
        // 8 large tubs in a single horizontal row
        glm::vec3 gelatoCols[8] = {
            {0.88f, 0.22f, 0.35f}, // raspberry red
            {0.92f, 0.45f, 0.15f}, // orange mango
            {0.98f, 0.92f, 0.80f}, // vanilla
            {0.45f, 0.22f, 0.12f}, // milk chocolate
            {0.55f, 0.82f, 0.55f}, // mint green
            {0.48f, 0.35f, 0.65f}, // blueberry purple
            {0.82f, 0.70f, 0.50f}, // coffee caramel
            {0.95f, 0.85f, 0.35f}  // lemon yellow
        };
        
        float tubW = 0.44f;
        float tubH = 0.95f; // Rectangular tubs fitting in the sloped tray
        
        for(int c=0; c<8; ++c) {
            float tx = trayX - 1.75f + c*0.50f; // evenly spaced for 8 tubs
            
            // Tub hole (dark inside background)
            flat(shader, {0.18f, 0.18f, 0.22f});
            glm::mat4 hole = glm::translate(base, {tx, trayY, trayZ + 0.02f});
            hole = glm::rotate(hole, glm::radians(-50.0f), {1, 0, 0});
            hole = glm::scale(hole, {tubW, tubH, 0.22f}); // slightly thicker to show depth
            shader.setMat4("model", hole);
            glBindVertexArray(cubeVAO); glDrawArrays(GL_TRIANGLES,0,36);
            
            // Ice cream mass (textured, popping out)
            shader.setBool("useTexture", true);
            glBindTexture(GL_TEXTURE_2D, scoopTexs[c % scoopTexs.size()]);
            shader.setVec3("objectColor", {1.0f, 1.0f, 1.0f});
            shader.setVec3("emission", {0.0f, 0.0f, 0.0f}); 
            shader.setVec3("lightSpecular", {0.03f, 0.03f, 0.03f}); // matte look
            shader.setFloat("shininess", 2.0f);
            // Draw 3 large oval scoops per tub, stacked vertically on the sloped face
            for(int s=-1; s<=1; ++s) {
                glm::mat4 sm = glm::translate(base, {tx, trayY, trayZ + 0.02f});
                sm = glm::rotate(sm, glm::radians(-50.0f), {1, 0, 0});
                sm = glm::translate(sm, {0.0f, s*0.30f, 0.12f}); // pop out along local Z (+Z points up/forward)
                sm = glm::scale(sm, {tubW-0.08f, 0.35f, 0.16f}); // flattened overlapping spheres
                shader.setMat4("model", sm);
                glBindVertexArray(popcornSph.sphereTexVAO);
                glDrawElements(GL_TRIANGLES, popcornSph.getIndexCount(), GL_UNSIGNED_INT, 0);
            }
            shader.setBool("useTexture", false);
            shader.setVec3("lightSpecular", {1.0f, 1.0f, 1.0f}); // reset
            shader.setFloat("shininess", 32.0f); // reset
            shader.setVec3("emission", {0, 0, 0});
        }
    }
    // Helper to get half-width
    float W_HALF() const { return 7.0f * 0.5f; }

    // ═══════════════════════════════════════════════════════════════
    // 2. RIGHT SIDE — SAWHORSE TABLE (trestle / A-frame legs)
    // ═══════════════════════════════════════════════════════════════
    void drawLeftSawhorseTable(Shader& shader, glm::mat4 base, unsigned int woodTex, const std::vector<unsigned int>& scoopTexs) {
        const glm::vec3 OFF(12.0f, 0.0f, -0.2f); // Moved to the right side of the umbrella
        shader.use();

        // Table top plank (raised to Y=1.20 center to cover leg joints)
        useWood(shader, woodTex);
        cube(shader, base, OFF+glm::vec3(0,1.20f,0), {3.5f, 0.10f, 1.6f}); // Surface at 1.25f

        // Two A-frame / X trestle supports
        flat(shader, {0.42f,0.26f,0.10f});
        for (int s : {-1,1}) {
            float ox = s * 0.82f;
            // crossed diagonal legs (centered at Y=0.55 so base reaches ~0)
            for (int d : {-1,1}) {
                glm::mat4 leg=glm::translate(base, OFF+glm::vec3(ox,0.55f,0));
                leg=glm::rotate(leg,glm::radians(float(d)*32.0f),{0,0,1});
                // Ensure legs are long enough to reach 0
                leg=glm::scale(leg,{0.065f,1.35f,0.065f});
                shader.setMat4("model",leg);
                glBindVertexArray(cubeVAO); glDrawArrays(GL_TRIANGLES,0,36);
            }
            // cross bar at ankle height
            glm::mat4 bar=glm::translate(base,OFF+glm::vec3(ox,0.18f,0));
            bar=glm::scale(bar,{0.065f,0.07f,0.9f});
            shader.setMat4("model",bar);
            glBindVertexArray(cubeVAO); glDrawArrays(GL_TRIANGLES,0,36);
        }
        // horizontal stretcher connecting the two A-frames
        flat(shader,{0.38f,0.22f,0.08f});
        cube(shader,base,OFF+glm::vec3(0,0.48f,0),{1.6f,0.06f,0.06f});

        // ── items on table (resting accurately on table top Y = 1.25f) ──
        
        // waffle cone with scoop (left) - placed in a small wooden cone holder block
        flat(shader, {0.78f, 0.55f, 0.18f});
        glm::mat4 holder = glm::translate(base, OFF+glm::vec3(-0.65f, 1.30f, 0)); // Center 1.30, Top 1.35
        holder = glm::scale(holder, {0.25f, 0.10f, 0.25f});
        shader.setMat4("model", holder);
        glBindVertexArray(cubeVAO); glDrawArrays(GL_TRIANGLES,0,36);

        // Lower Cone (restored previous version logic: peak down)
        flat(shader,{0.82f,0.60f,0.28f});
        glm::mat4 cone=glm::translate(base,OFF+glm::vec3(-0.65f, 1.45f, 0));
        cone=glm::rotate(cone,glm::radians(180.0f),{1,0,0});
        cone=glm::scale(cone,{0.16f,0.30f,0.16f});
        shader.setMat4("model",cone); pyr.draw();
        
        // Scoop texturing with scope2 (index 1 of scoopTexs) + previous version position ratio
        shader.setBool("useTexture", true);
        glBindTexture(GL_TEXTURE_2D, scoopTexs[1]);
        shader.setVec3("objectColor", {1.0f, 1.0f, 1.0f});
        shader.setVec3("emission", {0.0f, 0.0f, 0.0f});
        shader.setVec3("lightSpecular", {0.03f, 0.03f, 0.03f}); // matte look
        shader.setFloat("shininess", 2.0f);
        
        glm::mat4 scoop=glm::translate(base,OFF+glm::vec3(-0.65f, 1.67f, 0));
        scoop=glm::scale(scoop,glm::vec3(0.22f)); 
        shader.setMat4("model",scoop); 
        glBindVertexArray(popcornSph.sphereTexVAO);
        glDrawElements(GL_TRIANGLES, popcornSph.getIndexCount(), GL_UNSIGNED_INT, 0);

        shader.setBool("useTexture", false);
        shader.setVec3("lightSpecular", {1.0f, 1.0f, 1.0f});
        shader.setFloat("shininess", 32.0f);

        // large red bottle (centre-left)
        flat(shader,{0.90f,0.18f,0.18f});
        glm::mat4 bot=glm::translate(base,OFF+glm::vec3(0.15f, 1.25f, 0)); 
        bot=glm::scale(bot,{0.17f,0.55f,0.17f}); shader.setMat4("model",bot); cyl.draw();
        flat(shader,{0.95f,0.95f,0.95f});
        glm::mat4 bcap=glm::translate(base,OFF+glm::vec3(0.15f, 1.80f, 0)); 
        bcap=glm::scale(bcap,{0.11f,0.07f,0.11f}); shader.setMat4("model",bcap); cyl.draw();

        // small yellow jar (right)
        flat(shader,{0.92f,0.82f,0.18f});
        glm::mat4 jar=glm::translate(base,OFF+glm::vec3(0.75f, 1.25f, 0)); 
        jar=glm::scale(jar,{0.14f,0.30f,0.14f}); shader.setMat4("model",jar); cyl.draw();
        flat(shader,{0.75f,0.65f,0.10f});
        glm::mat4 jlid=glm::translate(base,OFF+glm::vec3(0.75f, 1.55f, 0)); 
        jlid=glm::scale(jlid,{0.16f,0.04f,0.16f}); shader.setMat4("model",jlid); cyl.draw();
    }

    // ═══════════════════════════════════════════════════════════════
    // 3. RIGHT SIDE A — BURGER BUN DOME STAND
    //    (round dome on white pedestal with drawer lines)
    // ═══════════════════════════════════════════════════════════════
    void drawBurgerDomeStand(Shader& shader, glm::mat4 base, unsigned int wood2Tex, unsigned int burgerTex) {
        const glm::vec3 OFF(4.8f, 0.0f, -0.1f);
        shader.use();

        // 1. Cabinet Body with wood2.png texture
        shader.setBool("useTexture", true);
        shader.setInt("texture_diffuse", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, wood2Tex);
        shader.setVec3("objectColor", {1.0f, 1.0f, 1.0f});

        // Main cabinet cube - TALLER (Height 1.55)
        const float cabH = 1.55f;
        cube(shader, base, OFF + glm::vec3(0, cabH/2.0f, 0), {1.25f, cabH, 1.05f});

        // 2. White drawer fronts
        shader.setBool("useTexture", false);
        flat(shader, {0.95f, 0.95f, 0.95f});
        for (int i=0; i<4; ++i) { // 4 drawers now
            float fy = 0.25f + i*0.38f;
            cube(shader, base, OFF + glm::vec3(0, fy, 0.53f), {1.05f, 0.30f, 0.02f});
        }

        // Darker trims/borders on drawers
        flat(shader, {0.40f, 0.22f, 0.12f});
        for (int i=0; i<4; ++i) {
            float fy = 0.25f + i*0.38f;
            cube(shader, base, OFF + glm::vec3(0, fy, 0.54f), {1.05f, 0.02f, 0.03f});
        }

        // 3. Top Plate (Dark wood trim)
        flat(shader, {0.35f, 0.18f, 0.08f});
        cube(shader, base, OFF + glm::vec3(0, cabH + 0.05f, 0), {1.45f, 0.12f, 1.20f});

        // 4. BIG BURGER DOME (Textured with burger.png) - REFINED PARABOLIC PEAK
        shader.setBool("useTexture", true);
        glBindTexture(GL_TEXTURE_2D, burgerTex);
        shader.setVec3("objectColor", {1.0f, 1.0f, 1.0f});
        shader.setVec3("emission", {0.10f, 0.10f, 0.10f});
        
        // Lower shininess to make it look like bread (matte)
        shader.setVec3("lightSpecular", {0.2f, 0.2f, 0.2f});
        shader.setFloat("shininess", 32.0f);

        // Match top plate dimensions (1.45 x 1.20). Radius is 0.5, so scale 1.45 means width 1.45
        // High Y-scale for parabolic look
        glm::mat4 dome = glm::translate(base, OFF + glm::vec3(0, cabH + 0.11f, 0)); 
        dome = glm::scale(dome, glm::vec3(1.45f, 1.90f, 1.20f)); 
        shader.setMat4("model", dome);
        
        glBindVertexArray(popcornSph.sphereTexVAO);
        glDrawElements(GL_TRIANGLES, popcornSph.getIndexCount(), GL_UNSIGNED_INT, 0);

        shader.setBool("useTexture", false);
        shader.setVec3("emission", {0,0,0});
        
        // Restore lighting
        shader.setVec3("lightSpecular", {1.0f, 1.0f, 1.0f}); 
    }

    void drawPopcornBucketStand(Shader& shader, glm::mat4 base, unsigned int stripeTex, unsigned int popcornTex) {
        const glm::vec3 OFF(7.2f, 0.0f, -0.1f);
        shader.use();

        // 1. Shorter wooden post
        const float postH = 1.3f;
        flat(shader,{0.42f,0.26f,0.10f});
        glm::mat4 post=glm::translate(base,OFF+glm::vec3(0,0.0f,0));
        post=glm::scale(post,{0.10f,postH,0.10f}); shader.setMat4("model",post); cyl.draw();

        // 2. Solid post base disc
        flat(shader,{0.35f,0.20f,0.08f});
        glm::mat4 base2=glm::translate(base,OFF+glm::vec3(0,0.04f,0));
        base2=glm::scale(base2,{0.90f,0.08f,0.90f}); shader.setMat4("model",base2); sph.drawSphere(shader);

        // 3. Solid Bucket platform
        flat(shader,{0.38f,0.22f,0.08f});
        glm::mat4 shelf=glm::translate(base,OFF+glm::vec3(0,postH,0));
        shelf=glm::scale(shelf,{1.30f,0.08f,1.30f}); shader.setMat4("model",shelf); sph.drawSphere(shader);

        // 4. Striped bucket
        glm::vec3 bandCols[]={
            {0.90f,0.10f,0.10f},{1.0f,1.0f,1.0f},
            {0.90f,0.10f,0.10f},{1.0f,1.0f,1.0f},
            {0.90f,0.10f,0.10f},{1.0f,1.0f,1.0f},
        };
        for (int i=0;i<6;++i) {
            flat(shader,bandCols[i]);
            float by = postH + 0.04f + i*0.16f;
            glm::mat4 band=glm::translate(base,OFF+glm::vec3(0,by,0));
            float r = 0.40f + i*0.015f;
            band=glm::scale(band,{r*2.0f,0.16f,r*2.0f}); shader.setMat4("model",band); cyl.draw();
        }

        // bucket rim
        flat(shader,{0.55f,0.12f,0.12f});
        float topBucketY = postH + 0.04f + 6*0.16f;
        float topR = 0.40f + 5*0.015f; // ~0.475
        glm::mat4 rim=glm::translate(base,OFF+glm::vec3(0,topBucketY,0));
        rim=glm::scale(rim,{(topR+0.04f)*2.0f,0.05f,(topR+0.04f)*2.0f}); shader.setMat4("model",rim); cyl.draw();

        // 5. OVERFLOWING TEXTURED POPCORN
        // Enable textures
        shader.setBool("useTexture", true);
        shader.setInt("texture_diffuse", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, popcornTex);
        shader.setVec3("objectColor", {1.0f, 1.0f, 1.0f});
        shader.setVec3("emission", {0.50f, 0.50f, 0.50f}); // High emission to make it appear pure white
        
        // Lower shininess for matte popcorn look
        shader.setVec3("lightSpecular", {0.05f, 0.05f, 0.05f});
        shader.setFloat("shininess", 16.0f);

        // Large textured base heap (dome) - lowered a lot so it's just a "fill"
        float actualBucketRadius = topR * 2.0f;
        glm::mat4 bulk = glm::translate(base, OFF+glm::vec3(0, topBucketY - 0.20f, 0));
        bulk = glm::scale(bulk, glm::vec3(actualBucketRadius * 1.9f, 0.8f, actualBucketRadius * 1.9f));
        shader.setMat4("model", bulk);
        glBindVertexArray(popcornSph.sphereTexVAO);
        glDrawElements(GL_TRIANGLES, popcornSph.getIndexCount(), GL_UNSIGNED_INT, 0);

        // Individual kernels heaping on top
        srand(42);
        for(int m=0; m<5; ++m) { // 5 layers now
            // MASSIVE density for the bottom layers to hide EVERYTHING
            int numKernels = (m == 0) ? 350 : (m == 1 ? 250 : 150 - m*30);
            float mY = topBucketY + 0.0f + (m * 0.15f);
            // bottom layers cover the full radius + a tiny bit of overlap to the rim
            float maxR = (m < 2) ? (actualBucketRadius * 1.02f) : (actualBucketRadius - (m-1) * 0.40f);

            for(int k=0; k < numKernels; ++k) {
                float angle = (rand() % 3600) * 0.1f * 3.14159f / 180.0f;
                float rNorm = sqrt((rand() % 1000) / 1000.0f);
                float dist = rNorm * maxR;
                float yOff = ((rand() % 100) / 100.0f) * 0.25f;

                glm::mat4 pc=glm::translate(base, OFF+glm::vec3(dist*cos(angle), mY + yOff, dist*sin(angle)));
                // vary kernel size for realism
                float scale = 0.14f + (rand()%60/1000.0f);
                pc=glm::scale(pc, glm::vec3(scale));
                shader.setMat4("model", pc);
                glBindVertexArray(popcornSph.sphereTexVAO);
                glDrawElements(GL_TRIANGLES, popcornSph.getIndexCount(), GL_UNSIGNED_INT, 0);
            }
        }
        shader.setBool("useTexture", false);
        shader.setVec3("emission", {0,0,0});
        
        // Restore lighting
        shader.setVec3("lightSpecular", {1.0f, 1.0f, 1.0f}); 
    }

    // ═══════════════════════════════════════════════════════════════
    // 5. RIGHT SIDE C — SIDE UMBRELLA (between burger stand & popcorn)
    // ═══════════════════════════════════════════════════════════════
    void drawSideUmbrella(Shader& shader, glm::mat4 base, unsigned int redWhiteTex) {
        // Right of popcorn stand, Y=0 (Grounded!)
        const glm::vec3 OFF(9.2f, 0.0f, -0.1f);
        shader.use();

        // Pole — 5 units tall, base at ground (translates from 0 up to poleH)
        const float poleH = 5.0f;
        flat(shader,{0.18f,0.13f,0.08f});
        glm::mat4 pole=glm::translate(base, OFF+glm::vec3(0, 0.0f, 0)); // No Y-offset!
        pole=glm::scale(pole,{0.09f, poleH, 0.09f});
        shader.setMat4("model",pole); cyl.draw();

        // Canopy sits exactly at pole top.
        const float canopyScale = 1.80f;
        float canopyY = poleH - 0.55f * 1.0f;
        shader.setBool("useTexture", true);
        shader.setInt("texture_diffuse", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, redWhiteTex);
        shader.setVec3("objectColor", glm::vec3(1.0f));
        
        // Lower shininess to see fabric texture better
        shader.setVec3("lightSpecular", {0.3f, 0.3f, 0.3f});
        shader.setFloat("shininess", 32.0f);
        
        shader.setVec3("emission", glm::vec3(0.05f,0.0f,0.0f));
        glm::mat4 canopy=glm::translate(base, OFF+glm::vec3(0, canopyY, 0));
        canopy=glm::scale(canopy,{canopyScale, 1.0f, canopyScale});
        shader.setMat4("model",canopy);
        umbrellaCanopy.draw();
        shader.setVec3("emission",glm::vec3(0.0f));
        
        // Restore lighting
        shader.setVec3("lightSpecular", {1.0f, 1.0f, 1.0f}); 

        // Gold tip at pole top
        flat(shader,{0.88f,0.72f,0.10f});
        glm::mat4 tip=glm::translate(base, OFF+glm::vec3(0, poleH+0.10f, 0));
        tip=glm::scale(tip,glm::vec3(0.11f));
        shader.setMat4("model",tip); sph.drawSphere(shader);
    }
};

#endif
