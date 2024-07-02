#pragma once
/**
 * Server and client sockets classes must be handled here
 *
 */

#include <bits/stdc++.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "networkpackets.h"
#include <thread>

#define PORT 6024
#define MAXLINE 1024

using namespace std;

class Messageable
{
public:
    virtual void message(MessageStruct m) = 0;
};

class Server
{

private:
public:
    int sockfd;
    char buffer[MAXLINE];

    bool running = true;

    struct sockaddr_in servaddr, cliaddr;

    socklen_t len;
    int n;

    void watchForMessages(Messageable *messagesreceiver)
    {
        MessageStruct mm;
        while (running)
        {

            n = recvfrom(sockfd, (char *)(char *)buffer, MAXLINE,
                         MSG_WAITALL, (struct sockaddr *)&cliaddr,
                         &len);
            buffer[n] = '\0';

            n = 0;
            strcpy(mm.values, buffer);
            // std::cout << "Server :" << mm.values << std::endl;

            messagesreceiver->message(mm);
        }

        std::cout << "Socket exiting\n";
    }

    Server(Messageable *msgh)
    {

        MessageStruct ms;
        strcpy(ms.values, "SelfTest");
        msgh->message(ms);

        // messagesreceiver = msgh;
        //  Creating socket file descriptor
        if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        {
            cout << "socket creation failed\n";
            // exit(EXIT_FAILURE);
        }

        memset(&servaddr, 0, sizeof(servaddr));
        memset(&cliaddr, 0, sizeof(cliaddr));

        // Filling server information
        servaddr.sin_family = AF_INET; // IPv4
        servaddr.sin_addr.s_addr = INADDR_ANY;
        servaddr.sin_port = htons(PORT);

        // Bind the socket with the server address
        if (bind(sockfd, (const struct sockaddr *)&servaddr,
                 sizeof(servaddr)) < 0)
        {
            cout << "bind failed\n";
            // exit(EXIT_FAILURE);
        }

        len = sizeof(cliaddr); // len is value/result

        std::thread t(&Server::watchForMessages, this, msgh);
        t.detach();
    }

    void closeSocket()
    {
        running = false;
        close(sockfd);
    }

    void send(MessageStruct mm)
    {
        sendto(sockfd, (const char *)mm.values, strlen(mm.values),
               MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
               len);
    }
};

class Client
{

public:
    int sockfd;
    char buffer[MAXLINE];
    const char *hello = "Hello from client";
    struct sockaddr_in servaddr;

    int n;
    socklen_t len;
    bool running = true;

    void watchForMessages(Messageable *messagesreceiver)
    {

        MessageStruct mm;
        while (running)
        {
            // len = sizeof(cliaddr); // len is value/result
            cout << "Waiting message\n";
            n = recvfrom(sockfd, (char *)buffer, MAXLINE,
                         MSG_WAITALL, (struct sockaddr *)&servaddr,
                         &len);
            buffer[n] = '\0';
            n = 0;
            cout << "Received message\n";
            strcpy(mm.values, buffer);
            messagesreceiver->message(mm);
        }
        std::cout << "Socket exiting\n";
    }

    Client(Messageable *msg, std::string targetip)
    {
        // messagesreceiver = msg;

        // Creating socket file descriptor
        if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        {
            cout << "socket creation failed\n";
            // exit(EXIT_FAILURE);
        }

        memset(&servaddr, 0, sizeof(servaddr));

        // Filling server information
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(PORT);
        servaddr.sin_addr.s_addr = inet_addr(targetip.c_str());

        len = sizeof(servaddr); // len is value/result

        /*int n;
        socklen_t len;

        sendto(sockfd, (const char *)hello, strlen(hello),
               MSG_CONFIRM, (const struct sockaddr *)&servaddr,
               sizeof(servaddr));
        std::cout << "Hello message sent." << std::endl;

        n = recvfrom(sockfd, (char *)buffer, MAXLINE,
                     MSG_WAITALL, (struct sockaddr *)&servaddr,
                     &len);
        buffer[n] = '\0';
        std::cout << "Server :" << buffer << std::endl;*/

        std::thread t(&Client::watchForMessages, this, msg);
        t.detach();
    }

    void send(MessageStruct mm)
    {
        sendto(sockfd, (char *)mm.values, strlen(mm.values),
               MSG_CONFIRM, (const struct sockaddr *)&servaddr,
               sizeof(servaddr));
    }

    void closeSocket()
    {
        running = false;
        close(sockfd);
    }
};