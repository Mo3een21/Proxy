#include <stdio.h>
#include <sys/socket.h>
#include <ctype.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <unistd.h>
#include "stdlib.h"
#include "netinet/in.h"
#include "netdb.h"
#include "string.h"
#define LENGTH 256


void removeLastChar(char *str) {
    if (str == NULL || str[0] == '\0') {
        // Handle invalid input
        return;
    }

    size_t len = strlen(str);
    if (len > 0) {
        str[len - 1] = '\0';
    }
}

int main(int argc, char *argv[]) {

    if (argc >3 || argc<2) {
        fprintf(stderr, "Usage: %s <URL> [-s]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    if(strlen(argv[1]) <=7){
        fprintf(stderr, "Usage: %s <URL> [-s]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *myCommand=argv[1];
//     char *myCommand="http://www.josephwcarrillo.com";

    char *colonPos = strstr(myCommand, ":");
    char *slashPos = strstr(myCommand+7, "/");
    char *portPosition=strstr(myCommand+7,":");
    char host[LENGTH], filepath[LENGTH];

    memset(host, 0, sizeof(host));
    memset(filepath, 0, sizeof(filepath));


    int port = 80;
    char portArr[100];
    size_t slashPosIndex;

    // Socket and connection setup
    if (colonPos != NULL  && colonPos < portPosition) {//url with port
        sscanf(colonPos+1, "%d", &port);

        strncpy(host, colonPos + 3, (portPosition-1)-(colonPos+2));
        if(portPosition[1]=='\0'){
            fprintf(stderr, "Usage: %s <URL> [-s]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
        if (slashPos != NULL) {
            slashPosIndex = slashPos - portPosition - 1;
        } else {
            slashPosIndex = strlen(myCommand) - (portPosition - myCommand);

        }

        strncpy(portArr, portPosition + 1, slashPosIndex);
        portArr[slashPosIndex] = '\0';
        for (size_t i = 0; i < strlen(portArr); i++) {//here we check if the input has letters in the port section so we handle it
            if (!isdigit(portArr[i])) {
                fprintf(stderr, "Usage: %s <URL> [-s]\n", argv[0]);
                exit(EXIT_FAILURE);
            }
        }
        port= atoi(portArr);
        host[portPosition - myCommand - 7] = '\0';

        if (slashPos == NULL || slashPos[1] == '\0') {
            // No file path or last character is "/", append "/index.html"
            strcat(filepath, "/");
        } else {
            strncpy(filepath, slashPos, LENGTH - 1);
        }

    } else {

        //url without port
        strncpy(host, myCommand + 7, slashPos != NULL ? (slashPos - colonPos) : LENGTH - 1);
        host[slashPos != NULL ? (slashPos - myCommand - 7) : LENGTH - 1] = '\0';
        if (slashPos == NULL || slashPos[1] == '\0') {
            // No file path or last character is "/", append "/index.html"
            strcat(filepath, "/");
        } else {
            strncpy(filepath, slashPos, LENGTH - 1);
        }

    }

    filepath[LENGTH - 1] = '\0';


    char *innerPath;
    char pathCopy[LENGTH];
    char currentPath[LENGTH]; // Adjust the size as needed
    char originalPath[LENGTH];
    memset(currentPath, 0, sizeof(currentPath));
    strcpy(pathCopy,host);
    strcpy(originalPath, pathCopy);// Duplicate the path to avoid modifying the original
    strcat(pathCopy,filepath);
    int found= access(pathCopy,F_OK);//checks if path exists in local file system
    if(found==0){
        if(filepath[strlen(filepath)-1]=='/' ){
            strcat(pathCopy,"index.html");
        }
        FILE *file = fopen(pathCopy, "r"); // Open the file in read mode
        if (file == NULL) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }

        fseek(file, 0, SEEK_END);
        long fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);

        char myRequestString[LENGTH*3];

        printf("File is given from local filesystem\n");
        sprintf(myRequestString, "HTTP/1.0 200 OK\r\nContent-Length: %ld\r\n\r\n",  fileSize);
        printf("%s",myRequestString);

        unsigned char *content = (unsigned char *)malloc(fileSize*sizeof (char)+ 1);
        if (content == NULL) {
            perror("hi");
            fclose(file);
            exit(EXIT_FAILURE);
        }
        if ( fread(content, 1, fileSize, file)!=(size_t) fileSize) {
            perror("fread");
            free(content);
            fclose(file);
            exit(EXIT_FAILURE);
        }

        // Null-terminate the content to make it a valid C string
        content[fileSize] = '\0';

        printf("%s",content);
        long totalBytes = fileSize + strlen(myRequestString);
        printf("\n Total response bytes: %ld\n", totalBytes);
        // Clean up
        free(content);
        fclose(file);
    }

    else {

        int sockfd = -1;
        struct hostent *server_info = NULL;

        /* Create a socket with the address format of IPV4 over TCP */
        if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
            perror("socket failed");
            exit(EXIT_FAILURE);
        }


        const char *host_name = host;


        /* Use gethostbyname to translate host name to network byte order IP address */
        server_info = gethostbyname(host_name);
        if (!server_info) {
            herror("gethostbyname failed");
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        /* Initialize sockaddr_in structure */
        struct sockaddr_in sock_info;

        /* Set its attributes to 0 to avoid undefined behavior */
        memset(&sock_info, 0, sizeof(struct sockaddr_in));

        /* Set the type of the address to be IPV4 */
        sock_info.sin_family = AF_INET;

        /* Set the socket's port */
        sock_info.sin_port = htons(port);

        /* Set the socket's IP */
        sock_info.sin_addr.s_addr = ((struct in_addr *) server_info->h_addr)->s_addr;

        /* Connect to the server */
        if (connect(sockfd, (struct sockaddr *) &sock_info, sizeof(struct sockaddr_in)) == -1) {
            perror("connect failed");
            close(sockfd);
            exit(EXIT_FAILURE);
        }



        // HTTP request construction and sending
        char myRequestString[LENGTH * 3];
        sprintf(myRequestString, "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n", filepath, host);
        printf("%s", myRequestString);
        ssize_t requestData = write(sockfd, myRequestString, strlen(myRequestString));

        if (requestData == -1) {
            perror("send failed");
            // Handle error, close socket, etc.
        } else {
            printf("Sent %zd bytes of data\n", requestData);
        }
        printf("HTTP request =\n%s\nLEN = %lu\n", myRequestString, strlen(myRequestString));

        innerPath = strtok(pathCopy, "/");
        while (innerPath != NULL) {
            strcat(currentPath, innerPath); // Concatenate the token to the current path
            strcat(currentPath, "/"); // Add a slash after each directory name

            // Create the directory
            innerPath = strtok(NULL, "/");
            if (innerPath == NULL && filepath[strlen(filepath) - 1] == '/') {
                mkdir(currentPath, 0777);
            } else if (innerPath != NULL) {
                mkdir(currentPath, 0777);

            } else {
                removeLastChar(currentPath);
            }


        }
        // File creation and data reception
        FILE *file;
        if (filepath[strlen(filepath) - 1] == '/') {
            strcat(currentPath, "/index.html");
            file = fopen(currentPath, "w");
            if (file == NULL) {
                perror("fopen");
                exit(EXIT_FAILURE);
            }

        } else {

            // Create a file for the last item
            file = fopen(currentPath, "w");
            if (file == NULL) {
                perror("fopen");
                exit(EXIT_FAILURE);
            }
        }

        unsigned char received[LENGTH] = "\0";
        ssize_t receivedData;
        bool headersFound = false;
        bool found200OK = false; // Flag to track if "200 OK" is found
        while ((receivedData = recv(sockfd, received, LENGTH, 0)) > 0) {
            // Print what is received on the screen
            fwrite(received, sizeof(char), receivedData, stdout);

            if (!found200OK) {
                if (strstr((char*)received, "200 OK") != NULL) {
                    found200OK = true;
                }
            }

            // Check for "\r\n\r\n" indicating the end of headers
            if (!headersFound) {
                char *endHeaders = strstr((char *)received, "\r\n\r\n");
                if (endHeaders != NULL) {
                    headersFound = true;
                    ssize_t bodyStart = endHeaders - (char *)received + 4;
                    ssize_t bodyLength = receivedData - bodyStart;
                    fwrite(received + bodyStart, sizeof(char), bodyLength, file);
                }
            } else {
                // Write the rest of the data to the file
                fwrite(received, sizeof(char), receivedData, file);
            }
        }



        char command[LENGTH + 8];  // Length of "firefox " plus null terminator
        strcat(command,"firefox ");
        strcat(command,currentPath);
        long totalBytes = sizeof (file) + strlen(myRequestString);
        printf("Total response bytes: %ld\n", totalBytes);

        if (!found200OK) {
            printf("Response does not contain '200 OK', deleting created directories.\n");

            // Delete file
            if (remove(currentPath) == 0) {
                printf("File deleted successfully.\n");
            } else {
                perror("Error deleting file");
            }

            // Delete directories
            char *dirPath = strtok(originalPath, "/");
            char tempPath[LENGTH];
            strcpy(tempPath, dirPath);
            while (dirPath != NULL) {
                dirPath = strtok(NULL, "/");
                if (dirPath != NULL) {
                    strcat(tempPath, "/");
                    strcat(tempPath, dirPath);
                    if (rmdir(tempPath) == 0) {
                        printf("Directory '%s' deleted successfully.\n", tempPath);
                    } else {
                        perror("Error deleting directory");
                    }
                }
            }
        }


        fclose(file);
        close(sockfd);
    }
    return 0;
}







