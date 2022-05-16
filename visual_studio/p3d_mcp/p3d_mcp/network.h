#include <WinSock2.h>
#include <WS2tcpip.h>
#include <system_error>
#include <string>
#include <iostream>

#pragma once

class WSASession
{
public:
    WSASession()
    {
        int ret = WSAStartup(MAKEWORD(2, 2), &data);
        if (ret != 0)
            throw std::system_error(WSAGetLastError(), std::system_category(), "WSAStartup Failed");
    }
    ~WSASession()
    {
        WSACleanup();
    }
private:
    WSAData data;
};

class UDPSocket
{
public:
    UDPSocket()
    {
        sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock == INVALID_SOCKET)
            throw std::system_error(WSAGetLastError(), std::system_category(), "Error opening socket");
    }
    ~UDPSocket()
    {
        closesocket(sock);
    }

    sockaddr_in mcp;
    bool got_addr = 0;

    void SendTo(sockaddr_in& address, const char* buffer, int len, int flags = 0)
    {
        int ret = sendto(sock, buffer, len, flags, reinterpret_cast<SOCKADDR*>(&address), sizeof(address));
        if (ret < 0)
            throw std::system_error(WSAGetLastError(), std::system_category(), "sendto failed");
    }
    void reply(const char* buffer, int len, int flags = 0) {
        if (!this->got_addr) {
            return;
        }
        std::cout << "Sending!" << std::endl;
        return this->SendTo(this->mcp, buffer, len, flags);
    }
    sockaddr_in RecvFrom(char* buffer, int len, int flags = 0)
    {
        sockaddr_in from;
        int size = sizeof(from);
        int ret = recvfrom(sock, buffer, len, flags, reinterpret_cast<SOCKADDR*>(&from), &size);
        if (ret < 0)
            throw std::system_error(WSAGetLastError(), std::system_category(), "recvfrom failed");

        // make the buffer zero terminated
        buffer[ret] = 0;
        this->mcp = from;
        this->mcp.sin_port = htons(3000);
        this->got_addr = 1;
        return from;
    }
    bool PacketReady() {
        unsigned long l;
        ioctlsocket(sock, FIONREAD, &l);
        return l > 0;
    }

    void Bind(unsigned short port)
    {
        sockaddr_in add;
        add.sin_family = AF_INET;
        add.sin_addr.s_addr = htonl(INADDR_ANY);
        add.sin_port = htons(port);

        int ret = bind(sock, reinterpret_cast<SOCKADDR*>(&add), sizeof(add));
        if (ret < 0)
            throw std::system_error(WSAGetLastError(), std::system_category(), "Bind failed");
    }

private:
    SOCKET sock;
};