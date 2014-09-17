#include <vector>
#include <string>
#include "socket.h"

class Webserver
{
private:
    char const *ipAddress;
    int port;
    bool debugMode;
    std::vector<std::string> extensionsAllowed;
    std::string root;
    std::string protocol;

public:
    Webserver(char const *ip, int portNumber, bool debug);
    int startWebserver();
    int loadConfigFile();
    int processRequests(Socket *socket);
};

