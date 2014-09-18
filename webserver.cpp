#include <iostream>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <limits.h>
#include <fstream>
#include "webserver.h"

#define IP_ADDRESS "141.117.57.46"  // Replace with the correct IP of the host.
#define CONFIG_FILE "myhttpd.conf"

using namespace std;


//=============================================================================
// Name:        getFormattedDate
//
// Description: Helper function to create the formatted date for the response.
//
// Parameters:  NA
//
// Return:      NA
//=============================================================================
string getFormattedDate()
{
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = gmtime(&rawtime);

    char buffer[256];

    // Format is: Date: Thu, 18 Sep 2014 04:44:26 GMT
    // ------------------------------------------------------------------------
    strftime(buffer, 256, "Date: %a, %d %b %Y %X GMT\r\n", timeinfo);

    string formattedData = string(buffer);

    return formattedData;
}

//=============================================================================
// Name:        getLastModified
//
// Description: Helper function to create the formatted date for last modified
//              timestamp.
//
// Parameters:  I- file name
//
// Return:      NA
//=============================================================================
string getLastModified(string filename)
{
    struct tm *timeinfo;
    struct stat attribute;
    stat(filename.c_str(), &attribute);
    timeinfo = gmtime(&(attribute.st_mtime));

    char buffer[256];

    // Format is: Date: Thu, 18 Sep 2014 04:44:26 GMT
    // ------------------------------------------------------------------------
    strftime(buffer, 256, "Last-Modified: %a, %d %b %Y %X GMT\r\n", timeinfo);

    string formattedData = string(buffer);

    return formattedData;
}

//=============================================================================
// Name:        getContentType
//
// Description: Helper function to create the formatted string for the content
//              type.
//
// Parameters:  I- file name
//
// Return:      NA
//=============================================================================
string getContentType(string fileFormatForRequest)
{
    if (   fileFormatForRequest.compare("html") == 0
        || fileFormatForRequest.compare("HTML") == 0
        || fileFormatForRequest.compare("htm") == 0 )
    {
        return "Content-Type: text/html; charset=iso-8859-1\r\n";
    }
}



// Constructor
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

        // Validate the syntax.
        // If it's incorrect, it will send 400 error.
        // --------------------------------------------------------------------
        if (validateRequestSyntax(input) < 0)
        {
            if (debugMode)
                cout << "Sending 400 error" << endl;
            sendErrorResponse(socket.clientFD, ERROR_400);
        }
        else
        {
            // Syntax is correct, try processing the request.
            // First verify that we the method is implemented.
            // ----------------------------------------------------------------
            command = input.substr(0, input.find(" "));

            if (command.compare("HEAD") == 0 || command.compare("GET") == 0)
            {
                processGetandHeadRequests(socket.clientFD, input, command);
            }
            else if (command.compare("POST") == 0)
            {
            /*    correctCommand = true;
                postCommand = true;*/
            }
            else
            {
                if (debugMode)
                    cout << "Sending 501 error" << endl;
                sendErrorResponse(socket.clientFD, ERROR_501);
            }

        }

        close(socket.clientFD);
    }
    return 0;
}

//=============================================================================
// Name:        validateRequestSyntax
//
// Description: Validate the request syntax.
//
// Parameters:  I- the request to validate.
//
// Return:      0 if successfull. Otherwise -1;
//=============================================================================
int Webserver::validateRequestSyntax(string command)
{
    string temp;
    string filePath;
    string protocolRequest;
    int    numberOfSpace    = 0;
    bool   errorEncountered = false;

    // Requests should be in the following form:
    //     GET /index.html HTTP/1.0
    // If it's not in this form, send 400 error.
    // ------------------------------------------------------------------------
    for (int i = 0; i < command.length(); i++)
    {
        if (command[i] == ' ')
            numberOfSpace++;
    }
    if (numberOfSpace != 2)
    {
        errorEncountered    = true;

        if (debugMode)
            cout << "Request Syntax Error: Invalid number of arguments" << endl;
    }

    if (!errorEncountered)
    {
        // Extract the rest of the command after the method without the space.
        // Example: GET /index.html HTTP/1.0 will extract "/index.hml HTTP/1.0"
        // ------------------------------------------------------------------------
        temp = command.substr(command.find(" ") + 1);

        // Process the file path requested.
        // ------------------------------------------------------------------------
        filePath = temp.substr(0, temp.find(" "));

        // File path provided does not start with "/" - send a 400 error
        // ------------------------------------------------------------------------
        if (filePath.at(0) != '/')
        {
            errorEncountered            = true;

            if (debugMode)
                cout << "Request Syntax Error: file path missing leading /" << endl;
        }
    }

    if (!errorEncountered)
    {
        // Parse the protocol
        // ------------------------------------------------------------------------
        protocolRequest = temp.substr(temp.find(" ") + 1);

        // HTTP/1.0 is missing the "/" - send a 400 error
        // ------------------------------------------------------------------------
        if (protocolRequest.find("/") == string::npos)
        {
            errorEncountered     = true;

            if (debugMode)
                cout << "Request Syntax Error: HTTP missing /" << endl;
        }
        else
        {
            // Verify that this protocol is compatible with our server.
            // --------------------------------------------------------------------
            if (protocolRequest.compare(protocol) != 0)
            {
                errorEncountered = true;

                if (debugMode)
                    cout << "Request Syntax Error: Protocol not compatible" << endl;
            }
        }
    }

    if (!errorEncountered)
    {
        // Protocol is valid and now we need to handle the file requested.
        // ------------------------------------------------------------------------
        bool fileExtensionAllowed = false;
        int  positionPeriod       = filePath.find(".");

        // If file does not have an extension, set fileExtensionAllowed to
        // false.
        // ------------------------------------------------------------------------
        if (positionPeriod == string::npos)
        {
            errorEncountered = true;

            if (debugMode)
                cout << "Request Syntax Error: Missing extension for the file" << endl;
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
                    fileFormatForRequest = extensionsAllowed[i];
                    break;
                }
            }
            if (!fileExtensionAllowed && debugMode)
            {
                errorEncountered = true;
                cout << "Request Syntax Error: File extension not supported" << endl;
            }
        }
    }

    if (errorEncountered)
    {
        return -1;
    }
    else
    {
        // Set the full path name for the request.
        // --------------------------------------------------------------------
        fullFilePathForRequest.append(root);
        fullFilePathForRequest.append(filePath);
    }

    return 0;
}

