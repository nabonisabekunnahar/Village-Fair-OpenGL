#ifndef CYLINDER_H
#define CYLINDER_H

#include <glad/glad.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

class Cylinder {
public:
    unsigned int VAO;
    unsigned int vertexCount;

    Cylinder(int sectors = 12) {
        std::vector<float> vertices;
        float sectorStep = 2.0f * glm::pi<float>() / sectors;

        // Sides
        for (int i = 0; i <= sectors; ++i) {
            float angle = i * sectorStep;
            float x = cos(angle);
            float z = sin(angle);

            // Top vertex
            vertices.push_back(x); vertices.push_back(1.0f); vertices.push_back(z); // pos: (x, 1, z)
            vertices.push_back(x); vertices.push_back(0.0f); vertices.push_back(z); // normal: (x, 0, z)
            vertices.push_back((float)i / sectors); vertices.push_back(1.0f);        // uv: (u, 1)
            
            // Bottom vertex
            vertices.push_back(x); vertices.push_back(0.0f); vertices.push_back(z); // pos: (x, 0, z)
            vertices.push_back(x); vertices.push_back(0.0f); vertices.push_back(z); // normal: (x, 0, z)
            vertices.push_back((float)i / sectors); vertices.push_back(0.0f);        // uv: (u, 0)
        }

        vertexCount = (sectors + 1) * 2;
        unsigned int VBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        // Attribute 0: position (xyz)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // Attribute 1: normal (xyz)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        // Attribute 2: texCoords (uv)
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
    }

    void draw() const {
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexCount);
        glBindVertexArray(0);
    }
};

#endif
