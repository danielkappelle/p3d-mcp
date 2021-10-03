#include <windows.h>
#include <stdio.h>
#include <chrono>
#include <thread>

void pollSerial(char* buf);
void setup();
void loop();
BOOL WINAPI cleanup(DWORD fdwCtrlType);

HANDLE hSerial;

int main() {
	
	setup();
	SetConsoleCtrlHandler(cleanup, true);

	loop();

	//CloseHandle(hSerial);
	cleanup(NULL);
	return 0;
}

void setup() {
	/*
	Setup serial connection
	*/
	
	// FIXME Choose com port on the fly somehow
	hSerial = CreateFileA("COM4", GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (hSerial == INVALID_HANDLE_VALUE) {
		if (GetLastError() == ERROR_FILE_NOT_FOUND) {
			//serial port does not exist. Inform user.
			printf("Error getting handle\n");
		}
		//some other error occurred. Inform user.
	}


	// Serial parameters
	DCB dcbSerialParams = { 0 };
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
	if (!GetCommState(hSerial, &dcbSerialParams)) {
		//error getting state
	}
	dcbSerialParams.BaudRate = CBR_115200;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = NOPARITY;
	if (!SetCommState(hSerial, &dcbSerialParams)) {
		//error setting serial port state
		printf("Error setting serial port state");
	}

	// Timeout parameters
	COMMTIMEOUTS timeouts = { 0 };
	timeouts.ReadIntervalTimeout = 50;
	timeouts.ReadTotalTimeoutConstant = 50;
	timeouts.ReadTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant = 50;
	timeouts.WriteTotalTimeoutMultiplier = 10;
	if (!SetCommTimeouts(hSerial, &timeouts)) {
		//error occureed. Inform user
		printf("Error setting timeouts\n");
	}

	printf("> Successfully opened serial connection\n");
}

BOOL WINAPI cleanup(DWORD fdwCtrlType) {
	// cleanup here (close serial connection)
	CloseHandle(hSerial);
	printf("> Closed serial connection\n> Bye\n");
	return FALSE;
}

void loop() {
	using namespace std::this_thread;
	using namespace std::chrono;
	//sleep_for(seconds(5));
	DWORD dwBytesRead;
	const unsigned int bytes_to_read = 7;
	char buf[bytes_to_read] = { 0 };

	if (!ReadFile(hSerial, buf, bytes_to_read, &dwBytesRead, NULL)) {
		printf("Could not read from device\n");
		return;
	}

	if (dwBytesRead > 0) {
		printf("GOT: %.5s\n", buf);
	}

	sleep_for(milliseconds(10));

	loop();
}

void pollSerial(char* buf) {
	DWORD dwBytesRead;
	do {
		const unsigned int n = 100;
		dwBytesRead = 0;

		if (!ReadFile(hSerial, buf, n, &dwBytesRead, NULL)) {
			printf("Could not read from device\n");
		}
	} while (dwBytesRead == 0);
	printf("Read: %d bytes\n", dwBytesRead);
}