#include <iostream>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <fstream>
#include "webserver.h"

#define IP_ADDRESS "141.117.57.46"  // Replace with the correct IP of the host.
#define CONFIG_FILE "myhttpd.conf"

using namespace std;

Webserver::Webserver(char const *ip, int portNumber, bool debug)
{
    ipAddress = ip;
    port = portNumber;
    debugMode = debug;
}

//=============================================================================
// Name:        startWebserver
//
// Description: Starts the webserver to handle requests from clients.
//
// Parameters:  NA
//
// Return:      0 if successfull. Otherwise -1;
//=============================================================================
int  Webserver::startWebserver()
{
    if (loadConfigFile() < 0)
        return -1;

    Socket socket;

    if (socket.startSocket(IP_ADDRESS, port, debugMode) < 0)
    {
        cerr << "Error: Problem creating a socket. Aborting!" << endl;
    }
    else
    {
        cout << "Server IP Address: " << ipAddress << endl;
        cout << "Server Port Number: " << port << endl;
        cout << "Server is ready and listening for connections" << endl;
    }

    int  byteCount;
    char buffer[1024];
    string command;

    memset(buffer, '\0', sizeof(buffer));

    // Keep accepting requests from remote clients.
    // ------------------------------------------------------------------------
    while ((socket.clientFD = accept(socket.serverFD, (struct sockaddr *) &(socket.client), (socklen_t *) &(socket.clientlen))) > 0)
    {
        // Do whatever a web server does.
        cout << "Connection Established with remote client" << endl;

        // Receive a request from the remote client and process it accordingly.
        // --------------------------------------------------------------------
        byteCount = recv(socket.clientFD, buffer, sizeof(buffer), 0);

        string input(buffer);

        // Get rid of CRLF being set by telnet session.
        // --------------------------------------------------------------------
        input = input.substr(0, input.length() - 2);
        cout << "Client Request:\n" << input << endl;

        command = input.substr(0,input.find(" "));

        if (command.compare("HEAD") == 0 || command.compare("GET") == 0)
            processGetandHeadRequests(input);
        else if (command.compare("POST") == 0)
        {
        /*    correctCommand = true;
            postCommand = true;*/
        }
        else
        {
            // Send 501 Error
        }



        send(socket.clientFD, "Hello World!",13, 0);
        close(socket.clientFD);
    }

}


//=============================================================================
// Name:        processGetandHeadRequests
//
// Description: Processes the GET and HEAD requests and sends the appropriate
//              response message and file back to the remote client.
// 
// Parameters:  I- Command to validate.
//
// Return:      NA
//=============================================================================
void Webserver::processGetandHeadRequests(string command)
{
    string filePath;
    string protocolRequest;
    bool fileExtensionAllowed = false;
    int numberOfSpace = 0;

    // Requests must be of the form:
    //     GET /index.html HTTP/1.0
    // If it's not, return a 400 error message.
    // ------------------------------------------------------------------------

    // Extract the rest of the command after the GET and HEAD without the
    // space.
    // Example: GET /index.html HTTP/1.0 will extract "/index.hml HTTP/1.0"
    // ------------------------------------------------------------------------
    string temp = command.substr(command.find(" ") + 1);

    for (int i = 0; i < temp.length(); i++)
    {
        if (temp[i] == ' ')
            numberOfSpace++;
    }

    if (numberOfSpace != 1)
    {
        cout << "Send 400 error: Only 1 space " << endl;
    }

    // Process the file path requested.
    // ------------------------------------------------------------------------
    filePath = temp.substr(0, temp.find(" "));

    // File path provided does not start with "/" - send a 400 error
    // ------------------------------------------------------------------------
    if (filePath.at(0) != '/')
    {
        cout << "Send 400 error: filepath missing leading /" << endl;
    }
    else
    {
        filePath = filePath.substr(1);
    }

    // Parse the protocol
    // ------------------------------------------------------------------------
    protocolRequest = temp.substr(temp.find(" ") + 1);

    // HTTP/1.0 is missing the "/" - send a 400 error
    // ------------------------------------------------------------------------
    if (protocolRequest.find("/") == string::npos)
    {
        cout << "Send 400 error: HTTP missing /" << endl;
    }
    else
    {
        // Verify that this protocol is compatible with our server.
        // --------------------------------------------------------------------
        protocolRequest = protocolRequest.erase(protocolRequest.find("/"), 1);

        if (protocolRequest.compare(protocol) != 0)
        {
            cout << "Send 400 error: Protocol not compatible" << endl;
        }
    }

    // Protocol is valid and now we need to handle the file requested.
    // ------------------------------------------------------------------------
    int positionPeriod = filePath.find(".");

    // If file does not have an extension, set fileExtensionAllowed to
    // false.
    // ------------------------------------------------------------------------
    if (positionPeriod == string::npos)
    {
        fileExtensionAllowed = false;
    }
    // File has an extension. Check if it is allowed.
    // ------------------------------------------------------------------------
    else
    {
        for (int i = 0; i < extensionsAllowed.size(); i++)
        {
            if (filePath.substr(filePath.find(".") + 1).compare(extensionsAllowed[i]) == 0)
            {
                fileExtensionAllowed = true;
                break;
            }
        }
    }

    // If the file extension is allowed, try to retrieve the file.
    // ------------------------------------------------------------------------
    if (fileExtensionAllowed)
    {

    }
    else
    {
        cout << "Send 404 error: File extension not compatible" << endl;
    }


}


