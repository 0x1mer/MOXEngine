#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp> // column()
#include <array>

struct Plane
{
    glm::vec3 n{ 0.0f }; // normal
    float d = 0.0f;    // plane equation: dot(n, x) + d = 0

    void Normalize()
    {
        const float len = glm::length(n);
        if (len > 0.0f) { n /= len; d /= len; }
    }

    // > 0 => point is in front (inside side for our frustum planes)
    float Distance(const glm::vec3& p) const
    {
        return glm::dot(n, p) + d;
    }
};

struct Frustum
{
    // Order: Left, Right, Bottom, Top, Near, Far
    std::array<Plane, 6> p;

    static Frustum FromMatrix(const glm::mat4& clip) // clip = P * V
    {
        Frustum f;

        // GLM is column-major.
        // Extract rows of clip matrix:
        const glm::vec4 row0 = glm::row(clip, 0);
        const glm::vec4 row1 = glm::row(clip, 1);
        const glm::vec4 row2 = glm::row(clip, 2);
        const glm::vec4 row3 = glm::row(clip, 3);

        auto makePlane = [](const glm::vec4& v) -> Plane {
            Plane pl;
            pl.n = glm::vec3(v);
            pl.d = v.w;
            pl.Normalize();
            return pl;
            };

        // Standard extraction:
        f.p[0] = makePlane(row3 + row0); // Left
        f.p[1] = makePlane(row3 - row0); // Right
        f.p[2] = makePlane(row3 + row1); // Bottom
        f.p[3] = makePlane(row3 - row1); // Top
        f.p[4] = makePlane(row3 + row2); // Near
        f.p[5] = makePlane(row3 - row2); // Far

        return f;
    }

    bool ContainsSphere(const glm::vec3& c, float r) const
    {
        for (const Plane& pl : p)
        {
            if (pl.Distance(c) < -r) // полностью снаружи
                return false;
        }
        return true;
    }

    // AABB in world space (min/max)
    bool IntersectsAABB(const glm::vec3& bmin, const glm::vec3& bmax) const
    {
        for (const Plane& pl : p)
        {
            // "positive vertex" test
            glm::vec3 v = bmin;
            if (pl.n.x >= 0) v.x = bmax.x;
            if (pl.n.y >= 0) v.y = bmax.y;
            if (pl.n.z >= 0) v.z = bmax.z;

            if (pl.Distance(v) < 0.0f)
                return false;
        }
        return true;
    }
};