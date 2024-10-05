#ifndef FileQueue
#define FileQueue
#include "device.h"
#include <ntddk.h>

typedef struct _File_Queue {
	LIST_ENTRY ListEntry;
	WCHAR FileName[260]; // Max file name length is 260.
} FILE_QUEUE, *PFILE_QUEUE;

NTSTATUS QueueFile(const PWCHAR FileName, PDEVICE_EXTENSION pDeviceExtension);
NTSTATUS QueueFilesFromDirectory(PWCHAR directoryPath, PDEVICE_OBJECT pDeviceObject);

#endif // !FileQueue
