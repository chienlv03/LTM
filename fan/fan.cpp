#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <thread>
#include <chrono>


#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "12345"
#define SERVER_ADDRESS "127.0.0.1"
#define BUFFER_LENGTH 1024

void send_command(const std::string& command) {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return;
    }

    struct addrinfo* addr = NULL, hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    result = getaddrinfo(SERVER_ADDRESS, DEFAULT_PORT, &hints, &addr);
    if (result != 0) {
        std::cerr << "getaddrinfo failed: " << result << std::endl;
        WSACleanup();
        return;
    }

    SOCKET client_socket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if (client_socket == INVALID_SOCKET) {
        std::cerr << "Error at socket(): " << WSAGetLastError() << std::endl;
        freeaddrinfo(addr);
        WSACleanup();
        return;
    }

    result = connect(client_socket, addr->ai_addr, (int)addr->ai_addrlen);
    if (result == SOCKET_ERROR) {
        std::cerr << "connect failed: " << WSAGetLastError() << std::endl;
        closesocket(client_socket);
        freeaddrinfo(addr);
        WSACleanup();
        return;
    }

    freeaddrinfo(addr);

    result = send(client_socket, command.c_str(), (int)command.size(), 0);
    if (result == SOCKET_ERROR) {
        std::cerr << "send failed: " << WSAGetLastError() << std::endl;
        closesocket(client_socket);
        WSACleanup();
        return;
    }

    char buffer[BUFFER_LENGTH];
    result = recv(client_socket, buffer, BUFFER_LENGTH, 0);
    if (result > 0) {
        buffer[result] = '\0';
        std::cout << "Phản hồi từ server: " << buffer << std::endl;
    }

    closesocket(client_socket);
    WSACleanup();
}

int main() {
    // Chạy vòng lặp lắng nghe và phản hồi từ server
    while (true) {
        send_command("status fan");
        std::this_thread::sleep_for(std::chrono::seconds(5));  // Lặp lại sau mỗi 5 giây
    }
    return 0;
}
