#pragma once
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "Gdiplus.lib")

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include "Gdiplus.h"
#include "Gdiplusheaders.h" 

#include <cstdio>
#include <cassert>

#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <memory>

#include "Constants.h"