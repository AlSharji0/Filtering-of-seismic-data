#include <ntddk.h>
#include "device.h"
#include "FileQueue.h"

#define IOCTL_GET_NEXT_FILE 0x800
#define DEVICE_NAME L"\\Device\\FileQueueDevice"
#define SYMBOLIC_LINK_NAME L"\\DosDevices\\FileQueueDevice"

NTSTATUS createFQdevice(PDRIVER_OBJECT DriverObject, PDEVICE_OBJECT *ppDeviceObject, PWCHAR directoryPath) {
	UNICODE_STRING deviceName, symbolicLinkName;

	RtlInitUnicodeString(&deviceName, DEVICE_NAME);

	NTSTATUS status = IoCreateDevice(DriverObject, sizeof(DEVICE_EXTENSION), &deviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, ppDeviceObject);
	if (!NT_SUCCESS(status)) {
		DbgPrint("Failed to create device: %X\n", status);
		return status;
	}
	DbgPrint("Device created successfully: %wZ\n", &deviceName);

	RtlInitUnicodeString(&symbolicLinkName, SYMBOLIC_LINK_NAME);
	status = IoCreateSymbolicLink(&symbolicLinkName, &deviceName);
	if (!NT_SUCCESS(status)) {
		IoDeleteDevice(*ppDeviceObject);
		DbgPrint("Failed to create symbolic link: %X\n", status);
		return status;
	}
	DbgPrint("Symbolic link created successfully: %wZ\n", &symbolicLinkName);

	status = QueueFilesFromDirectory(directoryPath, *ppDeviceObject);
	return status;
}

NTSTATUS DeviceIOCTLHandler(PDEVICE_OBJECT pDeviceObject, PIRP Irp) {
	PDEVICE_EXTENSION pDeviceExtension = (PDEVICE_EXTENSION)pDeviceObject->DeviceExtension;
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
	ULONG outputBuffSize = stack->Parameters.DeviceIoControl.OutputBufferLength;
	PWCHAR userBuffer = Irp->AssociatedIrp.SystemBuffer;
	NTSTATUS status;

	KIRQL oldIRQL;
	PFILE_QUEUE fileEntry;

	KeAcquireSpinLock(&pDeviceExtension->QueueLock, &oldIRQL);

	switch (stack->Parameters.DeviceIoControl.IoControlCode) {
	case IOCTL_GET_NEXT_FILE:
		if (IsListEmpty(&pDeviceExtension->FileQueueHead)) {
			status = STATUS_NO_MORE_ENTRIES;
			Irp->IoStatus.Information = 0;
		} else {
			fileEntry = CONTAINING_RECORD(pDeviceExtension->FileQueueHead.Blink, FILE_QUEUE, ListEntry);
			size_t fileNameSize = wcslen(fileEntry->FileName) * sizeof(WCHAR);

			RtlCopyMemory(userBuffer, fileEntry->FileName, outputBuffSize);
			RemoveEntryList(pDeviceExtension->FileQueueHead.Blink);
			ExFreePool(fileEntry);
			status = STATUS_SUCCESS;
			Irp->IoStatus.Information = fileNameSize;
		}
		break;

	default:
		status = STATUS_INVALID_DEVICE_REQUEST;
		Irp->IoStatus.Information = 0;
		break;
	}

	KeReleaseSpinLock(&pDeviceExtension->QueueLock, oldIRQL);

	Irp->IoStatus.Status = status;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}
