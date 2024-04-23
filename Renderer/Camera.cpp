//
// Created by allos on 4/23/2024.
//

#include "Camera.h"

glm::mat4 Camera::getViewMatrix() {

}

glm::mat4 Camera::getRotationMatrix() {

}

void Camera::processEvent(GLFWwindow *window) {

}

void Camera::update() {
    glm::mat4 cameraRotation = getRotationMatrix();
    position += glm::vec3(cameraRotation * glm::vec4(velocity * 0.5f, 0.f));
}
