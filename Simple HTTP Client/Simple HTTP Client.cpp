#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

// For HTTP server

//1. Create a socket.
//2. Connect to a server.
//3. Receive and send data.
//4. Disconnect/shutdown


int main(int argc, char** argv) {
    
    WSADATA wsaData;
    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;

    SOCKET connecting_socket = INVALID_SOCKET;

    const char* sendbuf = "Test buffer";
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;
    std::string response_buffer;


    // Validate the parameters
    if (argc != 2) {
        std::cerr << "usage: %s server-name" << argv[0] << std::endl;
        return 1;
    }

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cerr << "WSAStartup failed with error: " << iResult << std::endl;
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        std::cerr << "getaddrinfo failed with error: " << iResult << std::endl;
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        // Create a SOCKET for connecting to server
        connecting_socket = socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol);
        if (connecting_socket == INVALID_SOCKET) {
            std::cerr << "socket failed with error: " << WSAGetLastError() << std::endl;
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect(connecting_socket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(connecting_socket);
            connecting_socket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (connecting_socket == INVALID_SOCKET) {
        std::cerr << "Unable to connect to server!" << std::endl;
        WSACleanup();
        return 1;
    }

    // Send an initial buffer
    iResult = send(connecting_socket, sendbuf, (int)strlen(sendbuf), 0);
    if (iResult == SOCKET_ERROR) {
        std::cerr << "send failed with error: " << WSAGetLastError() << std::endl;
        closesocket(connecting_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "Bytes Sent: " << iResult << std::endl;

    // shutdown the connection since no more data will be sent
    iResult = shutdown(connecting_socket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        std::cerr << "shutdown failed with error: " << WSAGetLastError() << std::endl;
        closesocket(connecting_socket);
        WSACleanup();
        return 1;
    }

    // Receive until the peer closes the connection
    do {

        iResult = recv(connecting_socket, recvbuf, recvbuflen, 0);
        if (iResult > 0)
        {
            response_buffer.append(recvbuf, iResult);
            std::cout << "Bytes received: " << iResult << std::endl;
            std::cout << "Received data " << response_buffer.c_str() << std::endl;
        }
        else if (iResult == 0)
            std::cout << "Connection closed" << std::endl;
        else
            std::cerr << "recv failed with error: " << WSAGetLastError() << std::endl;

    } while (iResult > 0);

    //6. Disconnect.
    closesocket(connecting_socket);
    WSACleanup();

    return 0;
}

