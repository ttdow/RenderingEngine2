#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <future>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <random>
#include <string>
#include <utility>
#include <vector>

const double PI = 3.1415926535897932385;

inline double DegreesToRadians(double degrees)
{
	return degrees * PI / 180.0;
}