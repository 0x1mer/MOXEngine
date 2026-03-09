#pragma once

#include <cmath>

namespace math {

    [[nodiscard]]
    inline constexpr int floorDiv(int value, int divisor) noexcept
    {
        int q = value / divisor;
        int r = value % divisor;

        if ((r != 0) && ((r < 0) != (divisor < 0)))
            --q;

        return q;
    }

    [[nodiscard]]
    inline constexpr int mod(int x, int m) noexcept
    {
        int r = x % m;
        return r < 0 ? r + m : r;
    }

}; // namespace math