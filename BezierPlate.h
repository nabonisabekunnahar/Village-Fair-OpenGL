#ifndef BEZIER_PLATE_H
#define BEZIER_PLATE_H

#include <glad/glad.h>
#include <vector>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>

class BezierPlate {
public:
    BezierPlate() : VAO(0), VBO(0), EBO(0), indicesCount(0) {
        setupMesh();
    }

    ~BezierPlate() {
        if (VAO) glDeleteVertexArrays(1, &VAO);
        if (VBO) glDeleteBuffers(1, &VBO);
        if (EBO) glDeleteBuffers(1, &EBO);
    }

    void draw() {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)indicesCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

private:
    unsigned int VAO, VBO, EBO;
    size_t indicesCount;

    void setupMesh() {
        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        glm::vec2 p0(0.0f, 0.0f);
        glm::vec2 p1(0.7f, 0.0f);
        glm::vec2 p2(1.1f, 0.3f);
        glm::vec2 p3(1.3f, 0.4f);

        int latSegments = 30;
        int longSegments = 40;

        for (int i = 0; i <= latSegments; ++i) {
            float t = (float)i / latSegments;
            float invT = 1.0f - t;
            
            // Manual Cubic Bezier to avoid std::pow type issues and for efficiency
            float b0 = invT * invT * invT;
            float b1 = 3.0f * invT * invT * t;
            float b2 = 3.0f * invT * t * t;
            float b3 = t * t * t;

            glm::vec2 profile = b0 * p0 + b1 * p1 + b2 * p2 + b3 * p3;

            float r = profile.x;
            float y = profile.y;

            for (int j = 0; j <= longSegments; ++j) {
                float phi = glm::two_pi<float>() * (float)j / (float)longSegments;
                float x = r * (float)cos(phi);
                float z = r * (float)sin(phi);

                // Position
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);

                // Normal (Approximated as pointing upwards mostly)
                glm::vec3 normal = glm::normalize(glm::vec3(x * 0.2f, 1.0f, z * 0.2f));
                vertices.push_back(normal.x);
                vertices.push_back(normal.y);
                vertices.push_back(normal.z);

                // Planar UV Mapping (Better for top-down textures like ceramic/marble)
                float maxR = 1.3f; // Max radius from p3.x
                vertices.push_back((x / maxR) * 0.5f + 0.5f);
                vertices.push_back((z / maxR) * 0.5f + 0.5f);
            }
        }

        for (int i = 0; i < latSegments; ++i) {
            for (int j = 0; j < longSegments; ++j) {
                unsigned int first = i * (longSegments + 1) + j;
                unsigned int second = first + longSegments + 1;

                indices.push_back(first);
                indices.push_back(second);
                indices.push_back(first + 1);

                indices.push_back(second);
                indices.push_back(second + 1);
                indices.push_back(first + 1);
            }
        }

        indicesCount = indices.size();

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // Normal attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        // Texture coord attribute
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
    }
};

#endif
