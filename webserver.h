#include <vector>
#include <string>
#include <queue>
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
    std::string fileFormatForRequest;

public:
    Webserver(char const *ip, int portNumber, bool debug);
    int startWebserver();
    int loadConfigFile();
    void processGetandHeadRequests(int clientFD, std::string command, std::string method);
    int validateRequestSyntax(std::queue<std::string> inputCommands);
    void sendErrorResponse(int clientFD, std::string responseCode);
    void send200OkResponse(int clientFD, std::string responseCode, std::string content, std::string contentLength, std::string method);
};

