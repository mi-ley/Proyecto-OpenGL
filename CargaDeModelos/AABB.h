#pragma once
#include <glm/glm.hpp>

struct AABB {
    glm::vec3 min;
    glm::vec3 max;

    // Transforms the AABB with a model matrix
    AABB transform(const glm::mat4& model) const {
        glm::vec3 corners[8] = {
            min,
            glm::vec3(max.x, min.y, min.z),
            glm::vec3(min.x, max.y, min.z),
            glm::vec3(min.x, min.y, max.z),
            glm::vec3(max.x, max.y, min.z),
            glm::vec3(min.x, max.y, max.z),
            glm::vec3(max.x, min.y, max.z),
            max
        };
        glm::vec3 newMin = glm::vec3(model * glm::vec4(corners[0], 1.0f));
        glm::vec3 newMax = newMin;
        for (int i = 1; i < 8; ++i) {
            glm::vec3 transformed = glm::vec3(model * glm::vec4(corners[i], 1.0f));
            newMin = glm::min(newMin, transformed);
            newMax = glm::max(newMax, transformed);
        }
        return { newMin, newMax };
    }
};

// Collision detection between two AABBs
inline bool checkAABBCollision(const AABB& a, const AABB& b) {
    return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
           (a.min.y <= b.max.y && a.max.y >= b.min.y) &&
           (a.min.z <= b.max.z && a.max.z >= b.min.z);
}
