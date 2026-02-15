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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/sendfile.h>
#include <sys/wait.h>
#include <netinet/tcp.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <signal.h>


using namespace std;

#define USEC 1000000

#include "utility/Utility.hpp"
#include "Multiplexer/Multiplexer.hpp"
#include "AbstractSyntaxTree/AST.hpp"
#include "Validation/Validation.hpp"
#include "ErrorHandling/Error.hpp"
#include "Tokenizing/Tokenizing.hpp"
#include "Parsing/Parsing.hpp"
#include "Request/Request.hpp"
#include "Multiplexer/Multiplexer.hpp"
#include "Path/Path.hpp"
#include "IContext/IContext.hpp"
#include "AFd/AFd.hpp"
#include "Singleton/Singleton.hpp"
#include "Logging/Logging.hpp"
#include "Environment/Environment.hpp"

extern char **environ;

#define DEFAULT_CONF "conf/engineX.conf"