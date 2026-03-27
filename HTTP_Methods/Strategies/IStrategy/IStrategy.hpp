#pragma once
#include "Headers.hpp"

class IStrategy
{
public:
    virtual ~IStrategy();
    virtual bool Execute() = 0;
};