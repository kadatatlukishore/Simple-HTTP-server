#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

// For HTTP server

//1. Create a socket.
//2. Bind the socket.
//3. Listen on the socket for a client.
//4. Accept a connection from a client.
//5. Receive and send data.
//6. Disconnect.


/*
** Handles the client requests.
** Since it's a simple HTTP server we're gonna send some test response
*/
void handle_client(SOCKET clientSocket) {
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    int result = recv(clientSocket, recvbuf, recvbuflen, 0);

    if (result > 0) {
        std::cout << "Bytes received: " << result << std::endl;

        // Respond to the client
        const char* httpResponse =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: 13\r\n"
            "\r\n"
            "Hello, World!";

        send(clientSocket, httpResponse, (int)strlen(httpResponse), 0);
    }
    else if (result == 0) {
        std::cout << "Connection closing..." << std::endl;
    }
    else {
        std::cerr << "recv failed: " << WSAGetLastError() << std::endl;
    }

    closesocket(clientSocket);
}

int main() {

    struct addrinfo* result = NULL;
    struct addrinfo hints;

    int iResult;

    // Initialize Winsock.
    WSADATA wsaData;
    int rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (rc != 0) {
        std::cerr << "WSAStartup failed: " << rc << std::endl;
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        std::cerr << "getaddrinfo failed with error: " << iResult << std::endl;
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    //1. Create a socket.
    SOCKET listening_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    if (listening_socket == INVALID_SOCKET) {
        std::cerr << "Cannot create the socket" << std::endl;
        freeaddrinfo(result);
        WSACleanup();
        return -1;
    }

   
    //2. Bind the socket.
    if (bind(listening_socket, result->ai_addr, (int)result->ai_addrlen) == INVALID_SOCKET)
    {
        std::cerr << "bind() failed: " << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        WSACleanup();
        return -1;
    }

    freeaddrinfo(result);

    //3. Listen on the socket for a client.
    if (listen(listening_socket, 5 /*backlog*/) == INVALID_SOCKET) {
        std::cerr << "listen() failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return -1;
    }

    //4. Accept a connection from a client.
    while (TRUE) {
        SOCKET client_socket = accept(listening_socket, NULL, NULL);
        if (client_socket == INVALID_SOCKET) {
            std::cerr << "accept failed: " << WSAGetLastError() << std::endl;
            closesocket(listening_socket);
            WSACleanup();
            return -1;
        }

        //5. Receive and send data.
        handle_client(client_socket);
    }

    //6. Disconnect.
    closesocket(listening_socket);
    WSACleanup();

    return 0;
}

