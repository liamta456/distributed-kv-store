#include "net/socket_utils.h"

#include <iostream>
#include <optional>
#include <string>

#include <winsock2.h>

int main() {
    std::cout << "--- KV STORE: CLIENT ---\n" << std::endl;

    /* --- WSA Setup --- */

    WSADATA wsaData;
    if (initWinsock(wsaData) == false) {
        return 1;
    }
    std::cout << "Winsock initialized." << std::endl;

    SOCKET proxySock = createSocket();
    if (proxySock == INVALID_SOCKET) {
        cleanupWinsock();
        return 1;
    }
    std::cout << "Proxy API socket created." << std::endl;

    unsigned short proxyPort = 3030;
    if (connectToSocket(proxySock, proxyPort) == false) {
        closeSocket(proxySock);
        cleanupWinsock();
        return 1;
    }
    std::cout << "Proxy API connection established.\n" << std::endl;

    /* --- Input Loop --- */

    while (true) {
        std::cout << "Enter a command (<put|get|del> <key> <value|empty>). Enter without input to finish: " << std::flush;
        std::string command = "";
        std::getline(std::cin, command);

        if (command == "") {
            break;
        }

        if (sendString(proxySock, command) == false) {
            closeSocket(proxySock);
            cleanupWinsock();
            return 1;
        }

        auto res = receiveString(proxySock);
        if (res.has_value()) {
            std::cout << res.value() << "\n"<< std::endl;
        } else {
            std::cout << "Connection closed by proxy API." << std::endl;
            break;
        }  
    }

    /* --- Cleanup and Safe Exit --- */

    closeSocket(proxySock);
    cleanupWinsock();

    return 0;
}