#include "Singleton.hpp"

Singleton::Map Singleton::GetEnv()
{
    static Map env;

    return env;
}
