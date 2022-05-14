//------------------------------------------------------------------------------
//
//  PMDG 777X external connection sample 
// 
//------------------------------------------------------------------------------

#include "PMDG_777X_SDK.h"

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>

#include "SimConnect.h"

int     quit = 0;
HANDLE  hSimConnect = NULL;
bool    AircraftRunning = false;
PMDG_777X_Control Control;


enum DATA_REQUEST_ID {
	DATA_REQUEST,
	CONTROL_REQUEST,
	AIR_PATH_REQUEST
};

enum EVENT_ID {
	EVENT_SIM_START,	// used to track the loaded aircraft

	EVENT_LOGO_LIGHT_SWITCH,
	EVENT_FLIGHT_DIRECTOR_SWITCH,

	EVENT_KEYBOARD_A,
	EVENT_KEYBOARD_B,
	EVENT_KEYBOARD_C,
};

enum INPUT_ID {
	INPUT0			// used to handle key presses
};

enum GROUP_ID {
	GROUP_KEYBOARD		// used to handle key presses
};


bool B777X_FuelPumpLAftLight = true;
bool B777X_TaxiLightSwitch = false;
bool B777X_LogoLightSwitch = false;

// This function is called when 777X data changes
void Process777XData(PMDG_777X_Data* pS)
{
	// test the data access:
	// get the state of an annunciator light and display it
	if (pS->FUEL_annunLOWPRESS_Aft[0] != B777X_FuelPumpLAftLight)
	{
		B777X_FuelPumpLAftLight = pS->FUEL_annunLOWPRESS_Aft[0];
		if (B777X_FuelPumpLAftLight)
			printf("LOW PRESS LIGHT: [ON]\n");
		else
			printf("LOW PRESS LIGHT: [OFF]\n");
	}

	// get the state of switches and save it for later use
	if (pS->LTS_Taxi_Sw_ON != B777X_TaxiLightSwitch)
	{
		B777X_TaxiLightSwitch = pS->LTS_Taxi_Sw_ON;
		if (B777X_TaxiLightSwitch)
			printf("TAXI LIGHTS: [ON]\n");
		else
			printf("TAXI LIGHTS: [OFF]\n");
	}

	if (pS->LTS_Logo_Sw_ON != B777X_LogoLightSwitch)
	{
		B777X_LogoLightSwitch = pS->LTS_Logo_Sw_ON;
		if (B777X_LogoLightSwitch)
			printf("LOGO LIGHTS: [ON]\n");
		else
			printf("LOGO LIGHTS: [OFF]\n");
	}
}

void toggleTaxiLightSwitch()
{
	// Test the first control method: use the control data area.
	if (AircraftRunning)
	{
		bool New_TaxiLightSwitch = !B777X_TaxiLightSwitch;

		// Send a command only if there is no active command request and previous command has been processed by the 777X
		if (Control.Event == 0)
		{
			Control.Event = EVT_OH_LIGHTS_TAXI;		// = 69753
			if (New_TaxiLightSwitch)
				Control.Parameter = 1;
			else
				Control.Parameter = 0;
			SimConnect_SetClientData(hSimConnect, PMDG_777X_CONTROL_ID, PMDG_777X_CONTROL_DEFINITION,
				0, 0, sizeof(PMDG_777X_Control), &Control);
		}
	}
}

void toggleLogoLightsSwitch()
{
	// Test the second control method: send an event
	// use direct switch position
	bool New_LogoLightSwitch = !B777X_LogoLightSwitch;

	int parameter = New_LogoLightSwitch ? 1 : 0;
	SimConnect_TransmitClientEvent(hSimConnect, 0, EVENT_LOGO_LIGHT_SWITCH, parameter,
		SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
}

void toggleFlightDirector()
{
	// Test the second control method: send an event
	// use mouse simulation to toggle the switch
	SimConnect_TransmitClientEvent(hSimConnect, 0, EVENT_FLIGHT_DIRECTOR_SWITCH, MOUSE_FLAG_LEFTSINGLE,
		SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
	SimConnect_TransmitClientEvent(hSimConnect, 0, EVENT_FLIGHT_DIRECTOR_SWITCH, MOUSE_FLAG_LEFTRELEASE,
		SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
	printf("Flight Director toggled.\n");
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
		case DATA_REQUEST:
		{
			PMDG_777X_Data* pS = (PMDG_777X_Data*)&pObjData->dwData;
			Process777XData(pS);
			break;
		}
		case CONTROL_REQUEST:
		{
			// keep the present state of Control area to know if the server had received and reset the command
			PMDG_777X_Control* pS = (PMDG_777X_Control*)&pObjData->dwData;
			Control = *pS;
			break;
		}
		}
		break;
	}

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

	default:
		printf("Received:%d\n", pData->dwID);
		break;
	}
}






