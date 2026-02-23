#pragma once
#include "../Headers.hpp"

class DefaultPages
{
	static bool ErrorPages4xx;
	static bool ErrorPages5xx;
	static bool Index;
	static bool AutoIndex;
public:
	static void InitErrorPages4xx();
	static void InitErrorPages5xx();
	static void InitIndex();
	static void InitAutoIndex();
	static void InitDefaultPages();
};
