CPS730_WebTech
==============

web tech projects

Assignment 1 - Single thread webserver

To test on a browser
- have a VM running linux or better is to login on a Linux machine at home.
- change the IP address
- change the location of the config file
- change the config file so that it accepts HTTP/1.1 instead of HTTP/1.0
- start the server
- on a browser, type in ipAddress:portNumber/filename

200 OK -> will display the html file
Any error -> will display the Code followed by the explanation

Example:
400 Bad Request

Testing for POST.
Currently working for only one line of data to be written to the file.

Example:
POST /foo.html HTTP/1.0
Content-Length: 10
1234567890

This will create a file foo.html in the root with content "1234567890"
