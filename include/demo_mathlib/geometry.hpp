// demo_mathlib v1.1.0 — header-only (third-party 역할)
#pragma once
#include <cmath>

namespace demo_mathlib
{

inline double normalize_angle(double a)
{
  while (a > M_PI) {a -= 2.0 * M_PI;}
  while (a < -M_PI) {a += 2.0 * M_PI;}
  return a;
}

inline double distance(double x1, double y1, double x2, double y2)
{
  return std::hypot(x2 - x1, y2 - y1);
}

// v1.1.0: 추가
inline double clamp(double v, double lo, double hi)
{
  return v < lo ? lo : (v > hi ? hi : v);
}

}  // namespace demo_mathlib
