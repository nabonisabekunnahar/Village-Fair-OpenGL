#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// ============================================================
//  Camera Movement Directions
// ============================================================
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// ============================================================
//  Default camera settings
// ============================================================
const float YAW = -90.0f;  // Face -Z by default
const float PITCH = 0.0f;
const float SPEED = 5.0f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

// ============================================================
//  FPS-style Camera Class
//  Handles WASD movement, mouse look, and scroll zoom.
//  Usage:
//    Camera cam(startPos);
//    cam.ProcessKeyboard(FORWARD, deltaTime);
//    cam.ProcessMouseMovement(xOffset, yOffset);
//    cam.ProcessMouseScroll(yOffset);
//    glm::mat4 view = cam.GetViewMatrix();
// ============================================================
class Camera {
public:
    // Camera attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    // Euler angles
    float Yaw;
    float Pitch;
    float Roll;

    // Camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // --------------------------------------------------------
    //  Constructor with position vector
    // --------------------------------------------------------
    Camera(glm::vec3 position = glm::vec3(0.0f, 2.0f, 5.0f),
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
        float yaw = YAW,
        float pitch = PITCH)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)),
        MovementSpeed(SPEED),
        MouseSensitivity(SENSITIVITY),
        Zoom(ZOOM)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        Roll = 0.0f;
        updateCameraVectors();
    }

    // --------------------------------------------------------
    //  Returns the view matrix (look-at matrix)
    // --------------------------------------------------------
    glm::mat4 GetViewMatrix() const {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // --------------------------------------------------------
    //  Keyboard input: moves camera along its axes
    // --------------------------------------------------------
    void ProcessKeyboard(Camera_Movement direction, float deltaTime) {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)  Position += Front * velocity;
        if (direction == BACKWARD) Position -= Front * velocity;
        if (direction == LEFT)     Position -= Right * velocity;
        if (direction == RIGHT)    Position += Right * velocity;
    }

    // --------------------------------------------------------
    //  Mouse movement: updates Yaw and Pitch
    //  constrainPitch prevents screen flip
    // --------------------------------------------------------
    void ProcessMouseMovement(float xoffset, float yoffset,
        GLboolean constrainPitch = true) {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        if (constrainPitch) {
            if (Pitch > 89.0f) Pitch = 89.0f;
            if (Pitch < -89.0f) Pitch = -89.0f;
        }

        updateCameraVectors();
    }

    // --------------------------------------------------------
    //  Scroll wheel: adjusts field of view (zoom effect)
    // --------------------------------------------------------
    void ProcessMouseScroll(float yoffset) {
        Zoom -= yoffset;
        if (Zoom < 1.0f) Zoom = 1.0f;
        if (Zoom > 90.0f) Zoom = 90.0f;
    }

    // --------------------------------------------------------
    //  Recalculates Front, Right, Up from Euler angles
    // --------------------------------------------------------
    void updateCameraVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);

        // Re-orthogonalize Right and Up vectors (standard)
        glm::vec3 right = glm::normalize(glm::cross(Front, WorldUp));
        glm::vec3 up    = glm::normalize(glm::cross(right, Front));

        // Apply Roll vector modifier
        glm::mat4 rollMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(Roll), Front);
        Right = glm::vec3(rollMatrix * glm::vec4(right, 0.0f));
        Up = glm::vec3(rollMatrix * glm::vec4(up, 0.0f));
    }
};

#endif // CAMERA_H