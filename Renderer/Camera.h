//
// Created by allos on 4/23/2024.
//
#pragma once
#include "VkTypes.h"


class Camera {
public:
    glm::vec3 velocity;
    glm::vec3 position;

    //vertical rotation
    float pitch { 0.f };
    // horizontal rotation
    float yaw { 0.f };

    glm::mat4 getViewMatrix();
    glm::mat4 getRotationMatrix();

    ///TODO remove this, its just a dummy
    void processEvent(GLFWwindow* window);

    void update();

private:
    double lastY = 0;
    double lastX = 0;
};

