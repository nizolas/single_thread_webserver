#include <vector>
#include <string>
#include "socket.h"

#define OK_200    "200 OK"
#define OK_201    "201 Created"
#define ERROR_400 "400 Bad Request"
#define ERROR_403 "403 Forbidden"
#define ERROR_404 "404 Not Found"
#define ERROR_501 "501 Not Implemented"

class Webserver
{
private:
    char const *ipAddress;
    int port;
    bool debugMode;
    std::vector<std::string> extensionsAllowed;
    std::string root;
    std::string protocol;
    std::string fullFilePathForRequest;

public:
    Webserver(char const *ip, int portNumber, bool debug);
    int startWebserver();
    int loadConfigFile();
    void processGetandHeadRequests(std::string command);
    int validateRequestSyntax(std::string command);
    void sendErrorResponse(int clientFD, std::string responseCode);
};

