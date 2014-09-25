#include <netinet/in.h>
#include <unistd.h>

class Socket
{
public:
    // Private member variables.
    int clientFD;
    int serverFD;
    int clientlen;
    int portset;
    struct sockaddr_in server, client;

    Socket();
    void init(char const *ipAddress, const int port);
    int startSocket(char const *ipAddress, const int port, const bool debug = false);
};
