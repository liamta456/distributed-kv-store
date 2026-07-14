#include "net/socket_utils.h"

#include <iostream>
#include <optional>
#include <string>

#include <winsock2.h>

int main() {
    std::cout << "--- KV STORE: PROXY API ---\n" << std::endl;

    /* --- WSA Setup: Proxy-Server Connection --- */

    WSADATA wsaData;
    if (initWinsock(wsaData) == false) {
        return 1;
    }
    std::cout << "Winsock initialized." << std::endl;

    SOCKET serverSock = createSocket();
    if (serverSock == INVALID_SOCKET) {
        cleanupWinsock();
        return 1;
    }
    std::cout << "Server socket created." << std::endl;

    unsigned short serverPort = 3000;
    if (connectToSocket(serverSock, serverPort) == false) {
        closeSocket(serverSock);
        cleanupWinsock();
        return 1;
    }
    std::cout << "Server connection established.\n" << std::endl;

    /* --- WSA Setup: Client-Proxy Listening --- */

    SOCKET listenSock = createSocket();
    if (listenSock == INVALID_SOCKET) {
        closeSocket(serverSock);
        cleanupWinsock();
        return 1;
    }
    std::cout << "Listen socket created." << std::endl;

    unsigned short listenPort = 3030;
    if (bindAndListen(listenSock, listenPort) == false) {
        closeSocket(serverSock);
        closeSocket(listenSock);
        cleanupWinsock();
        return 1;
    };
    std::cout << "Listen socket binded and listening on port " << listenPort << "..." << std::endl;

    /* --- Main Loop: Data Transfer Between Client and Server --- */

    SOCKET clientSock = INVALID_SOCKET;
    bool cont = true;

    while (cont) {
        clientSock = acceptClient(listenSock);
        if (clientSock == INVALID_SOCKET) {
            closeSocket(listenSock);
            closeSocket(serverSock);
            cleanupWinsock();
            return 1;
        }
        std::cout << "Client connection established.\n" << std::endl;

        while (true) {
            auto clientCommand = receiveString(clientSock);

            if (!clientCommand.has_value()) {
                std::cout << "\nConnection closed by client." << std::endl;
                closeSocket(clientSock);
                std::cout << "Listening on port " << listenPort << "..." << std::endl;
                break;
            }

            std::cout << "Received from client: " << clientCommand.value() << std::endl;
            if (sendString(serverSock, clientCommand.value()) == false) {
                closeSocket(serverSock);
                closeSocket(clientSock);
                closeSocket(listenSock);
                cleanupWinsock();
                return 1;
            }
            std::cout << "Sent to server." << std::endl;

            auto serverRes = receiveString(serverSock);

            if (!serverRes.has_value()) {
                std::cout << "\nConnection closed by server." << std::endl;
                cont = false;
                break;
            }

            std::cout << "Received from server: " << serverRes.value() << std::endl;
            if (sendString(clientSock, serverRes.value()) == false) {
                closeSocket(serverSock);
                closeSocket(clientSock);
                closeSocket(listenSock);
                cleanupWinsock();
                return 1;
            }
            std::cout << "Sent to client." << std::endl;
        }

    }

    /* --- Cleanup and Safe Exit --- */

    closeSocket(serverSock);
    closeSocket(clientSock);
    closeSocket(listenSock);
    cleanupWinsock();
    
    return 0;
}
