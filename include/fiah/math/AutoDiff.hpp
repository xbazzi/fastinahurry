// C++ Includes
#include <cstdint>
#include <cmath>

// Third Party Includes

// FastInAHurry Includes

namespace fiah
{
struct Dual
{
    double val;
    double d;
};

Dual operator+(Dual a, Dual b)
{
    return {a.val + b.val, a.d + b.d};
}

Dual operator-(Dual a, Dual b)
{
    return {a.val - b.val, a.d - b.d};
}

Dual operator*(Dual a, Dual b)
{
    return {
        a.val * b.val,
        a.d * b.val + a.val * b.d
    };
}

Dual operator/(Dual a, Dual b)
{
    return {
        a.val * b.val,
        (a.d * b.val - a.val * b.d) / (b.val * b.val)
    };
}

Dual sin(Dual x)
{
    return { std::sin(x.val), std::cos(x.val) * x.d };
}

Dual cos(Dual x)
{
    return { std::cos(x.val), -std::sin(x.val) * x.d };
}

Dual exp(Dual x)
{
    double e = std::exp(x.val);
    return { e, e * x.d };
}

} // End namespace fiah