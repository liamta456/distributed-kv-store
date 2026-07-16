#include "net/socket_utils.h"

#include <iostream>
#include <optional>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>

#include <winsock2.h>

// Key-value, in-memory store
std::unordered_map<std::string, std::string> store;

/* --- Function Declarations --- */

void put(const std::string &key, const std::string &value);
std::optional<std::string> get(const std::string &key);
bool del(const std::string &key);
std::string processCommand(const std::string &command);

int main() {
    std::cout << "--- KV STORE: STORAGE NODE ---" << std::endl;

    /* --- WSA Setup --- */

    WSADATA wsaData;
    if (initWinsock(wsaData) == false) {
        return 1;
    }
    std::cout << "\nWinsock initialized." << std::endl;

    /* --- Listen for Proxy API Connection --- */

    SOCKET listenSock = createSocket();
    if (listenSock == INVALID_SOCKET) {
        std::cerr << "\nError: Failed to create listening socket." << std::endl;
        cleanupWinsock();
        return 1;
    }
    std::cout << "\nListen socket created." << std::endl;

    unsigned short port = 3000;
    if (bindAndListen(listenSock, port) == false) {
        std::cerr << "Error: Failed to bind and listen from listening socket." << std::endl;
        closeSocket(listenSock);
        cleanupWinsock();
        return 1;
    };
    std::cout << "Listening socket binded and listening on port " << port << "..." << std::endl;

    /* --- Proxy API Connection Loop --- */

    SOCKET proxySock = INVALID_SOCKET;

    while (true) {
        /* --- Accept Proxy API Connection --- */

        proxySock = acceptClient(listenSock);
        if (proxySock == INVALID_SOCKET) {
            std::cerr << "\nError: Failed to accept proxy API connection." << std::endl;
            continue;
        }
        std::cout << "\nProxy API connection established." << std::endl;

        while (true) {
            /* --- Receive Command from Proxy API --- */

            auto data = receiveString(proxySock);
            if (!data.has_value()) {
                std::cout << "Connection closed by proxy API." << std::endl;
                closeSocket(proxySock);
                break;
            }
            std::cout << "Received: " << data.value() << std::endl;

            /* --- Validate Command and Perform Operation if Valid --- */

            std::string res = processCommand(data.value());

            /* --- Send Response to Proxy API --- */

            if (sendString(proxySock, res) == false) {
                std::cerr << "Error: Failed to send response to proxy API." << std::endl;
                closeSocket(proxySock);
                break;
            }
        }
    }

    /* --- Cleanup and Safe Exit --- */

    closeSocket(listenSock);
    closeSocket(proxySock);
    cleanupWinsock();

    return 0;
}

/* --- Function definitions --- */

void put(const std::string &key, const std::string &value) {
    store.insert_or_assign(key, value);
}

std::optional<std::string> get(const std::string &key) {
    auto iterator = store.find(key);
    if (iterator == store.end()) {
        return std::nullopt;
    }
    return iterator->second;
}

// Returns true if KV was deleted, false if not found
bool del(const std::string &key) {
    return store.erase(key) == 1;
}

std::string processCommand(const std::string &command) {
    std::stringstream commandStream(command);
    std::vector<std::string> tokens;
    std::string res = "";

    for (int i = 0; i < 3; i++) {
        std::string token;
        std::getline(commandStream, token, ' ');
        tokens.push_back(token);
    }

    if (tokens[0] == "GET") {
        auto val = get(tokens[1]);

        if (val.has_value()) {
            res = "GET(" + tokens[1] + ") = " + val.value() + "";
        } else {
            res = "Failed to perform GET(" + tokens[1] + ").";
        }

    } else if (tokens[0] == "DEL") {
        if (del(tokens[1])) {
            res = "DEL(" + tokens[1] + ")";
        } else {
            res = "Failed to perform DEL(" + tokens[1] + ").";
        }

    } else if (tokens[0] == "PUT") {
        put(tokens[1], tokens[2]);
        res = "PUT(" + tokens[1] + ", " + tokens[2] + ")";

    } else {
        res = "Failed to process command.";
    }

    return res;
}
