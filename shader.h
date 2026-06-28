#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

// ============================================================
//  Shader Class
//  Loads, compiles, and links GLSL vertex + fragment shaders.
//
//  Usage:
//    Shader myShader("vert.glsl", "frag.glsl");
//    myShader.use();
//    myShader.setMat4("model", modelMatrix);
//    myShader.setVec3("lightColor", glm::vec3(1.0f));
// ============================================================
class Shader {
public:
    unsigned int ID; // OpenGL program ID

    // --------------------------------------------------------
    //  Constructor: reads and compiles shaders from file paths
    // --------------------------------------------------------
    Shader(const char* vertexPath, const char* fragmentPath) {
        // 1. Read source code from files
        std::string   vertexCode, fragmentCode;
        std::ifstream vShaderFile, fShaderFile;

        vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        try {
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vss, fss;
            vss << vShaderFile.rdbuf();
            fss << fShaderFile.rdbuf();
            vertexCode = vss.str();
            fragmentCode = fss.str();
        }
        catch (std::ifstream::failure&) {
            std::cerr << "[Shader] ERROR: Could not read shader files.\n"
                << "  Vertex:   " << vertexPath << "\n"
                << "  Fragment: " << fragmentPath << "\n";
        }

        const char* vSource = vertexCode.c_str();
        const char* fSource = fragmentCode.c_str();

        // 2. Compile vertex shader
        unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vSource, nullptr);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");

        // 3. Compile fragment shader
        unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fSource, nullptr);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");

        // 4. Link into a shader program
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");

        // 5. Clean up individual shaders (no longer needed after linking)
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        
        // Validate program
        int success;
        glGetProgramiv(ID, GL_LINK_STATUS, &success);
        if (!success) {
            std::cerr << "[Shader] WARNING: Shader program linking failed, ID reset to 0\n";
            glDeleteProgram(ID);
            ID = 0;
        }
    }
    void use() const {
        if (ID != 0) {
            glUseProgram(ID);
        } else {
            std::cerr << "[Shader] WARNING: Attempting to use invalid shader program (ID=0)\n";
        }
    }

    // --------------------------------------------------------
    //  Uniform setters - convenience wrappers
    // --------------------------------------------------------
    void setBool(const std::string& name, bool  value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }
    void setInt(const std::string& name, int   value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
    void setFloat(const std::string& name, float value) const {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }
    void setVec3(const std::string& name, const glm::vec3& v) const {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(v));
    }
    void setVec3(const std::string& name, float x, float y, float z) const {
        glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
    }
    void setVec4(const std::string& name, const glm::vec4& v) const {
        glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(v));
    }
    void setVec4(const std::string& name, float x, float y, float z, float w) const {
        glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
    }
    void setMat4(const std::string& name, const glm::mat4& m) const {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()),
            1, GL_FALSE, glm::value_ptr(m));
    }

private:
    // --------------------------------------------------------
    //  Check and print compile/link errors
    // --------------------------------------------------------
    void checkCompileErrors(unsigned int shader, const std::string& type) const {
        int  success;
        char infoLog[1024];
        if (type != "PROGRAM") {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
                std::cerr << "[Shader] COMPILE ERROR (" << type << "):\n"
                    << infoLog << "\n";
            }
        }
        else {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
                std::cerr << "[Shader] LINK ERROR:\n" << infoLog << "\n";
            }
        }
    }
};

#endif // SHADER_H
