# Proxy-Server
This is a proxy server simulation done in linux operating system.

Creator: [Moeen Abu Katish]

<--Introduction-->
This project aims to provide hands-on experience with socket programming and understanding application-level protocols, specifically HTTP. The HTTP proxy implemented here serves as an intermediary between clients and web servers, enabling various functionalities such as content caching and filtering.

<--Features-->
Accepts a URL input from the user via command-line arguments.
Constructs and sends HTTP requests to external HTTP web servers.
Retrieves and saves files locally.
Displays HTTP responses.
Supports IPv4 connections.
Optionally opens the requested page in a web browser.
HTTP Proxy Overview:
The HTTP proxy client implemented in this project follows a simple workflow:

<--Extra Method : removeLastChar() -->
It removes the last char of the string.


Parses the URL provided by the user.
Checks if the requested file exists locally.
If found, serves the file from the local filesystem.
If not found, constructs an HTTP request and sends it to the server.
Receives the HTTP response from the server.
Saves the file locally.
Displays the response and file size.
Optionally opens the saved file in a web browser.

<--HTTP Request Structure-->
An HTTP request consists of a header and, optionally, a body.
The header includes the request method, request URI, and HTTP version.

Example:
GET /index.html HTTP/1.0\r\nHost: www.example.com\r\n\r\n.

Usage:
Compile the proxy using the provided command:
gcc -Wall -o cproxy cproxy.c

Run the proxy with the following command-line usage:

bash:
./cproxy [URL] [-s]

[URL]: Specifies the URL of the requested object in the format http://hostname[:port]/filepath.
-s: Optional flag to open the retrieved page in a web browser.

Error Handling
Utilizes perror() and herror() for system call errors.
Prints usage error message: "Usage: cproxy <URL> [-s]".

<--Examples-->

Requesting a file from the local filesystem:
File is given from local filesystem
HTTP/1.0 200 OK
Content-Length: 405
<file content>
 Total response bytes: 442

Requesting a file from the server:
HTTP request =
<constructed HTTP request>
LEN = 143
HTTP/1.0 200 OK
Content-Length: 405
<file content>
 Total response bytes: 442

<--Test Case-->
Test the client with various HTTP servers and URLs to ensure correct functionality.