//=============================================================================
// Name:        processGetandHeadRequests
//
// Description: Processes the GET and HEAD requests and sends the appropriate
//              response message and file back to the remote client.
//
// Parameters:  I- the file descriptor of the client
//              I- Command to validate.
//              I- method used.
//
//
// Return:      NA
//=============================================================================
void Webserver::processGetandHeadRequests(int clientFD, string command, string method)
{
    FILE *file;
    long size;
    char *buffer;
    size_t numElements;

    // File extension check has already been done in validateRequestSyntax().
    // All we need to do verify if file exists and if it it accessible. Then
    // send response code accordingly.
    // - 200 OK if file is readable
    // - 403 Forbidden if file does not have read permission
    // - 404 Not Found if file does not exist
    // ------------------------------------------------------------------------
    if (access(fullFilePathForRequest.c_str(), F_OK) == 0)
    {
        // File is readable.
        // --------------------------------------------------------------------
        if(file = fopen(fullFilePathForRequest.c_str(), "r"))
        {
            // Get file size
            // ----------------------------------------------------------------
            fseek(file , 0 , SEEK_END);
            size = ftell(file);
            rewind(file);

            // Allocate memory for the whole file and start reading.
            // ----------------------------------------------------------------
            buffer = (char*) malloc (sizeof(char)*size);
            numElements = fread (buffer, 1, size, file);
            fclose(file);

            string content(buffer);
            char temp[512];
            sprintf(temp, "%d", content.length());

            if (debugMode)
                cout << "Send 200 OK" << endl;
            send200OkResponse(clientFD, OK_200, buffer, temp, method);
        }
        // File is not readable.
        // --------------------------------------------------------------------
        else
        {
            if (debugMode)
                cout << "Send 403 error: File not Readable" <<endl;
            sendErrorResponse(clientFD, ERROR_403);
        }
    }
    // File does not exist.
    // ------------------------------------------------------------------------
    else
    {
        if (debugMode)
            cout << "Send 404 error: File does not exist" <<endl;
        sendErrorResponse(clientFD, ERROR_404);
    }
}


//=============================================================================
// Name:        sendErrorResponse
//
// Description: Sends the appropriate error response.
//
// Parameters:  I- the client file descriptor.
//              I- the response code to send.
//
// Return:      0 if successfull. Otherwise -1.
//=============================================================================
void Webserver::sendErrorResponse(int clientFD, string responseCode)
{
    string response = protocol;
    response.append(" ");
    response.append(responseCode);
    response.append("\r\n");
    response.append(getFormattedDate());
    response.append("Server: AwesomeServer 1.0\r\n");
    response.append("Connection: close\r\n");

    send(clientFD, response.c_str(), response.length(), 0);

}


//=============================================================================
// Name:        send200OkResponse
//
// Description: Sends the appropriate error response.
//
// Parameters:  I- the client file descriptor.
//              I- the response code to send.
//              I- the content of the file.
//              I- the content length.
//              I- the method used.
//
// Return:      0 if successfull. Otherwise -1.
//=============================================================================
void Webserver::send200OkResponse(int clientFD,
                                  string responseCode,
                                  string content,
                                  string contentLength,
                                  string method)
{

    string response = protocol;
    response.append(" ");
    response.append(responseCode);
    response.append("\r\n");
    response.append(getFormattedDate());
    response.append("Server: AwesomeServer 1.0\r\n");
    response.append(getLastModified(fullFilePathForRequest));
    response.append("Content-Length: ");
    response.append(contentLength);
    response.append("\r\n");
    response.append("Connection: close\r\n");
    response.append(getContentType(fileFormatForRequest));
    response.append("\r\n");

    // Add the content of the file only if it is a GET request
    // ------------------------------------------------------------------------
    if (method.compare("GET") == 0)
    {
        response.append(content);
    }

    send(clientFD, response.c_str(), response.length(), 0);
}


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
        //   1 in order to remove the " ". Add a "/" after HTTP to make it
        //   follow the protocol.
        // - To get the root directory, extract as from the first character
        //   after the "[" and the size to extract will be calculated with
        //   position of "]" - position of "[" - 1 in order to get the root.
        //
        // FIXME: It might be easier to do that using regex as an improvement
        // --------------------------------------------------------------------

        protocol = line.substr(0, line.find(" "));
        protocol = protocol.insert(4, "/");
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

    configFile.close();
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
