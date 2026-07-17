#include <iostream>
#include <optional>

#include <winsock2.h>

bool initWinsock(WSADATA &wsaData) {
    WORD wVersionRequested = MAKEWORD(2, 2);
    int res = WSAStartup(wVersionRequested, &wsaData);
    return (res == 0);
}

SOCKET createSocket() {
    // Create new server socket
    SOCKET sock = INVALID_SOCKET;
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    return sock;
}

bool bindAndListen(SOCKET sock, unsigned short port) {
    if (sock == INVALID_SOCKET) {
        return false;
    }

    // Set up server socket address information
    sockaddr_in sockAddr;
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(port);
    sockAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind server socket to the designated port and address
    if (bind(sock, (sockaddr*) &sockAddr, sizeof(sockAddr)) == SOCKET_ERROR) {
        return false;
    }

    // Listen for connections
    if (listen(sock, SOMAXCONN) == SOCKET_ERROR) {
        return false;
    }

    return true;
}

SOCKET acceptClient(SOCKET sock) {
    if (sock == INVALID_SOCKET) {
        return INVALID_SOCKET;
    }

    // Accept client connection
    sockaddr_in clientSockAddr;
    int clientSockAddrSize = sizeof(clientSockAddr);

    SOCKET clientSock = accept(sock, (sockaddr*) &clientSockAddr, &clientSockAddrSize);
    return clientSock;
}

bool connectSocket(SOCKET sock, unsigned short listenPort) {
    // Set up server address
    sockaddr_in listenSockAddr;
    listenSockAddr.sin_family = AF_INET;
    listenSockAddr.sin_port = htons(listenPort);
    listenSockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to server
    if (connect(sock, (sockaddr*) &listenSockAddr, sizeof(listenSockAddr)) == SOCKET_ERROR) {
        return false;
    }
    
    return true;
}

std::optional<std::string> receiveString(SOCKET sock) {
    if (sock == INVALID_SOCKET) {
        return std::nullopt;
    }

    // Receive and handle data
    char buffer[1024] = {0};
    int numBytesReceived = recv(sock, buffer, sizeof(buffer) - 1, 0);

    if (numBytesReceived > 0) {
        std::string receivedData(buffer, numBytesReceived);
        return receivedData;
    }
    
    return std::nullopt;
}

bool sendString(SOCKET sock, const std::string &data) {
    if (sock == INVALID_SOCKET) {
        return false;
    }

    if (send(sock, data.c_str(), data.size(), 0) == SOCKET_ERROR) {
        return false;
    }
    
    return true;
}

void closeSocket(SOCKET &sock) {
    closesocket(sock);
    sock = INVALID_SOCKET;
}

void cleanupWinsock() {
    WSACleanup();
}

int getLastWinsockError() {
    return WSAGetLastError();
}
