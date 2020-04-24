#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <conio.h>
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <limits>

using namespace std;

namespace constants
{
	const int kMaxSize = 100;
	const int kDelay = 100;
	const DWORD kDwMaximumSizeLow = 0;
	const DWORD kDwMaximumSizeHigh = kMaxSize;
}

#undef max

void Client();
void Server(char *);