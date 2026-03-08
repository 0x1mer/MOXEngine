#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Scale
{
    glm::vec3 value{ 1.0f, 1.0f, 1.0f };

    Scale() = default;
    explicit Scale(const glm::vec3& v) : value(v) {}
    Scale(float x, float y, float z) : value(x, y, z) {}
    Scale(int x, int y, int z) : value(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)) {}

    // Scale op Scale (component-wise)
    friend Scale operator*(const Scale& a, const Scale& b) noexcept { return Scale(a.value * b.value); }
    friend Scale operator/(const Scale& a, const Scale& b) noexcept { return Scale(a.value / b.value); }

    Scale& operator*=(const Scale& rhs) noexcept { value *= rhs.value; return *this; }
    Scale& operator/=(const Scale& rhs) noexcept { value /= rhs.value; return *this; }

    // Scale op scalar
    friend Scale operator*(const Scale& s, float k) noexcept { return Scale(s.value * k); }
    friend Scale operator*(float k, const Scale& s) noexcept { return Scale(s.value * k); }
    friend Scale operator/(const Scale& s, float k) noexcept { return Scale(s.value / k); }

    Scale& operator*=(float k) noexcept { value *= k; return *this; }
    Scale& operator/=(float k) noexcept { value /= k; return *this; }
};

struct Rotation
{
    glm::quat value{ 1.0f, 0.0f, 0.0f, 0.0f }; // identity (w, x, y, z)

    Rotation() = default;
    explicit Rotation(const glm::quat& q) : value(q) {}

    static Rotation FromEulerDegrees(const glm::vec3& degrees)
    {
        return Rotation(glm::quat(glm::radians(degrees)));
    }

    static Rotation FromAxisAngleDegrees(const glm::vec3& axis, float degrees)
    {
        return Rotation(glm::angleAxis(glm::radians(degrees), glm::normalize(axis)));
    }

    // Composition: applying b then a? (glm convention: q * v applies q)
    // Here: (a * b) means "first b, then a" when applied to a vector.
    friend Rotation operator*(const Rotation& a, const Rotation& b) noexcept
    {
        return Rotation(a.value * b.value);
    }

    Rotation& operator*=(const Rotation& rhs) noexcept
    {
        value = value * rhs.value;
        return *this;
    }

    // Divide = compose with inverse
    friend Rotation operator/(const Rotation& a, const Rotation& b) noexcept
    {
        return Rotation(a.value * glm::inverse(b.value));
    }

    Rotation& operator/=(const Rotation& rhs) noexcept
    {
        value = value * glm::inverse(rhs.value);
        return *this;
    }

    // Apply rotation to vector
    glm::vec3 operator*(const glm::vec3& v) const noexcept
    {
        return value * v;
    }

    Rotation Inverse() const noexcept { return Rotation(glm::inverse(value)); }
    Rotation Normalized() const noexcept { return Rotation(glm::normalize(value)); }
};

struct Position
{
    glm::vec3 value{ 0.0f, 0.0f, 0.0f };

    Position() = default;
    explicit Position(const glm::vec3& v) : value(v) {}
    Position(float x, float y, float z) : value(x, y, z) {}
    Position(int x, int y, int z) : value(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)) {}

    // Position op Position
    friend Position operator+(const Position& a, const Position& b) noexcept { return Position(a.value + b.value); }
    friend Position operator-(const Position& a, const Position& b) noexcept { return Position(a.value - b.value); }

    Position& operator+=(const Position& rhs) noexcept { value += rhs.value; return *this; }
    Position& operator-=(const Position& rhs) noexcept { value -= rhs.value; return *this; }

    // Position op vec3
    friend Position operator+(const Position& p, const glm::vec3& v) noexcept { return Position(p.value + v); }
    friend Position operator-(const Position& p, const glm::vec3& v) noexcept { return Position(p.value - v); }

    Position& operator+=(const glm::vec3& v) noexcept { value += v; return *this; }
    Position& operator-=(const glm::vec3& v) noexcept { value -= v; return *this; }

    // Position op scalar
    friend Position operator*(const Position& p, float k) noexcept { return Position(p.value * k); }
    friend Position operator*(float k, const Position& p) noexcept { return Position(p.value * k); }
    friend Position operator/(const Position& p, float k) noexcept { return Position(p.value / k); }

    Position& operator*=(float k) noexcept { value *= k; return *this; }
    Position& operator/=(float k) noexcept { value /= k; return *this; }
};

struct Transform
{
    Position position{};
    Rotation rotation{};
    Scale    scale{};

    // Apply to point (position in space)
    glm::vec3 ApplyPoint(const glm::vec3& p) const noexcept
    {
        // p' = T( R( S(p) ) )
        return position.value + (rotation.value * (scale.value * p));
    }

    // Apply to direction (vector), ignores translation
    glm::vec3 ApplyVector(const glm::vec3& v) const noexcept
    {
        return rotation.value * (scale.value * v);
    }

    glm::mat4 Matrix() const
    {
        glm::mat4 T = glm::translate(glm::mat4(1.0f), position.value);
        glm::mat4 R = glm::toMat4(rotation.value);
        glm::mat4 S = glm::scale(glm::mat4(1.0f), scale.value);
        return T * R * S;
    }

    void SetPosition(const glm::vec3& p) noexcept
    {
        position.value = p;
    }

    void SetRotation(const Rotation& r) noexcept
    {
        rotation = r;
    }

    void SetScale(const glm::vec3& s) noexcept
    {
        scale.value = s;
    }

    void SetUniformScale(float s) noexcept
    {
        scale.value = glm::vec3(s);
    }

    void TranslateWorld(const glm::vec3& delta) noexcept
    {
        position.value += delta;
    }

    void TranslateLocal(const glm::vec3& delta) noexcept
    {
        position.value += rotation.value * delta;
    }

    void RotateLocal(const Rotation& r) noexcept
    {
        rotation = rotation * r;
        rotation.value = glm::normalize(rotation.value);
    }

    void RotateWorld(const Rotation& r) noexcept
    {
        rotation = r * rotation;
        rotation.value = glm::normalize(rotation.value);
    }

    void ScaleBy(const glm::vec3& factor) noexcept
    {
        scale.value *= factor;
    }

    void ScaleBy(float uniform) noexcept
    {
        scale.value *= uniform;
    }

    glm::vec3 Forward() const noexcept
    {
        return rotation.value * glm::vec3(0, 0, -1);
    }

    glm::vec3 Right() const noexcept
    {
        return rotation.value * glm::vec3(1, 0, 0);
    }

    glm::vec3 Up() const noexcept
    {
        return rotation.value * glm::vec3(0, 1, 0);
    }

    void LookAt(const glm::vec3& target, const glm::vec3& worldUp = { 0,1,0 })
    {
        glm::mat4 look = glm::lookAt(position.value, target, worldUp);
        rotation.value = glm::conjugate(glm::quat_cast(look));
    }

    // Composition: (a * b) == apply b then a
    friend Transform operator*(const Transform& a, const Transform& b) noexcept
    {
        Transform out;

        // Scale comp: component-wise
        out.scale = a.scale * b.scale;

        // Rotation comp
        out.rotation = a.rotation * b.rotation;

        // Translation comp:
        // p_out = a.p + a.R * (a.S * b.p)
        out.position = Position(a.position.value + (a.rotation.value * (a.scale.value * b.position.value)));

        return out;
    }

    Transform& operator*=(const Transform& rhs) noexcept
    {
        *this = (*this) * rhs;
        return *this;
    }
};