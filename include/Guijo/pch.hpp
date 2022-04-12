#pragma once
#ifdef USE_OPENGL
#include <glad/glad.h>
#endif
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H

#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>

#include <array>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <codecvt>
#include <concepts>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <fstream>
#include <iostream>
#include <list>
#include <locale>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <ranges>
#include <regex>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <variant>
#include <vector>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dwmapi.h>
#include <unknwn.h>
#include <windowsx.h>
#include <ShlObj_core.h>
