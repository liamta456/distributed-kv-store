#pragma once

#include <iostream>
#include <optional>
#include <string>

#include <winsock2.h>

bool initWinsock(WSADATA &wsaData);
SOCKET createSocket();
bool bindAndListen(SOCKET sock, unsigned short port);
SOCKET acceptClient(SOCKET sock);
bool connectSocket(SOCKET sock, unsigned short listenPort);
std::optional<std::string> receiveString(SOCKET sock);
bool sendString(SOCKET sock, const std::string &data);
void closeSocket(SOCKET &sock);
void cleanupWinsock();
int getLastWinsockError();