//=============================================================================
// Name:        processRequests
//
// Description: Processes the requests from the remote clients.
//
// Parameters:  I- Ptr to socket used.
//
// Return:      0 if successful. Otherwise -1;
//=============================================================================
/*int Webserver::processRequests(Socket *socket)
{
    int  byteCount;
    char buffer[1024];
    string command;

    memset(buffer, '\0', sizeof(buffer));

    // Keep accepting requests from remote clients.
    // ------------------------------------------------------------------------
    while ((socket->clientFD = accept(socket->serverFD, (struct sockaddr *) &(socket->client), (socklen_t *) &(socket->clientlen))) > 0)
    {
        // Do whatever a web server does.
        cout << "Connection Established with remote client" << endl;

        byteCount = recv(socket->clientFD, buffer, sizeof(buffer), 0);

        string input(buffer);

        // Get rid of CRLF being set by telnet session.
        // --------------------------------------------------------------------
        input = input.substr(0, input.length() - 2);
        cout << "Client Request:\n" << input << endl;

        command = input.substr(0,input.find(" "));

        if (command.compare("HEAD") == 0 || command.compare("GET") == 0)
            processGetandHeadRequests(input);
        else if (command.compare("POST") == 0)
        {
        //    correctCommand = true;
//            postCommand = true;
        }
        else
        {
            // Send 501 Error
        }



        send(socket->clientFD, "Hello World!",13, 0);
    }
}
*/

//=============================================================================
// Name:        loadConfigFile
//
// Description: Loads the configuration file into the webserver in order to
//              process HTTP requests.
//
// Parameters:  NA
//
// Return:      0 if successfull. Otherwise -1.
//=============================================================================
int Webserver::loadConfigFile()
{
    string pathConfigFile, line;

    // Get the complete path to the config file.
    // ------------------------------------------------------------------------
    char *temp = getenv("HOME");

    pathConfigFile.assign(temp);
    pathConfigFile.append("/html/");
    pathConfigFile.append(CONFIG_FILE);

    // Open the config file.
    // ------------------------------------------------------------------------
    if (debugMode)
        cout << "Opening config file: " << pathConfigFile << endl;

    ifstream configFile(pathConfigFile.c_str());

    if (configFile.is_open())
    {
        // Read first line of the config file.
        // The first line must contain the http protocol to be used and the
        // root directory.
        //
        // NOTE: We are not doing error checking for the config file. We are
        //       assuming it is the proper format and just extracting.
        // --------------------------------------------------------------------
        getline(configFile, line);

        // Extract the protocol first and then the root directory.
        // Example:
        // HTTP1.0 [/home/user/html]
        // - To get the protocol, extract from position 0 up to the " " and minus
        //   1 in order to remove the " ".
        // - To get the root directory, extract as from the first character
        //   after the "[" and the size to extract will be calculated with
        //   position of "]" - position of "[" - 1 in order to get the root.
        //
        // FIXME: It might be easier to do that using regex as an improvement
        // --------------------------------------------------------------------

        protocol = line.substr(0, line.find(" "));
        root     = line.substr( line.find("[") + 1,
                                line.find("]") - line.find("[") - 1);

        if (debugMode)
        {
            cout << "Protocol is: " << protocol << endl;
            cout << "Root is: " << root << endl;
        }

        // Read second line of the config file.
        // The second line must contain the file extentions allowed.
        //
        // NOTE: We are not doing error checking for the config file. We are
        //       assuming it is the proper format and just extracting.
        // --------------------------------------------------------------------
        getline(configFile, line);

        // Extract all extensions
        // --------------------------------------------------------------------
        while(line.find(" ") != string::npos)
        {
            extensionsAllowed.push_back(line.substr(0,line.find(" ")));
            line = line.substr(line.find(" ")+1);
        }
        extensionsAllowed.push_back(line);

        if (debugMode)
        {
            cout << "File extentions allowed: " << endl;
            for(int i=0; i< extensionsAllowed.size();i++)
            {
                cout << extensionsAllowed[i] << endl;
            }
            cout << endl;
        }
    }
    else
    {
        cerr << "Error: Unable to read the config file. Aborting!" << endl;
        return -1;
    }

    return 0;
}

void showUsage(const char *arg)
{
    cerr << "Usage: " << arg << " -p [port] -d(optional)" << endl;
}

int main(int argc, char *argv[])
{
    int port;
    bool debugMode = false;

    if (argc < 3 || argc > 4)
    {
        showUsage(argv[0]);
        return 1;
    }

    // Handle passing in wrong value for first argument.
    // ------------------------------------------------------------------------
    if (strcmp(argv[1], "-p") != 0)
    {
        cerr << "First argument is invalid!" << endl;
        showUsage(argv[1]);
        return 1;
    }

    // Handle passing in wrong value for second argument.
    // ------------------------------------------------------------------------
    port = atoi(argv[2]);

    if (port < 60000 || port > 65355)
    {
        cerr << "Second argument is invalid! Allowed range is 60000-65355" << endl;
        return 1;
    }

    // Handle passing in wrong value for third argument.
    // ------------------------------------------------------------------------
    if (argc > 3)
    {
        if (strcmp(argv[3], "-d") != 0)
        {
            cerr << "Third argument is invalid!" << endl;
            showUsage(argv[1]);
            return 1;
        }
        else
        {
            debugMode = true;
            cout << "Debug Mode!" << endl;
        }
    }

    Webserver server(IP_ADDRESS, port, debugMode);
    server.startWebserver();
}
