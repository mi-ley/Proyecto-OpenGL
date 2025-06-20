// Draw an AABB as lines
#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "AABB.h"
#include "shaderClass.h"
#include "Camera.h"
#include <glad/glad.h>

class AABBRenderer {
    GLuint VAO, VBO;
public:
    AABBRenderer() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
    }
    ~AABBRenderer() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }
    void draw(const AABB& box, Shader& shader, Camera& camera, glm::vec3 color, const glm::mat4& model) {
        glm::vec3 min = box.min;
        glm::vec3 max = box.max;
        std::vector<glm::vec3> lines = {
            // Bottom
            {min.x, min.y, min.z}, {max.x, min.y, min.z},
            {max.x, min.y, min.z}, {max.x, min.y, max.z},
            {max.x, min.y, max.z}, {min.x, min.y, max.z},
            {min.x, min.y, max.z}, {min.x, min.y, min.z},
            // Top
            {min.x, max.y, min.z}, {max.x, max.y, min.z},
            {max.x, max.y, min.z}, {max.x, max.y, max.z},
            {max.x, max.y, max.z}, {min.x, max.y, max.z},
            {min.x, max.y, max.z}, {min.x, max.y, min.z},
            // Sides
            {min.x, min.y, min.z}, {min.x, max.y, min.z},
            {max.x, min.y, min.z}, {max.x, max.y, min.z},
            {max.x, min.y, max.z}, {max.x, max.y, max.z},
            {min.x, min.y, max.z}, {min.x, max.y, max.z},
        };
        for (auto& v : lines) v = glm::vec3(model * glm::vec4(v, 1.0f));
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(lines.size() * sizeof(glm::vec3)), lines.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glEnableVertexAttribArray(0);
        shader.Activate();
        camera.Matrix(shader, "view");
        // Enviar proyección
        GLint projLoc = glGetUniformLocation(shader.ID, "projection");
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)camera.width / camera.height, 0.1f, 1000.0f);
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, &glm::mat4(1.0f)[0][0]);
        glUniform3fv(glGetUniformLocation(shader.ID, "lineColor"), 1, &color[0]);
        glLineWidth(3.0f); // Grosor de línea
        glDrawArrays(GL_LINES, 0, (GLsizei)lines.size());
        glBindVertexArray(0);
    }
};
