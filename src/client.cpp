#include "net/socket_utils.h"

#include <cctype>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>

#include <winsock2.h>

bool processCommand(std::string &command);

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

        if (!processCommand(command)) {
            std::cerr << "Error: invalid command.\n" << std::endl;
            continue;
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

bool processCommand(std::string &command) {
    std::stringstream commandStream(command);

    std::string op;
    std::string key;
    std::string value;
    std::string extra;

    if (!(commandStream >> op) || !(commandStream >> key)) {
        return false;
    }

    for (char &c : op) {
        c = std::toupper(static_cast<unsigned char>(c));
    }

    
    if (op == "PUT") {
        if (!(commandStream >> value)) {
            return false;
        }
        if (commandStream >> extra) {
            return false;
        }
        command = op + " " + key + " " + value;

    } else if (op == "GET") {
        if (commandStream >> extra) {
            return false;
        }
        command = op + " " + key;

    } else if (op == "DEL") {
        if (commandStream >> extra) {
            return false;
        }
        command = op + " " + key;

    } else {
        return false;
    }

    return true;
}