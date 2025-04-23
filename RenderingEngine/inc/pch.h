#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _WIN32_WINNT 0x0A00
#define ASIO_STANDALONE

#include <algorithm>
#include <array>
#include <asio.hpp>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <DirectXMath.h>
#include <fstream>
#include <future>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <random>
#include "shader.fxh"
#include <string>
#include <thread>
#include <utility>
#include <vector>
#include <Windows.h>

#pragma comment(lib, "d3d12")
#pragma comment(lib, "dxgi")

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 533;
const double ASPECT_RATIO = 1.5;

const double PI = 3.14159265358979323846264338327950288;

inline double DegreesToRadians(double degrees)
{
	return degrees * PI / 180.0;
}