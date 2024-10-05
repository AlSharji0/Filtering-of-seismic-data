#include <Windows.h>
#include <iostream>
#include <string>

#define IOCTL_GET_NEXT_FILE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define DEVICE_PATH L""
#define CNN_PATH ""

std::string SendIOCTLtoDevice() {
	HANDLE hDevice = CreateFileW(DEVICE_PATH, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hDevice == INVALID_HANDLE_VALUE) {
		std::cerr << "Failed to open device." << GetLastError() << std::endl;
		exit(-1);
	}

	char outputBuffer[1024];
	DWORD bytesReturned = 0;

	BOOL status = DeviceIoControl(hDevice, IOCTL_GET_NEXT_FILE, NULL, 0, outputBuffer, sizeof(outputBuffer), &bytesReturned, NULL);
	if (!status || bytesReturned == 0) {  // Check if the IOCTL failed or no data was returned
		std::cerr << "No file path returned by IOCTL or IOCTL failed." << std::endl;
		CloseHandle(hDevice);
		return "";  // Return an empty string to signal that no file path was returned
	}

	CloseHandle(hDevice);
	return std::string(outputBuffer, bytesReturned);
}

void callCNNwithData(const std::string& data) {
	std::string command = "python" + std::string(CNN_PATH) + "\"" + data + "\"";

	int status = system(command.c_str());
	if (status != 0) {
		std::cerr << "Python script execution failed." << std::endl;
		exit(-1);
	}
}

int main() {
	while (true) {
		std::string receivedPath = SendIOCTLtoDevice();
		if (receivedPath.empty()) {
			std::cout << "No more files to process." << std::endl;
			break;
		}

		callCNNwithData(receivedPath);
	}
	return 0;
}
