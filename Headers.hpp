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

#include "utility/Utility.hpp"
#include "Multiplexer/Multiplexer.hpp"
#include "AbstractSyntaxTree/AST.hpp"
#include "Logging/Logging.hpp"

#include "Deprecated/DeprecatedValidation/DeprecatedValidation.hpp"
#include "Validation/Validation.hpp"

#include "ErrorHandling/Error.hpp"
#include "Deprecated/DeprecatedTokenizing/DeprecatedTokenizing.hpp"
#include "Tokenizing/Tokenizing.hpp"

#include "Config/Config.hpp"

#include "Deprecated/DeprecatedParsing/DeprecatedParsing.hpp"
#include "Parsing/Parsing.hpp"

#include "ARequest/ARequest.hpp"
#include "ClientRequest/ClientRequest.hpp"
#include "Multiplexer/Multiplexer.hpp"
#include "Deprecated/DeprecatedPath/DeprecatedPath.hpp"
#include "Path/Path.hpp"

#include "IContext/IContext.hpp"
#include "AFd/AFd.hpp"
#include "Singleton/Singleton.hpp"

#include "Environment/Environment.hpp"
#include "StaticFile/StaticFile.hpp"
#include "ISocket/ISocket.hpp"

extern char **environ;

#define DEFAULT_CONF "conf/engineX.conf"
