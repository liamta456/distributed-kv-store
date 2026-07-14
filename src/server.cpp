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
std::string processCommand(const std::string command);

int main() {
    std::cout << "--- KV STORE: STORAGE SERVER ---\n" << std::endl;

    /* --- WSA Setup --- */

    WSADATA wsaData;
    if (initWinsock(wsaData) == false) {
        return 1;
    }
    std::cout << "Winsock initialized." << std::endl;

    SOCKET listenSock = createSocket();
    if (listenSock == INVALID_SOCKET) {
        cleanupWinsock();
        return 1;
    }
    std::cout << "Listen socket created." << std::endl;

    unsigned short port = 3000;
    if (bindAndListen(listenSock, port) == false) {
        closeSocket(listenSock);
        cleanupWinsock();
        return 1;
    };
    std::cout << "Listen socket binded and listening on port " << port << "..." << std::endl;

    SOCKET proxySock = acceptClient(listenSock);
    if (proxySock == INVALID_SOCKET) {
        closeSocket(listenSock);
        cleanupWinsock();
        return 1;
    }
    std::cout << "Proxy API connection established.\n" << std::endl;

    /* --- Client Connection Loop --- */

    while (true) {
        auto data = receiveString(proxySock);
        if (data.has_value()) {
            std::cout << "Received: " << data.value() << std::endl;
            std::string res = processCommand(data.value());

            sendString(proxySock, res);

        } else {
            std::cout << "\nConnection closed by proxy API." << std::endl;
            break;
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

std::string processCommand(const std::string command) {
    std::stringstream commandStream(command);
    std::vector<std::string> tokens;
    std::string res = "";

    for (int i = 0; i < 3; i++) {
        std::string token;
        std::getline(commandStream, token, ' ');
        tokens.push_back(token);
    }

    if (tokens[0] == "get") {
        auto val = get(tokens[1]);

        if (val.has_value()) {
            res = "Successfully performed GET: key=" + tokens[1] + " value=" + val.value();
        } else {
            res = "Failed to perform GET: key=" + tokens[1];
        }

    } else if (tokens[0] == "del") {
        if (del(tokens[1])) {
            res = "Successfully performed DEL: key=" + tokens[1];
        } else {
            res = "Failed to perform DEL: key=" + tokens[1];
        }

    } else if (tokens[0] == "put") {
        put(tokens[1], tokens[2]);
        res = "Successfully performed PUT: key=" + tokens[1] + " value=" + tokens[2];

    } else {
        res = "Failed to process command.";
    }

    return res;
}
