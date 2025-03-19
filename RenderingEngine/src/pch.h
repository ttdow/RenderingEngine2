#pragma once

#include <chrono>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <random>
#include <vector>
#include <future>
#include <memory>
#include <optional>
#include <utility>

const double INF = std::numeric_limits<double>::infinity();
const double PI = 3.1415926535897932385;

inline double DegreesToRadians(double degrees)
{
	return degrees * PI / 180.0;
}