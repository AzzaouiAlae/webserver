#pragma once


#include <map>
#include <vector>
#include <set>
#include <algorithm>
#include <stdlib.h>
#include <climits>
#include <limits>
#include <string>
#include <cstring>
#include <fstream>
#include <cstdlib>
#include <unistd.h>
#include <sstream>
#include <cctype>
#include <fcntl.h>
#include <iostream>

#include <sys/stat.h>
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
#include "Request/Request.hpp"
#include "Multiplexer/Multiplexer.hpp"



#define DEFAULT_CONF "conf/engineX.conf"