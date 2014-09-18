//Some libraries that might be helpful
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <sys/stat.h>

//These ones might be nice if you feel like using C++
#include <string>
#include <iostream>

//used to access files
#include <fstream>
#include <vector>

using namespace std;

int c,s,port, byte;
struct sockaddr_in server, client;
char *ipaddress = "141.117.57.46";
int clientlen = sizeof(client);
int portset=0;
char buffer[512];
string command, filepath, htp, line, configHTP, localDir;
bool correctCommand, postCommand, correctSlash, fileExists, fileRead, correctHTP, correctExtension;
vector<string> extensions;

void getConfigFile()
{
	ifstream configFile("myhttpd.conf");
	if (configFile.is_open())
	{
		getline(configFile,line);
		configHTP = line.substr(0,line.find(" "));
		localDir = line.substr( line.find(" ")+2 ,line.find("]")-line.find(" ")-2);
		cout << "Configuration Hypertext Protocol: " << configHTP <<endl;
		cout << "Configuration Local Directory: " << localDir <<endl;
		
		getline(configFile,line);
		
		while(line.find(" ") < 100)
		{
			extensions.push_back(line.substr(0,line.find(" ")+1));
			line = line.substr(line.find(" ")+1);
		}
		
		extensions.push_back(line);
	}
}

void setSocket()
{
	port = 60704;
	
	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	

	if (!inet_aton(ipaddress, &server.sin_addr))
		cerr << "inet_addr() conversion error\n";

	s = socket(AF_INET, SOCK_STREAM, 0);
	int optionValue = 1;
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &optionValue, sizeof(optionValue));
	if (!s){
		perror("socket");
		exit(0);
	}

	if (bind(s, (struct sockaddr *) &server, sizeof(server)) < 0){
		perror("bind");
		exit(0);
	}
	
	if (listen(s, SOMAXCONN) < 0){
		perror("listen");
		exit(0);
	}
}

int main(){
	//determine the contents of the configuration file
	//get the path mainly
	
	getConfigFile();
	setSocket();
	
	while ((c = accept(s, (struct sockaddr *) &client, (socklen_t *) &clientlen)) > 0){
		// Do whatever a web server does.
		bzero(buffer,512);
		cout << "CONNECTED"<<endl;
		byte = recv(c, buffer, sizeof(buffer),0);
		string input(buffer);
		
		
		input = input.substr(0,input.length()-2);
		cout << "Client Input: " << input << endl;
		command = input.substr(0,input.find(" "));

		if (command.compare("HEAD")==0)
			correctCommand = true;
		else if (command.compare("GET")==0)
			correctCommand = true;
		else if (command.compare("POST")==0)
		{
			correctCommand = true;
			postCommand = true;
		}
		else{
			char *message = "501\n";
			send(c, message, sizeof(message), 0);
		}
	
		if(correctCommand)
		{	
			string temp = input.substr(input.find(" ")+1);
			filepath = temp.substr(0,temp.find(" "));
			temp = temp.substr(temp.find(" ")+1);
			htp= temp.substr(0,temp.find("/"))+temp.substr(temp.find("/")+1,temp.find('\n')-1);
			
			if(filepath.substr(0,1).compare("//"))
			{
				cout << "file path contains / " << endl;
				filepath = filepath.substr(1);
				cout << "File and path: " << filepath <<endl;
				correctSlash = true;
			}
			else
			{
				cout << filepath.substr(0,1) << endl;
				char *message = "400\n";
				send(c, message, sizeof(message), 0);	
			}
			cout << "Hypertext Transfer Protocol: " << htp << endl;
		}
		//check extension first
		int i;
		for(i=0; i< extensions.size();i++)
		{
			if(filepath.substr(filepath.find(".")+1).compare(extensions[i])==0)
			{
				correctExtension = true;
				cout << "The file extension is supported" << endl;
			}
			if(i == extensions.size()-1 and !correctExtension)
			{
				cout << "The file extension is not supported" << endl;
				char *message = "403\n";
				send(c,message,sizeof(message),0);
			}
		}
		//check if the command is post and if the extension is correct
		// bypass file check and file read check
		if(postCommand and correctSlash and correctExtension)
		{
			ofstream newFile(filepath.c_str());
			ifstream createdFile(filepath.c_str());
			if(createdFile.good())
			{
				createdFile.close();
				fileRead = true;
				cout << "File created" <<endl;
			}	
		}
		//check if the file exists for get and head command 
		if(correctSlash and !postCommand and correctExtension)
		{
			
			cout <<filepath.c_str() << endl;
			if(access(filepath.c_str(),F_OK)==0) 
			{
				fileExists = true;
				cout << "File exists" <<endl;
			}
			else
			{
				cout << "File does not exist" <<endl;
				char *message = "404\n";
				send(c,message,sizeof(message),0);
				
			}
		}
		//check if the file has read permission for get and head command
		if(fileExists)
		{
			if(FILE *file = fopen(filepath.c_str(), "r"))
			{
				fclose(file);
				fileRead = true;
				cout << "File is Readable"<<endl;
			}
			else
			{
				cout << "File not Readable" <<endl;
				char *message = "403\n";
				send(c,message,sizeof(message),0);
			}
		}
		//check the HTP
		if(fileRead)
		{
			if(configHTP.compare(htp)==0)
			{
				cout << "Hypertext Transfer Protocol is the same as Config" << endl;
				correctHTP = true;
			}
			else
			{
				cout << "Hypertext Transfer Protocol is not supported" << endl;
				cout <<configHTP<< " and " << htp << endl;
				char *message = "400\n";
				send(c, message, sizeof(message), 0);		
			}
		}
		//take an extra line before continuing to processdd		
		bzero(buffer,512);
		byte = recv(c, buffer, sizeof(buffer),0);
		string input2(buffer);
		
		cout <<input2 <<endl;
		//if all the above is followed return 200
		if(correctHTP)
		{
			cout << "Command is valid (HEAD, GET, or POST)"
			<< " file has leading /, and file exists"
			<<" HTP is correct."
			<< "\nRequest looks good" << endl;
			char *message = "200\n";
			send(c, message, sizeof(message), 0);		
		}
	}
	close(s);
}

