#include "device.h"
#include "FileQueue.h"
#define directoryPath L"C:\\Users\\Gamer\\OneDrive\\Desktop\\space_apps_2024_seismic_detection\\space_apps_2024_seismic_detection\\data\\lunar\\test\\data\\S16_GradeA"
#define SYMBOLIC_LINK_NAME L"\\DosDevices\\FileQueueDevice"

VOID UnloadDriver(PDRIVER_OBJECT DriverObject) {
    PDEVICE_EXTENSION pDeviceExtension = (PDEVICE_EXTENSION)DriverObject->DeviceObject->DeviceExtension;

    while (!IsListEmpty(&pDeviceExtension->FileQueueHead)) {
        PFILE_QUEUE fileEntry = CONTAINING_RECORD(RemoveHeadList(&pDeviceExtension->FileQueueHead), FILE_QUEUE, ListEntry);
        ExFreePool(fileEntry);
    }

    IoDeleteDevice(DriverObject->DeviceObject);
    UNICODE_STRING symbolicLinkName;
    RtlInitUnicodeString(&symbolicLinkName, SYMBOLIC_LINK_NAME);
    IoDeleteSymbolicLink(&symbolicLinkName);
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject) {
    DbgPrint("Entering DriverEntry");

    NTSTATUS status;
    PDEVICE_OBJECT pDeviceObject = NULL;

    DriverObject->DriverUnload = UnloadDriver;

    status = createFQdevice(DriverObject, &pDeviceObject, directoryPath);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DeviceIOCTLHandler;
    return status; 
}
