#include "Network.h"

#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>

#include "events.h"
#include "rotary.h"
#include "p3d_connect.h"

struct LastData {
	unsigned short hdg = 0;
};

struct LastData last_data;


WSASession Session;
UDPSocket Socket;

Rotary rot0(0, EVENT_CPT_MINIMUMS);
Rotary rot1(1, EVENT_SPEED);
Rotary rot2(2, EVENT_HDG);
Rotary rot3(3, EVENT_VS);
Rotary rot4(4, EVENT_ALT);
Rotary rot5(6, EVENT_FO_MINIMUMS);
//P3dConnect sim;

int main()
{
    try
    {
		/* Set up UDP shizzle*/
        char buffer[100];

        Socket.Bind(3010);

		/* Setup Sim connection */
		P3dConnect::connect();

        while (1)
        {
			if (Socket.PacketReady()) {
				sockaddr_in add = Socket.RecvFrom(buffer, sizeof(buffer));
				std::string input(buffer);
				std::cout << "Received " << input << std::endl;
				
				Rotary::update_all(input);
			}

			P3dConnect::loop();
			Sleep(1);
        }
    }
    catch (std::system_error& e)
    {
        std::cout << e.what();
    }
}

/*
void change_hdg(int dir) {
	int parameter;
	if (dir > 0) {
		parameter = MOUSE_FLAG_WHEEL_UP;
	}
	else {
		parameter = MOUSE_FLAG_WHEEL_DOWN;
	}

	SimConnect_TransmitClientEvent(hSimConnect, 0, EVENT_HDG, parameter,
		SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
}
*/
