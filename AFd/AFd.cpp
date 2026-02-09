#include "../Routing/Routing.hpp"
#include "AFd.hpp"

AFd::AFd(int fd)
{
	this->fd = fd;
}

AFd::operator int() const 
{
    return fd;
}
