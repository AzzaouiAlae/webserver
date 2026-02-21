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
#include <sys/time.h>
#include <signal.h>
#include <sys/types.h> 
#include <dirent.h>
#include <iomanip>
#include <ctime>

using namespace std;

#define USEC 1000000

#include "Utility.hpp"
#include "Multiplexer.hpp"
#include "AST.hpp"
#include "Logging.hpp"

#include "DeprecatedValidation.hpp"
#include "Validation.hpp"

#include "Error.hpp"
#include "DeprecatedTokenizing.hpp"
#include "Tokenizing/Tokenizing.hpp"

#include "Config.hpp"

#include "DeprecatedParsing.hpp"
#include "Parsing.hpp"

#include "ARequest.hpp"
#include "ClientRequest.hpp"
#include "Multiplexer.hpp"
#include "DeprecatedPath.hpp"
#include "Path.hpp"

#include "IContext.hpp"
#include "AFd.hpp"
#include "Singleton.hpp"

#include "Environment.hpp"
#include "StaticFile.hpp"
#include "ISocket.hpp"

extern char **environ;

#define DEFAULT_CONF "conf/engineX.conf"
