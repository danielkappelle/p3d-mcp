#include "Network.h"

int main()
{
    try
    {
        WSASession Session;
        UDPSocket Socket;
        char buffer[100];

        Socket.Bind(3010);
        while (1)
        {
            sockaddr_in add = Socket.RecvFrom(buffer, sizeof(buffer));

            std::string input(buffer);
            std::cout << "Received " << input << std::endl;
            //std::reverse(std::begin(input), std::end(input));
            //Socket.SendTo(add, input.c_str(), input.size());
        }
    }
    catch (std::system_error& e)
    {
        std::cout << e.what();
    }
}