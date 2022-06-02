#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <direct.h>
#include <string>

using namespace std;

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "4000"

WSADATA wsaData;
char current_work_dir[FILENAME_MAX];
SOCKET ConnectSocket = INVALID_SOCKET;
struct addrinfo* result = NULL,
    * ptr = NULL,
    hints;
const char* sendbuf;
char recvbuf[DEFAULT_BUFLEN];
int iResult;
int recvbuflen = DEFAULT_BUFLEN;


int connection(int argc, char **argv) {

    if (argc != 2) {
        cout << "usage: %s server-name" << argv[0] << '\n';
        return 1;
    }

    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        cout << "WSAStartup failed with error: " << iResult << '\n';
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        cout << "getaddrinfo failed with error: " << iResult << '\n';
        WSACleanup();
        return 1;
    }

    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            cout << "socket failed with error: " << WSAGetLastError() << '\n';
            WSACleanup();
            return 1;
        }

        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        cout << "Unable to connect to server!\n";
        WSACleanup();
        return 1;
    }

    return 0;
}

int message(string var) {

    sendbuf = var.c_str();

    iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
    if (iResult == SOCKET_ERROR) {
        cout << "send failed with error: " << WSAGetLastError() << '\n';
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    cout << "Отправленное сообщение: " << sendbuf << '\n';   

        iResult = recv(ConnectSocket, recvbuf, sizeof(recvbuf) - 1, 0);
        if (iResult > 0) {
            recvbuf[iResult] = 0;
            cout << "Принятое сообщение: " << recvbuf << '\n';
        }
        else if (iResult == 0)
            cout << "Connection closed\n";
        else
            cout << "recv failed with error: " << WSAGetLastError() << '\n';
    return 0;
}

void command() {

    const char* quit = "bye";
    const char* pwd = "pwd";

    string command;
    while (true) {
        system("cls");
        cout << "Введите команду.\n";
        cout << "Допустимые команды:\n";
        cout << "cd path - смена каталога\n";
        cout << "pwd - текущий каталог\n";
        cout << "bye - выход\n";
        getline(cin, command);
        message(command);
        if (strcmp(command.c_str(), quit) == 0) {
            iResult = shutdown(ConnectSocket, SD_SEND);
            if (iResult == SOCKET_ERROR) {
                cout << "shutdown failed with error: " << WSAGetLastError() << '\n';
                closesocket(ConnectSocket);
                WSACleanup();
            }
            closesocket(ConnectSocket);
            WSACleanup();
            exit(0);
        }
        system("pause");
    }
}



int __cdecl main(int argc, char** argv)
{   
    setlocale(LC_ALL, "Russian");
    if (connection(argc, argv) == 1) {
        return 1;
    };
    command();
  
    return 0;
}
