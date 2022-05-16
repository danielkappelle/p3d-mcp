#pragma once
#include "p3d_connect.h"

HRESULT P3dConnect::hr;
HANDLE  P3dConnect::hSimConnect = NULL;
PMDG_777X_Control P3dConnect::Control;

void P3dConnect::loop() {
	SimConnect_CallDispatch(P3dConnect::hSimConnect, P3dConnect::MyDispatchProc, NULL);
}

// This function is called when 777X data changes
void P3dConnect::Process777XData(PMDG_777X_Data* pS)
{/*
	unsigned short hdg = pS->MCP_Heading;
	if (last_data.hdg != hdg) {
		std::cout << "hdg changed" << std::endl;
		last_data.hdg = hdg;
		char cmd[20];
		sprintf_s(cmd, "DISP:2:%d", hdg);
		Socket.reply(cmd, sizeof(cmd));
	}*/
}

void P3dConnect::send_event(enum EVENT_ID event_id, int param) {
	/*int parameter;
	if (dir > 0) {
		parameter = MOUSE_FLAG_WHEEL_UP;
	}
	else {
		parameter = MOUSE_FLAG_WHEEL_DOWN;
	}*/

	SimConnect_TransmitClientEvent(P3dConnect::hSimConnect, 0, event_id, param,
		SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
}


void CALLBACK P3dConnect::MyDispatchProc(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext)
{
	switch (pData->dwID)
	{
	case SIMCONNECT_RECV_ID_CLIENT_DATA: // Receive and process the 777X data block
	{
		SIMCONNECT_RECV_CLIENT_DATA* pObjData = (SIMCONNECT_RECV_CLIENT_DATA*)pData;

		switch (pObjData->dwRequestID)
		{
		case DATA_REQUEST:
		{
			PMDG_777X_Data* pS = (PMDG_777X_Data*)&pObjData->dwData;
			P3dConnect::Process777XData(pS);
			break;
		}
		case CONTROL_REQUEST:
		{
			// keep the present state of Control area to know if the server had received and reset the command
			PMDG_777X_Control* pS = (PMDG_777X_Control*)&pObjData->dwData;
			P3dConnect::Control = *pS;
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


void P3dConnect::connect()
{

	if (SUCCEEDED(SimConnect_Open(&hSimConnect, "PMDG 777X Test", NULL, 0, 0, 0)))
	{
		std::cout << "Connected to Flight Simulator!" << std::endl;

		// 1) Set up data connection

		// Associate an ID with the PMDG data area name
		P3dConnect::hr = SimConnect_MapClientDataNameToID(hSimConnect, PMDG_777X_DATA_NAME, PMDG_777X_DATA_ID);

		// Define the data area structure - this is a required step
		P3dConnect::hr = SimConnect_AddToClientDataDefinition(hSimConnect, PMDG_777X_DATA_DEFINITION, 0, sizeof(PMDG_777X_Data), 0, 0);


		// 2) Set up control connection

		// First method: control data area
		P3dConnect::Control.Event = 0;
		P3dConnect::Control.Parameter = 0;

		// Associate an ID with the PMDG control area name
		P3dConnect::hr = SimConnect_MapClientDataNameToID(hSimConnect, PMDG_777X_CONTROL_NAME, PMDG_777X_CONTROL_ID);

		// Define the control area structure - this is a required step
		P3dConnect::hr = SimConnect_AddToClientDataDefinition(hSimConnect, PMDG_777X_CONTROL_DEFINITION, 0, sizeof(PMDG_777X_Control), 0, 0);

		// Sign up for notification of control change.  
		P3dConnect::hr = SimConnect_RequestClientData(hSimConnect, PMDG_777X_CONTROL_ID, CONTROL_REQUEST, PMDG_777X_CONTROL_DEFINITION,
			SIMCONNECT_CLIENT_DATA_PERIOD_VISUAL_FRAME, SIMCONNECT_CLIENT_DATA_REQUEST_FLAG_CHANGED, 0, 0, 0);

		// Second method: Create event IDs for controls that we are going to operate
		// base val is 69632
		P3dConnect::hr = SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_HDG, "#71812");
		P3dConnect::hr = SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_SPEED, "#69842");
		P3dConnect::hr = SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_ALT, "#71882");
		P3dConnect::hr = SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_VS, "#69854");
		P3dConnect::hr = SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_CPT_MINIMUMS, "#69814");
		P3dConnect::hr = SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_FO_MINIMUMS, "#69881");

		P3dConnect::hr = SimConnect_RequestClientData(hSimConnect, PMDG_777X_DATA_ID,
			DATA_REQUEST, PMDG_777X_DATA_DEFINITION,
			SIMCONNECT_CLIENT_DATA_PERIOD_ON_SET,
			SIMCONNECT_CLIENT_DATA_REQUEST_FLAG_CHANGED, 0, 0, 0);
		/*
		hr = SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_FLIGHT_DIRECTOR_SWITCH, "#69834");	//EVT_MCP_FD_SWITCH_L



		// 3) Request current aircraft .air file path
		hr = SimConnect_RequestSystemState(hSimConnect, AIR_PATH_REQUEST, "AircraftLoaded");
		// also request notifications on sim start and aircraft change
		hr = SimConnect_SubscribeToSystemEvent(hSimConnect, EVENT_SIM_START, "SimStart");

		hr = SimConnect_Close(hSimConnect);
		*/
	}
	else {
		printf("Unable to connect!\n\n");
	}
}

void P3dConnect::disconnect() {
	P3dConnect::hr = SimConnect_Close(P3dConnect::hSimConnect);
}