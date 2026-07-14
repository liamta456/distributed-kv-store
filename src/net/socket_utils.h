#pragma once

#include <iostream>
#include <optional>
#include <string>

#include <winsock2.h>

bool initWinsock(WSADATA &wsaData);
SOCKET createSocket();
bool bindAndListen(SOCKET listenSock, unsigned short port);
SOCKET acceptClient(SOCKET listenSock);
std::optional<std::string> receiveString(SOCKET senderSock);
bool sendString(SOCKET senderSock, const std::string &data);
void closeSocket(SOCKET sock);
void cleanupWinsock();