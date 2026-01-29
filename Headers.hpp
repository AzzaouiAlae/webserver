#pragma once

#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <algorithm>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <fstream>
#include <cstdlib>
#include <unistd.h>
#include <cctype>
using namespace std;

#define USEC 1000000

#include "utility/Utility.hpp"
#include "Multiplexer/Multiplexer.hpp"
#include "AbstractSyntaxTree/AST.hpp"
#include "Validation/Validation.hpp"
#include "ErrorHandling/Error.hpp"
#include "Singleton/Singleton.hpp"
#include "Tokenizing/Tokenizing.hpp"
#include "Parsing/Parsing.hpp"
#include "Socket/socket.hpp"
#include "request/request.hpp"
#include "Multiplexer/Multiplexer.hpp"



#define DEFAULT_CONF "conf/engineX.conf"