#ifndef PYRAMID_H
#define PYRAMID_H

#include <glad/glad.h>
#include <vector>
#include <glm/glm.hpp>

class Pyramid {
public:
    unsigned int VAO;
    unsigned int vertexCount;

    Pyramid() {
        // Form: 4 Triangles for walls + 1 square base (2 triangles)
        // Vertex arrangement: pos (3), norm (3), uv (2)
        
        // Normals for sides (approximate for simple lighting)
        glm::vec3 nF = glm::normalize(glm::vec3(0, 0.5f, 1.0f));
        glm::vec3 nB = glm::normalize(glm::vec3(0, 0.5f, -1.0f));
        glm::vec3 nL = glm::normalize(glm::vec3(-1.0f, 0.5f, 0));
        glm::vec3 nR = glm::normalize(glm::vec3(1.0f, 0.5f, 0));
        glm::vec3 nD = glm::vec3(0, -1.0f, 0);

        float vertices[] = {
            // Positions          // Normals           // UVs
            // Front face
             0.0f,  0.5f,  0.0f,   nF.x, nF.y, nF.z,   0.5f, 1.0f,
            -0.5f, -0.5f,  0.5f,   nF.x, nF.y, nF.z,   0.0f, 0.0f,
             0.5f, -0.5f,  0.5f,   nF.x, nF.y, nF.z,   1.0f, 0.0f,

            // Right face
             0.0f,  0.5f,  0.0f,   nR.x, nR.y, nR.z,   0.5f, 1.0f,
             0.5f, -0.5f,  0.5f,   nR.x, nR.y, nR.z,   0.0f, 0.0f,
             0.5f, -0.5f, -0.5f,   nR.x, nR.y, nR.z,   1.0f, 0.0f,

            // Back face
             0.0f,  0.5f,  0.0f,   nB.x, nB.y, nB.z,   0.5f, 1.0f,
             0.5f, -0.5f, -0.5f,   nB.x, nB.y, nB.z,   0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,   nB.x, nB.y, nB.z,   1.0f, 0.0f,

            // Left face
             0.0f,  0.5f,  0.0f,   nL.x, nL.y, nL.z,   0.5f, 1.0f,
            -0.5f, -0.5f, -0.5f,   nL.x, nL.y, nL.z,   0.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,   nL.x, nL.y, nL.z,   1.0f, 0.0f,

            // Base (Square)
            -0.5f, -0.5f, -0.5f,   nD.x, nD.y, nD.z,   0.0f, 0.0f,
             0.5f, -0.5f, -0.5f,   nD.x, nD.y, nD.z,   1.0f, 0.0f,
             0.5f, -0.5f,  0.5f,   nD.x, nD.y, nD.z,   1.0f, 1.0f,
             0.5f, -0.5f,  0.5f,   nD.x, nD.y, nD.z,   1.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,   nD.x, nD.y, nD.z,   0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,   nD.x, nD.y, nD.z,   0.0f, 0.0f
        };

        vertexCount = sizeof(vertices) / (8 * sizeof(float));
        unsigned int VBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        
        glBindVertexArray(0);
    }

    void draw() const {
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        glBindVertexArray(0);
    }
};

#endif
