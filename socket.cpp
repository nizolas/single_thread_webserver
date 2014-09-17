//Some libraries that might be helpful
/*
#include <sys/socket.h>
#include <sys/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string>
*/
//#include <netinet/in.h>
#include <strings.h>
//#include <sys/types.h>
//#include <netdb.h>

#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include "socket.h"
#include <iostream>
#include <cstring>

using namespace std;

Socket::Socket()
{
    clientlen = sizeof(client);
    portset   = 0;
    bzero(&server, sizeof(server));
    bzero(&client, sizeof(client));
}

//=============================================================================
// Name:        init
//
// Description: Initializes struct to be used for creating a socket.
//
// Parameters:  I- ipAddress of the server
//              I- port to be used on the server
//
// Return:      NA
//=============================================================================
void Socket::init(char const *ipAddress, const int port)
{
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    if (!inet_aton(ipAddress, &server.sin_addr))
    {
        cerr << "inet_addr() conversion error" << endl;
    }
}

//=============================================================================
// Name:        startSocket
//
// Description: Creates a socket, binds a name to the socket and then starts
//              listening for socket connections.
//
// Parameters:  I- ipAddress - ip address of the server
//              I- port      - port to be used on the server
//              I- debug     - is debug mode enabled
//
// Return:      0 if successful, otherwise -1
//=============================================================================
int Socket::startSocket(char const *ipAddress, const int port, const bool debug)
{
    int optionValue = 1;

    init(ipAddress, port);
    cout << ipAddress << ":" << port <<endl;

    // Create a socket for communication.
    // ------------------------------------------------------------------------
    serverFD = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFD < 0)
    {
        cerr << "socket" << strerror(errno) << endl;
        return -1;
    }
    else if (debug)
    {
        cout << "Socket has been created successfully" << endl;
    }

    // Set the socket options to use SO_REUSEADDR so that we can get around
    // "Address already in use" error when terminating the server.
    // ------------------------------------------------------------------------
    if (setsockopt(serverFD, SOL_SOCKET, SO_REUSEADDR, &optionValue, sizeof(optionValue)) < 0)
    {
        cerr << "setsockopt: " << strerror(errno) << endl;
        return -1;
    }
    else if (debug)
    {
        cout << "Socket options have been set to SO_REUSEADDR" << endl;
    }

    // Bind a name to the socket created.
    // ------------------------------------------------------------------------
    if (bind(serverFD, (struct sockaddr *) &server, sizeof(server)) < 0)
    {
        cerr << "bind: " << strerror(errno) << endl;
        return -1;
    }
    else if (debug)
    {
        cout << "Name has been binded to the socket successfully" << endl;
    }


    // Start listening for connections.
    // ------------------------------------------------------------------------
    if (listen(serverFD, SOMAXCONN) < 0)
    {
        cerr << "listen: " << strerror(errno) << endl;
        close(serverFD);
        return -1;
    }

    return 0;
}
