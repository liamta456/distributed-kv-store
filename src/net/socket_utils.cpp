#include <iostream>
#include <optional>

#include <winsock2.h>

bool initWinsock(WSADATA &wsaData) {
    WORD wVersionRequested = MAKEWORD(2, 2);
    int res = WSAStartup(wVersionRequested, &wsaData);

    if (res != 0) {
        std::cerr << "WSAStartup() failed: " << WSAGetLastError() << std::endl;
        return false;
    }

    return true;
}

SOCKET createSocket() {
    // Create new server socket
    SOCKET sock = INVALID_SOCKET;
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sock == INVALID_SOCKET) {
        std::cerr << "socket() failed: " << WSAGetLastError() << std::endl;
    }
    
    return sock;
}

bool bindAndListen(SOCKET listenSock, unsigned short port) {
    if (listenSock == INVALID_SOCKET) {
        std::cerr << "bindAndListen() failed: listenSock argument is invalid." << std::endl;
        return false;
    }

    // Set up server socket address information
    sockaddr_in listenSockAddr;
    listenSockAddr.sin_family = AF_INET;
    listenSockAddr.sin_port = htons(port);
    listenSockAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind server socket to the designated port and address
    if (bind(listenSock, (sockaddr*) &listenSockAddr, sizeof(listenSockAddr)) == SOCKET_ERROR) {
        std::cerr << "bind() failed: " << WSAGetLastError() << std::endl;
        return false;
    }

    // Listen for connections
    if (listen(listenSock, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "listen() failed: " << WSAGetLastError() << std::endl;
        return false;
    }

    return true;
}

SOCKET acceptClient(SOCKET listenSock) {
    if (listenSock == INVALID_SOCKET) {
        std::cerr << "acceptClient() failed: listenSock argument is invalid." << std::endl;
        return INVALID_SOCKET;
    }

    // Accept client connection
    sockaddr_in clientSockAddr;
    int clientSockAddrSize = sizeof(clientSockAddr);

    SOCKET clientSock = accept(listenSock, (sockaddr*) &clientSockAddr, &clientSockAddrSize);
    if (clientSock == INVALID_SOCKET) {
        std::cerr << "accept() failed: " << WSAGetLastError() << std::endl;
    }

    return clientSock;
}

std::optional<std::string> receiveString(SOCKET senderSock) {
    if (senderSock == INVALID_SOCKET) {
        std::cerr << "receiveString() failed: senderSock argument is invalid." << std::endl;
        return std::nullopt;
    }

    // Receive and handle data
    char buffer[1024] = {0};
    int bytesReceived = recv(senderSock, buffer, sizeof(buffer) - 1, 0);

    if (bytesReceived > 0) {
        std::string receivedData(buffer, bytesReceived);
        return receivedData;
    }
    if (bytesReceived == 0) {
        return std::nullopt;
    }
    std::cerr << "recv() failed: " << WSAGetLastError() << std::endl;
    return std::nullopt;
}

bool sendString(SOCKET senderSock, std::string data) {
    if (senderSock == INVALID_SOCKET) {
        std::cerr << "sendString() failed: senderSock argument is invalid." << std::endl;
        return false;
    }

    if (send(senderSock, data.c_str(), data.size(), 0) == SOCKET_ERROR) {
        std::cerr << "send() failed: " << WSAGetLastError() << std::endl;
        return false;
    }
    
    return true;
}

void closeSocket(SOCKET sock) {
    closesocket(sock);
}

void cleanupWinsock() {
    WSACleanup();
}
