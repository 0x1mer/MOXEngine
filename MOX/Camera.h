#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Transform.h"
#include "Frustum.h"

enum class CameraMovement
{
    Forward,
    Backward,
    Left,
    Right,
    Up,
    Down
};

class Camera
{
public:
    Transform transform{};

    float movementSpeed = 2.5f;
    float mouseSensitivity = 0.1f;
    float zoomDegrees = 45.0f;

    bool  constrainPitch = true;
    float pitchLimitDeg = 89.0f;

    float yawDeg = -90.0f;
    float pitchDeg = 0.0f;

    Camera(
        glm::vec3 position = { 0.0f, 0.0f, 0.0f },
        glm::vec3 worldUp = { 0.0f, 1.0f, 0.0f },
        float yaw = -90.0f,
        float pitch = 0.0f)
        : worldUp_(glm::normalize(worldUp))
        , yawDeg(yaw)
        , pitchDeg(pitch)
    {
        transform.SetPosition(position);
        ApplyYawPitchToRotation();
    }

    glm::mat4 GetViewMatrix() const
    {
        const glm::vec3 pos = transform.position.value;
        const glm::vec3 fwd = Forward();
        const glm::vec3 up = Up();
        return glm::lookAt(pos, pos + fwd, up);
    }

    glm::mat4 GetProjectionMatrix(
        float aspect,
        float nearPlane = 0.1f,
        float farPlane = 100.0f) const
    {
        return glm::perspective(
            glm::radians(zoomDegrees),
            aspect,
            nearPlane,
            farPlane
        );
    }

    float GetZoom() const noexcept { return zoomDegrees; }

    glm::vec3 Position() const noexcept { return transform.position.value; }
    glm::vec3 Forward()  const noexcept { return transform.Forward(); }
    glm::vec3 Right()    const noexcept { return transform.Right(); }
    glm::vec3 Up()       const noexcept { return transform.Up(); }

    Frustum GetFrustum(float aspect, float nearPlane = 0.1f, float farPlane = 100.0f) const
    {
        const glm::mat4 P = GetProjectionMatrix(aspect, nearPlane, farPlane);
        const glm::mat4 V = GetViewMatrix();
        return Frustum::FromMatrix(P * V);
    }

    void ProcessKeyboard(CameraMovement dir, float dt)
    {
        const float v = movementSpeed * dt;

        const glm::vec3 f = glm::normalize(glm::vec3(Forward().x, 0.0f, Forward().z));
        const glm::vec3 r = glm::normalize(glm::vec3(Right().x, 0.0f, Right().z));

        switch (dir)
        {
        case CameraMovement::Forward:  transform.TranslateWorld(f * v); break;
        case CameraMovement::Backward: transform.TranslateWorld(-f * v); break;
        case CameraMovement::Left:     transform.TranslateWorld(-r * v); break;
        case CameraMovement::Right:    transform.TranslateWorld(r * v); break;
        case CameraMovement::Up:       transform.TranslateWorld(worldUp_ * v); break;
        case CameraMovement::Down:     transform.TranslateWorld(-worldUp_ * v); break;
        }
    }

    void ProcessMouseMovement(float xoffset, float yoffset)
    {
        xoffset *= mouseSensitivity;
        yoffset *= mouseSensitivity;

        yawDeg += xoffset;
        pitchDeg += yoffset;

        if (constrainPitch)
        {
            if (pitchDeg > pitchLimitDeg) pitchDeg = pitchLimitDeg;
            if (pitchDeg < -pitchLimitDeg) pitchDeg = -pitchLimitDeg;
        }

        ApplyYawPitchToRotation();
    }

    void ProcessMouseScroll(float yoffset)
    {
        zoomDegrees -= yoffset;
        if (zoomDegrees < 1.0f)  zoomDegrees = 1.0f;
        if (zoomDegrees > 45.0f) zoomDegrees = 45.0f;
    }

    void LookAt(const glm::vec3& target)
    {
        transform.LookAt(target, worldUp_);
        SyncYawPitchFromRotationApprox();
    }

private:
    glm::vec3 worldUp_{ 0.0f, 1.0f, 0.0f };

    void ApplyYawPitchToRotation()
    {
        const Rotation yawR = Rotation::FromAxisAngleDegrees(worldUp_, yawDeg);

        const glm::vec3 rightAfterYaw = yawR.value * glm::vec3(1.0f, 0.0f, 0.0f);
        const Rotation  pitchR = Rotation::FromAxisAngleDegrees(rightAfterYaw, pitchDeg);

        transform.rotation = (pitchR * yawR).Normalized();
    }

    void SyncYawPitchFromRotationApprox()
    {
        const glm::vec3 f = Forward();

        // Approximate extraction for mouse continuity after LookAt().
        yawDeg = glm::degrees(std::atan2(f.z, f.x)) - 90.0f;
        pitchDeg = glm::degrees(std::asin(glm::clamp(f.y, -1.0f, 1.0f)));
    }
};