#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <unordered_map>
#include <string>
#include <sstream>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "12345"
#define BUFFER_LENGTH 1024

std::unordered_map<std::string, bool> device_status = {
    {"light", false},
    {"fan", false}
};

void handle_client(SOCKET client_socket) {
    char buffer[BUFFER_LENGTH];
    int result = recv(client_socket, buffer, BUFFER_LENGTH, 0);
    if (result > 0) {
        buffer[result] = '\0';
        std::string command(buffer);
        std::istringstream iss(command);
        std::string action, device;
        iss >> action >> device;

        if (action == "on") {
            device_status[device] = true;
        }
        else if (action == "off") {
            device_status[device] = false;
        }

        std::string response = device + " is " + (device_status[device] ? "on" : "off");
        send(client_socket, response.c_str(), response.size(), 0);
    }
    closesocket(client_socket);
}

int main() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return 1;
    }

    struct addrinfo* addr = NULL, hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    result = getaddrinfo(NULL, DEFAULT_PORT, &hints, &addr);
    if (result != 0) {
        std::cerr << "getaddrinfo failed: " << result << std::endl;
        WSACleanup();
        return 1;
    }

    SOCKET listen_socket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if (listen_socket == INVALID_SOCKET) {
        std::cerr << "Error at socket(): " << WSAGetLastError() << std::endl;
        freeaddrinfo(addr);
        WSACleanup();
        return 1;
    }

    result = bind(listen_socket, addr->ai_addr, (int)addr->ai_addrlen);
    if (result == SOCKET_ERROR) {
        std::cerr << "bind failed: " << WSAGetLastError() << std::endl;
        freeaddrinfo(addr);
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(addr);

    result = listen(listen_socket, SOMAXCONN);
    if (result == SOCKET_ERROR) {
        std::cerr << "listen failed: " << WSAGetLastError() << std::endl;
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server is listening on port " << DEFAULT_PORT << std::endl;

    while (true) {
        SOCKET client_socket = accept(listen_socket, NULL, NULL);
        if (client_socket == INVALID_SOCKET) {
            std::cerr << "accept failed: " << WSAGetLastError() << std::endl;
            closesocket(listen_socket);
            WSACleanup();
            return 1;
        }
        handle_client(client_socket);
    }

    closesocket(listen_socket);
    WSACleanup();
    return 0;
}
