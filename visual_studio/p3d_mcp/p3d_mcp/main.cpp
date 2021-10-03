#include <windows.h>
#include <stdio.h>

int main() {
	printf("Hello world\n");
	HANDLE hSerial;
	hSerial = CreateFileA("COM4", GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (hSerial == INVALID_HANDLE_VALUE) {
		if (GetLastError() == ERROR_FILE_NOT_FOUND) {
			//serial port does not exist. Inform user.
			printf("Error getting handle\n");
		}
		//some other error occurred. Inform user.
	}

	DCB dcbSerialParams = { 0 };
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
	if (!GetCommState(hSerial, &dcbSerialParams)) {
		//error getting state
	}
	dcbSerialParams.BaudRate = CBR_9600;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = NOPARITY;
	if (!SetCommState(hSerial, &dcbSerialParams)) {
		//error setting serial port state
		printf("Error setting serial port state");
	}


	COMMTIMEOUTS timeouts = { 0 };
	timeouts.ReadIntervalTimeout = 50;
	timeouts.ReadTotalTimeoutConstant = 2000;
	timeouts.ReadTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant = 50;
	timeouts.WriteTotalTimeoutMultiplier = 10;
	if (!SetCommTimeouts(hSerial, &timeouts)) {
		//error occureed. Inform user
		printf("Error setting timeouts\n");
	}

	const unsigned int n = 4;

	char szBuff[n + 1] = { 0 };
	DWORD dwBytesRead = 0;

	if (!ReadFile(hSerial, szBuff, n, &dwBytesRead, NULL)) {
		printf("Could not read from device\n");
	}

	printf("Read %d bytes\n", dwBytesRead);
	if (dwBytesRead > 0) {
		printf("Content: %.4s", szBuff);
	}

	printf("All done");

	CloseHandle(hSerial);
}