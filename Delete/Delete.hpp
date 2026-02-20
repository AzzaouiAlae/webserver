#pragma once
#include "../Headers.hpp"
#include "../AMethod/AMethod.hpp"

class Delete: AMethod
{
	void deleteFile();
public:
	bool HandleResponse();
};