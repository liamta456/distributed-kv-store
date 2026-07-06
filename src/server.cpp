#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>

// Key-value, in-memory store
std::unordered_map<std::string, std::string> store;

// Function declarations
void put(const std::string &key, const std::string &value);
std::optional<std::string> get(const std::string &key);
bool del(const std::string &key);

int main() {


    return 0;
}

// Function definitions

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
