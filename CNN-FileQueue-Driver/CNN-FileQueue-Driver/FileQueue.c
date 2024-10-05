#include <ntifs.h>
#include "FileQueue.h"
#include "device.h"
#include <ntstrsafe.h>
#define FILE_QUEUE_TAG 'Qeli'

NTSTATUS QueueFile(const PWCHAR FileName, PDEVICE_EXTENSION pDeviceExtension) {
	NTSTATUS status;
	PFILE_QUEUE newFile;

	if (!pDeviceExtension->initialized) {
		InitializeListHead(&pDeviceExtension->FileQueueHead);
		KeInitializeSpinLock(&pDeviceExtension->QueueLock);
		pDeviceExtension->initialized = TRUE;
	}
	
	newFile = (PFILE_QUEUE)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(FILE_QUEUE), FILE_QUEUE_TAG);
	if (!newFile) return STATUS_INSUFFICIENT_RESOURCES;

	if (wcslen(FileName) >= 260) {
		ExFreePool(newFile);
		return STATUS_NAME_TOO_LONG;
	}
	status = wcscpy_s(newFile->FileName, sizeof(newFile->FileName) / sizeof(wchar_t), FileName);
	
	KIRQL oldIRLQ;
	KeAcquireSpinLock(&pDeviceExtension->QueueLock, &oldIRLQ);
	InsertTailList(&(pDeviceExtension->FileQueueHead), &newFile->ListEntry);
	KeReleaseSpinLock(&pDeviceExtension->QueueLock, oldIRLQ);

	return status;
}

NTSTATUS QueueFilesFromDirectory(PWCHAR directoryPath, PDEVICE_OBJECT pDeviceObject) {
	NTSTATUS status;
	HANDLE directoryHandle;
	UNICODE_STRING directoryName;
	OBJECT_ATTRIBUTES objAttr;
	PVOID buffer = NULL;
	ULONG bufferSize = 5120; // 5KB of buffer length.
	IO_STATUS_BLOCK ioStatusBlock;
	FILE_DIRECTORY_INFORMATION* fileInfo;
	PDEVICE_EXTENSION pDeviceExtension = (PDEVICE_EXTENSION)pDeviceObject->DeviceExtension;

	RtlInitUnicodeString(&directoryName, directoryPath);
	InitializeObjectAttributes(&objAttr, &directoryName, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);

	status = ZwCreateFile(&directoryHandle, FILE_LIST_DIRECTORY | SYNCHRONIZE, &objAttr, &ioStatusBlock, NULL, FILE_ATTRIBUTE_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_OPEN, FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
	if (!NT_SUCCESS(status)) return status;

	buffer = ExAllocatePool2(POOL_FLAG_NON_PAGED, bufferSize, FILE_QUEUE_TAG);
	if (!buffer) {
		ZwClose(directoryHandle);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	while (TRUE) {
		status = ZwQueryDirectoryFile(directoryHandle, NULL, NULL, NULL, &ioStatusBlock, buffer, bufferSize, FileDirectoryInformation, TRUE, NULL, FALSE);

		if (!NT_SUCCESS(status)) {
			if (status == STATUS_NO_MORE_FILES) status = STATUS_SUCCESS;
			break;
		}

		fileInfo = (FILE_DIRECTORY_INFORMATION*)buffer;

		while (fileInfo) {
			WCHAR fileNameBuffer[260];
			WCHAR fullPathBuffer[512];
			UNICODE_STRING fullPathUnicodeString;

			RtlCopyMemory(fileNameBuffer, fileInfo->FileName, fileInfo->FileNameLength);
			fileNameBuffer[fileInfo->FileNameLength / sizeof(WCHAR)] = L'\0';

			RtlInitEmptyUnicodeString(&fullPathUnicodeString, fullPathBuffer, sizeof(fullPathBuffer));

			status = RtlUnicodeStringPrintf(&fullPathUnicodeString, L"%ws\\%ws", directoryPath, fileNameBuffer);
			if (!NT_SUCCESS(status)) break;

			QueueFile(fullPathUnicodeString.Buffer, pDeviceExtension);
			
			if (fileInfo->NextEntryOffset == 0) break;
			fileInfo = (FILE_DIRECTORY_INFORMATION*)((PUCHAR)fileInfo + fileInfo->NextEntryOffset);
		}
		
	}

	if (buffer) ExFreePool(buffer);
	ZwClose(directoryHandle);
	return status;
}
