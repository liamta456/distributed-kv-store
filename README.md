# Distributed Key-Value Store
Windows-based program that allows a client to perform PUT, GET, and DEL key-value operations on a distributed set of storage nodes via a proxy API using consistent hashing.

## Author
Liam Ta

## Tech Stack
- C++
- Winsock2
- Windows

## Features
- In-Memory Key Value Storage
- Distributed Node System
- Consistent Hashing Ring
- TCP Socket Networking via Winsock2
- Proxy Routing
- Space-Delimited Command Protocol
- Client CLI

## Architecture

Client <--> Proxy API <--> Storage Nodes

### Applications
Client
- Creates a socket to connect to the proxy API
    - Retrieves a command input from the user
    - Validates the command
    - Sends the command to the proxy
    - Receives a response from the proxy

Proxy API
- Creates a socket to listen for client connections
    - Accepts a client connection
        - Receives the client command
        - Chooses the proper storage node to interact with via consistent hashing
        - Connects to the proper node
        - Sends the command to the node
        - Receives a response from the node
        - Sends the response to the client
        - Closes connection to the node

Storage Node
- Creates a socket to listen for the proxy connection
    - Accepts the proxy connection
    - Receives the command from the proxy
    - Validates the command
    - Performs the desired operation
    - Sends a response to the proxy

### Client Command Structure
`<put|get|del> <key> <value>`
- `<value>` is omitted for `get` and `del`.

## To Run
- Utilize PowerShell.
- Compile each file into a `bin/` directory.
    - `g++ src/client.cpp src/net/socket_utils.cpp -o bin/client.exe -lws2_32`
    - `g++ src/proxy.cpp src/net/socket_utils.cpp -o bin/proxy.exe -lws2_32`
    - For each node file with name `<nodefile>`:
        - `g++ src/nodes/<nodefile>.cpp src/net/socket_utils.cpp -o bin/<nodefile>.exe -lws2_32`
- Launch the generated executables in `bin/`.
    - `Get-ChildItem ./bin/*.exe | ForEach-Object { Start-Process $_.FullName }`