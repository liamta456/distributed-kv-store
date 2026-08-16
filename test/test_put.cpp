#include "../src/net/socket_utils.h"

#include <cctype>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>

#include <winsock2.h>

bool processCommand(std::string &command);

int main() {
    std::cout << "--- KV STORE: TEST CLIENT ---" << std::endl;

    /* --- WSA Setup --- */

    WSADATA wsaData;
    if (initWinsock(wsaData) == false) {
        return 1;
    }
    std::cout << "\nWinsock initialized." << std::endl;

    /* --- Establish Proxy Connection --- */

    SOCKET proxySock = createSocket();
    if (proxySock == INVALID_SOCKET) {
        std::cerr << "\nError: Failed to create proxy API socket." << std::endl;
        cleanupWinsock();
        return 1;
    }
    std::cout << "\nProxy API socket created." << std::endl;

    unsigned short proxyPort = 3030;
    if (connectSocket(proxySock, proxyPort) == false) {
        std::cerr << "Error: Failed to connect to proxy API." << std::endl;
        closeSocket(proxySock);
        cleanupWinsock();
        return 1;
    }
    std::cout << "Proxy API connection established." << std::endl;

    /* --- Input Loop --- */

    for (int i = 0; i < 100; i++) {
        std::string command = "put key" + std::to_string(i % 10) +
                              " value" + std::to_string(i);
        processCommand(command);

        /* --- Send Command to Proxy --- */

        if (sendString(proxySock, command) == false) {
            std::cerr << "Error: Failed to send command to proxy API." << std::endl;
            closeSocket(proxySock);
            cleanupWinsock();
            return 1;
        }

        /* --- Receive Response from Proxy --- */

        auto resOpt = receiveString(proxySock);
        if (!resOpt.has_value()) {
            std::cout << "Connection closed by proxy API." << std::endl;
            break;
            
        }
        std::cout << resOpt.value() << std::endl;
    }

    /* --- Cleanup and Safe Exit --- */

    closeSocket(proxySock);
    cleanupWinsock();

    return 0;
}

/**
 * Validates command structure. If successful, returns true and formats command
 * argument. Otherwise, returns false without altering command argument.
 */
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