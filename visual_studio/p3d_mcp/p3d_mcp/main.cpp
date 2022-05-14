#include "Network.h"
#include "PMDG_777X_SDK.h"

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>

#include "SimConnect.h"

HRESULT hr;
HANDLE  hSimConnect = NULL;
PMDG_777X_Control Control;

bool taxiswitch = 1;

void connect();
void change_speed(int dir);
void CALLBACK MyDispatchProc(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext);

enum DATA_REQUEST_ID {
	DATA_REQUEST,
	CONTROL_REQUEST,
	AIR_PATH_REQUEST
};

int main()
{
    try
    {
		/* Set up UDP shizzle*/
        WSASession Session;
        UDPSocket Socket;
        char buffer[100];

        Socket.Bind(3010);

		/* Setup Sim connection */
		connect();

        while (1)
        {
			if (Socket.PacketReady()) {
				sockaddr_in add = Socket.RecvFrom(buffer, sizeof(buffer));
				std::string input(buffer);
				std::cout << "Received " << input << std::endl;
				
				if (input.compare("ROT:0:UP") == 0) {
					change_speed(1);
				}
				else if (input.compare("ROT:0:DN") == 0) {
					change_speed(-1);
				}
			}

			SimConnect_CallDispatch(hSimConnect, MyDispatchProc, NULL);
			Sleep(1);
        }
    }
    catch (std::system_error& e)
    {
        std::cout << e.what();
    }
}

void CALLBACK MyDispatchProc(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext)
{
	switch (pData->dwID)
	{
	case SIMCONNECT_RECV_ID_CLIENT_DATA: // Receive and process the 777X data block
	{
		SIMCONNECT_RECV_CLIENT_DATA* pObjData = (SIMCONNECT_RECV_CLIENT_DATA*)pData;

		switch (pObjData->dwRequestID)
		{
			/*
		case DATA_REQUEST:
		{
			PMDG_777X_Data* pS = (PMDG_777X_Data*)&pObjData->dwData;
			Process777XData(pS);
			break;
		}
		*/
		case CONTROL_REQUEST:
		{
			// keep the present state of Control area to know if the server had received and reset the command
			PMDG_777X_Control* pS = (PMDG_777X_Control*)&pObjData->dwData;
			std::cout << "Updated control" << std::endl;
			Control = *pS;
			break;
		}
		}
		break;
	}
	/*
	case SIMCONNECT_RECV_ID_EVENT:
	{
		SIMCONNECT_RECV_EVENT* evt = (SIMCONNECT_RECV_EVENT*)pData;
		switch (evt->uEventID)
		{
		case EVENT_SIM_START:	// Track aircraft changes
		{
			HRESULT hr = SimConnect_RequestSystemState(hSimConnect, AIR_PATH_REQUEST, "AircraftLoaded");
			break;
		}
		case EVENT_KEYBOARD_A:
		{
			toggleTaxiLightSwitch();
			break;
		}
		case EVENT_KEYBOARD_B:
		{
			toggleLogoLightsSwitch();
			break;
		}
		case EVENT_KEYBOARD_C:
		{
			toggleFlightDirector();
			break;
		}
		}
		break;
	}
	*/

	/*
	case SIMCONNECT_RECV_ID_SYSTEM_STATE: // Track aircraft changes
	{
		SIMCONNECT_RECV_SYSTEM_STATE* evt = (SIMCONNECT_RECV_SYSTEM_STATE*)pData;
		if (evt->dwRequestID == AIR_PATH_REQUEST)
		{
			if (strstr(evt->szString, "PMDG 777") != NULL)
				AircraftRunning = true;
			else
				AircraftRunning = false;
		}
		break;
	}

	case SIMCONNECT_RECV_ID_QUIT:
	{
		quit = 1;
		break;
	}
	*/
	default:
		printf("Received:%d\n", pData->dwID);
		break;
	}
}

void change_speed(int dir) {
	// Send a command only if there is no active command request and previous command has been processed by the 777X
	if (Control.Event == 0)
	{
		Control.Event = EVT_MCP_SPEED_SELECTOR;		// = 69753
		if (dir > 0) {
			Control.Parameter = MOUSE_FLAG_WHEEL_UP;
		}
		else {
			Control.Parameter = MOUSE_FLAG_WHEEL_DOWN;
		}
		SimConnect_SetClientData(hSimConnect, PMDG_777X_CONTROL_ID, PMDG_777X_CONTROL_DEFINITION,
			0, 0, sizeof(PMDG_777X_Control), &Control);
	}
	else {
		std::cout << "Event non-zero" << std::endl;
	}
}

void connect()
{

	if (SUCCEEDED(SimConnect_Open(&hSimConnect, "PMDG 777X Test", NULL, 0, 0, 0)))
	{
		printf("Connected to Flight Simulator!\n");

		// 1) Set up data connection

		// Associate an ID with the PMDG data area name
		hr = SimConnect_MapClientDataNameToID(hSimConnect, PMDG_777X_DATA_NAME, PMDG_777X_DATA_ID);

		// Define the data area structure - this is a required step
		hr = SimConnect_AddToClientDataDefinition(hSimConnect, PMDG_777X_DATA_DEFINITION, 0, sizeof(PMDG_777X_Data), 0, 0);

		
		// 2) Set up control connection

		// First method: control data area
		Control.Event = 0;
		Control.Parameter = 0;

		// Associate an ID with the PMDG control area name
		hr = SimConnect_MapClientDataNameToID(hSimConnect, PMDG_777X_CONTROL_NAME, PMDG_777X_CONTROL_ID);

		// Define the control area structure - this is a required step
		hr = SimConnect_AddToClientDataDefinition(hSimConnect, PMDG_777X_CONTROL_DEFINITION, 0, sizeof(PMDG_777X_Control), 0, 0);
		
		// Sign up for notification of control change.  
		hr = SimConnect_RequestClientData(hSimConnect, PMDG_777X_CONTROL_ID, CONTROL_REQUEST, PMDG_777X_CONTROL_DEFINITION,
			SIMCONNECT_CLIENT_DATA_PERIOD_VISUAL_FRAME, SIMCONNECT_CLIENT_DATA_REQUEST_FLAG_CHANGED, 0, 0, 0);
		/*
		// Second method: Create event IDs for controls that we are going to operate
		hr = SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_LOGO_LIGHT_SWITCH, "#69748");		//EVT_OH_LIGHTS_LOGO
		hr = SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_FLIGHT_DIRECTOR_SWITCH, "#69834");	//EVT_MCP_FD_SWITCH_L


		// 3) Request current aircraft .air file path
		hr = SimConnect_RequestSystemState(hSimConnect, AIR_PATH_REQUEST, "AircraftLoaded");
		// also request notifications on sim start and aircraft change
		hr = SimConnect_SubscribeToSystemEvent(hSimConnect, EVENT_SIM_START, "SimStart");


		// 5) Main loop
		while (quit == 0)
		{
			// receive and process the 777X data
			SimConnect_CallDispatch(hSimConnect, MyDispatchProc, NULL);

			Sleep(1);
		}

		hr = SimConnect_Close(hSimConnect);
		*/
	}
	else {
		printf("Unable to connect!\n\n");
	}
}

void disconnect() {
	hr = SimConnect_Close(hSimConnect);
}