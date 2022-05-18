
#include "network.h"
#include "PMDG_777X_SDK.h"
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>
#include "SimConnect.h"


#include "events.h"
#include <iostream>

extern UDPSocket Socket;

enum DATA_REQUEST_ID {
	DATA_REQUEST,
	CONTROL_REQUEST,
	AIR_PATH_REQUEST
};

struct LastData {
	unsigned short hdg = 0;
	unsigned short alt = 0;
	unsigned short vs = 0;
	float speed = 0;
	bool vs_blank = true;
	bool ias_blank = true;
	
};

class P3dConnect {
public:
	static void connect();
	static void disconnect();
	static void loop();
	static void send_event(enum EVENT_ID event_id, int param);
	static struct LastData last_data;
private:
	static void CALLBACK MyDispatchProc(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext);
	static void Process777XData(PMDG_777X_Data* pS);
	static HRESULT hr;
	static HANDLE  hSimConnect;
	static PMDG_777X_Control Control;
};