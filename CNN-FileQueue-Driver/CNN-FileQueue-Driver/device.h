#ifndef DEVICE_EXTENSION_H
#define DEVICE_EXTENSION_H

#include <ntddk.h>

typedef struct _DEVICE_EXTENSION {
    LIST_ENTRY FileQueueHead;
    KSPIN_LOCK QueueLock;
    BOOLEAN initialized;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

NTSTATUS createFQdevice(PDRIVER_OBJECT DriverObject, PDEVICE_OBJECT* ppDeviceObject, PWCHAR directoryPath);
NTSTATUS DeviceIOCTLHandler(PDEVICE_OBJECT pDeviceObject, PIRP Irp);

#endif  // DEVICE_EXTENSION_H