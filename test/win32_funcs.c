
#include "xplist.h"

#ifdef __MINGW32__
#define WIN32_LEAN_AND_MEAN 1
#include <stddef.h> // For offsetof
#include <devguid.h>    // Device guids
#include <setupapi.h>   // for SetupDiXxx functions.
#include <initguid.h>
#include <ddk/scsi.h>
#include <ddk/ntdddisk.h>
#include <ddk/ntddscsi.h>
#include <ddk/ntddstor.h>

extern list win32_getdisks(void);

list get_devs(void) {
	list lp;

#if 1
	lp = win32_getdisks();
#else
	char devs[8192], *p;
	char drive[64];
	int sz, err;
	int i;
	HANDLE hDevice;

	lp = list_create();

	sz = QueryDosDevice(0, devs, sizeof(devs));
	printf("sz: %d\n", sz);
	err = GetLastError();
	if (!sz) {
		printf("errno: %d(%x)\n", err, err);
		return lp;
	}
	p = devs;
	while(1) {
#define PD "PhysicalDrive"
//		dprintf("p: %s\n", p);
		if (strncmp(p,PD,strlen(PD)) == 0) {
			sprintf(drive,"\\\\.\\%s", p);
			printf("drive: %s\n", drive);
			list_add(lp,drive,strlen(drive)+1);
		}
		while(*p) p++;
		if (!*p) p++;
		if (!*p) break;
	}
#endif

	return lp;
}

#include <stddef.h> // For offsetof
#include <ddk/ntdddisk.h>
#include <ddk/ntddscsi.h>

#define SENSELEN 24
int do_ioctl(filehandle_t fd, unsigned char *cdb, int cdb_len, unsigned char *buffer, int buflen) {
        struct _req {
                SCSI_PASS_THROUGH_DIRECT Sptd;
                unsigned char SenseBuf[SENSELEN];
        } req;
        int status,err;
        DWORD returnedLen;

        memset(&req,0,sizeof(req));
        req.Sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
        req.Sptd.CdbLength = cdb_len;
        req.Sptd.SenseInfoLength = SENSELEN;
        req.Sptd.DataIn = SCSI_IOCTL_DATA_IN;
        req.Sptd.DataTransferLength = buflen;
        req.Sptd.TimeOutValue = 2;
        req.Sptd.DataBuffer = buffer;
        req.Sptd.SenseInfoOffset = offsetof(struct _req, SenseBuf);
        memcpy(&req.Sptd.Cdb,cdb,cdb_len);

        status = DeviceIoControl(
                fd,
                IOCTL_SCSI_PASS_THROUGH_DIRECT,
                &req,
                sizeof(req),
                &req,
                sizeof(req),
                &returnedLen,
                0
        );
        dprintf("status: %d\n", status);
        if (!status) {
                err = 1;
        } else {
                err = (req.Sptd.ScsiStatus != 0);
        }
        return err;
}

#if 0
#define SENSELEN 24
//int inquiry(HANDLE hDevice, unsigned char *data, int buflen) {
int do_ioctl(filehandle_t fd, unsigned char *cdb, int cdb_len, unsigned char *buffer, int buflen) {
	struct _req {
		SCSI_PASS_THROUGH_DIRECT Sptd;
		unsigned char SenseBuf[SENSELEN];
	} req;
	int status;
	DWORD returnedLen;

//	hDevice = (HANDLE) _get_osfhandle(fd);

	memset(&req,0,sizeof(req));
	req.Sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
	req.Sptd.CdbLength = 6;
	req.Sptd.SenseInfoLength = SENSELEN;
	req.Sptd.DataIn = SCSI_IOCTL_DATA_IN;
	req.Sptd.DataTransferLength = buflen;
	req.Sptd.TimeOutValue = 2;
	req.Sptd.DataBuffer = buffer;
	req.Sptd.SenseInfoOffset = offsetof(struct _req, SenseBuf);
	req.Sptd.Cdb[0] = 0x12;
        req.Sptd.Cdb[3] = buflen / 256;
        req.Sptd.Cdb[4] = buflen / 256;


	status = DeviceIoControl(
		fd,
		IOCTL_SCSI_PASS_THROUGH_DIRECT,
		&req,
		sizeof(req),
		&req,
		sizeof(req),
		&returnedLen,
		0
	);
	dprintf("status: %d\n", status);
	if (!status) {
		dprintf("errno: %d\n", (int)GetLastError());
		return 1;
	}
	dprintf("returnedLen: %d\n", (int) returnedLen);

	return 0;
}
#endif

#if 0
int capacity(HANDLE hDevice, int *size) {
	SCSI_PASS_THROUGH_DIRECT Sptd;
	int status;
	DWORD rlen;
        unsigned char data[32], *p;
        unsigned long long bytes;
        unsigned long lba,len;

	memset(&Sptd,0,sizeof(Sptd));
	Sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
	Sptd.CdbLength = 10;
	Sptd.DataIn = SCSI_IOCTL_DATA_IN;
	Sptd.DataTransferLength = sizeof(data);
	Sptd.TimeOutValue = 2;
	Sptd.DataBuffer = data;
	Sptd.Cdb[0] = SCSIOP_READ_CAPACITY;

	dprintf("calling DeviceIoControl...\n");
	status = DeviceIoControl(
		hDevice,
		IOCTL_SCSI_PASS_THROUGH_DIRECT,
		&Sptd,
		sizeof(Sptd),
		&Sptd,
		sizeof(Sptd),
		&rlen,
		0
	);
	dprintf("status: %d\n", status);
	if (!status) {
		dprintf("errno: %d\n", (int)GetLastError());
		return 1;
	}
	dprintf("rlen: %d\n", (int) rlen);

        p = data;
        lba = (*(p+0) << 24) | (*(p+1) << 16) | (*(p+2) << 8) | *(p+3);
        len = (*(p+4) << 24) | (*(p+5) << 16) | (*(p+6) << 8) | *(p+7);
        dprintf("lba: %ld, len: %ld\n", lba, len);
        bytes = lba;
        bytes *= len;
        dprintf("bytes: %lld\n", bytes);
        *size = bytes / 1048576;
        dprintf("size: %dMB\n", *size);
        return 0;
}
#endif

#if 0
int try(list l) {
    HDEVINFO hDevInfo;
       SP_DEVINFO_DATA DeviceInfoData;
       DWORD i;

       // Create a HDEVINFO with all present devices.
	hDevInfo = SetupDiGetClassDevs(&GUID_DEVCLASS_DISKDRIVE, NULL, NULL, DIGCF_PRESENT);
//	hDevInfo = SetupDiGetClassDevs(&GUID_DEVCLASS_DISKDRIVE, NULL, NULL, DIGCF_DEVICEINTERFACE);
//	hDevInfo = SetupDiGetClassDevs(&GUID_DEVCLASS_DISKDRIVE, NULL, NULL, DIGCF_ALLCLASSES);
	printf("hDevInfo: %p\n", hDevInfo);
	if (hDevInfo == INVALID_HANDLE_VALUE) {
		// Insert error handling here.
		return 1;
	}

	// Enumerate through all devices in Set.
	DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
       for (i=0;SetupDiEnumDeviceInfo(hDevInfo,i,&DeviceInfoData);i++) {
           DWORD DataT;
           LPTSTR buffer = NULL;
           DWORD buffersize = 0;
 
	printf("i: %d\n", (int)i);
           //
           // Call function with null to begin with, 
           // then use the returned buffer size (doubled)
           // to Alloc the buffer. Keep calling until
           // success or an unknown failure.
           //
           //  Double the returned buffersize to correct
           //  for underlying legacy CM functions that 
           //  return an incorrect buffersize value on 
           //  DBCS/MBCS systems.
           // 
//               SPDRP_DEVICEDESC,
           while (!SetupDiGetDeviceRegistryProperty(
               hDevInfo,
               &DeviceInfoData,
		SPDRP_PHYSICAL_DEVICE_OBJECT_NAME,
               &DataT,
               (PBYTE)buffer,
               buffersize,
               &buffersize))
           {
               if (GetLastError() == 
                   ERROR_INSUFFICIENT_BUFFER)
               {
			printf("changing buffer...\n");
                   // Change the buffer size.
                   if (buffer) LocalFree(buffer);
                   // Double the size to avoid problems on 
                   // W2k MBCS systems per KB 888609. 
                   buffer = LocalAlloc(LPTR,buffersize * 2);
               }
               else
               {
                   // Insert error handling here.
		printf("error!\n");
                   break;
               }
           }
           
           printf("Result:[%s]\n",buffer);
		list_add(l,buffer,strlen(buffer)+1);
           
           if (buffer) LocalFree(buffer);
       }

	return 0;
}
#endif

#if 1
list win32_getdisks(void) {
	list l;
	int i;
	HDEVINFO hIntDevInfo;
	SP_INTERFACE_DEVICE_DATA interfaceData;
	PSP_DEVICE_INTERFACE_DETAIL_DATA interfaceDetailData;
	DWORD reqSize, interfaceDetailDataSize;
//	SCSI_PASS_THROUGH_DIRECT Sptd;
//	DWORD status;

	l = list_create();

	// Only Devices present & Interface class
	hIntDevInfo = SetupDiGetClassDevs (&GUID_DEVINTERFACE_DISK,0,0,(DIGCF_PRESENT | DIGCF_INTERFACEDEVICE));
	printf("hDevInfo: %p\n", hIntDevInfo);

#if 0
	Sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
	Sptd.ScsiStatus = 0;
	Sptd.PathId = 0;
	Sptd.TargetId = 0;
	Sptd.Lun = 0;
	Sptd.CdbLength = pass->cdb_len;
	Sptd.SenseInfoLength = SENSELEN;
	if (pass->buflen)
		req.Spt.DataIn = (pass->write ? SCSI_IOCTL_DATA_OUT : SCSI_IOCTL_DATA_IN);
	else
		req.Spt.DataIn = SCSI_IOCTL_DATA_UNSPECIFIED;
	Sptd.DataTransferLength = pass->buflen;
	Sptd.TimeOutValue = 2;
	Sptd.DataBuffer = buffer;
	Sptd.SenseInfoOffset = sizeof(SCSI_PASS_THROUGH_DIRECT);
#endif

	interfaceData.cbSize = sizeof(SP_INTERFACE_DEVICE_DATA);
	for (i=0; SetupDiEnumDeviceInterfaces(hIntDevInfo,0,&GUID_DEVINTERFACE_DISK,i,&interfaceData); i++) {
		printf("i: %d\n", i);

		interfaceDetailData = 0;
		interfaceDetailDataSize = 0;
		while(!SetupDiGetDeviceInterfaceDetail(hIntDevInfo,&interfaceData,interfaceDetailData,interfaceDetailDataSize,&reqSize,0)) {
			printf("reqSize: %d\n", (int) reqSize);
			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
				if (interfaceDetailData) LocalFree(interfaceDetailData);
				interfaceDetailDataSize = reqSize * 2;
				interfaceDetailData = malloc (interfaceDetailDataSize);
    				interfaceDetailData->cbSize = sizeof (SP_INTERFACE_DEVICE_DETAIL_DATA);
			} else {
				printf("SetupDiGetDeviceInterfaceDetail failed: %d\n", (int)GetLastError());
				return 0;
			}
		}
		printf("path: %s\n", interfaceDetailData->DevicePath);
		list_add(l, interfaceDetailData->DevicePath, strlen(interfaceDetailData->DevicePath)+1);
	}

//	try(l);
//	try2(l);
	return l;
}
#endif

#if 0
BOOL WINAPI GetVolumeInformation(
  __in_opt   LPCTSTR lpRootPathName,
  __out      LPTSTR lpVolumeNameBuffer,
  __in       DWORD nVolumeNameSize,
  __out_opt  LPDWORD lpVolumeSerialNumber,
  __out_opt  LPDWORD lpMaximumComponentLength,
  __out_opt  LPDWORD lpFileSystemFlags,
  __out      LPTSTR lpFileSystemNameBuffer,
  __in       DWORD nFileSystemNameSize
);

/*++

Copyright (c) 1990-2000 Microsoft Corporation, All Rights Reserved

Module Name:

    enumdisk.h

Abstract:

    This file includes data declarations for the Enumdisk

Author:

    Raju Ramanathan     05/15/2000

Notes:

Revision History:


--*/

#ifndef _ENUMDISK_H_
#define _ENUMDISK_H_


//
// Command Descriptor Block constants.
//

#define CDB6GENERIC_LENGTH         6
#define CDB10GENERIC_LENGTH        10


//
// SCSI CDB operation codes
//

#define SCSIOP_TEST_UNIT_READY     0x00
#define SCSIOP_REZERO_UNIT         0x01
#define SCSIOP_REWIND              0x01
#define SCSIOP_REQUEST_BLOCK_ADDR  0x02
#define SCSIOP_REQUEST_SENSE       0x03
#define SCSIOP_FORMAT_UNIT         0x04
#define SCSIOP_READ_BLOCK_LIMITS   0x05
#define SCSIOP_REASSIGN_BLOCKS     0x07
#define SCSIOP_READ6               0x08
#define SCSIOP_RECEIVE             0x08
#define SCSIOP_WRITE6              0x0A
#define SCSIOP_PRINT               0x0A
#define SCSIOP_SEND                0x0A
#define SCSIOP_SEEK6               0x0B
#define SCSIOP_TRACK_SELECT        0x0B
#define SCSIOP_SLEW_PRINT          0x0B
#define SCSIOP_SEEK_BLOCK          0x0C
#define SCSIOP_PARTITION           0x0D
#define SCSIOP_READ_REVERSE        0x0F
#define SCSIOP_WRITE_FILEMARKS     0x10
#define SCSIOP_FLUSH_BUFFER        0x10
#define SCSIOP_SPACE               0x11
#define SCSIOP_INQUIRY             0x12
#define SCSIOP_VERIFY6             0x13
#define SCSIOP_RECOVER_BUF_DATA    0x14
#define SCSIOP_MODE_SELECT         0x15
#define SCSIOP_RESERVE_UNIT        0x16
#define SCSIOP_RELEASE_UNIT        0x17
#define SCSIOP_COPY                0x18
#define SCSIOP_ERASE               0x19
#define SCSIOP_MODE_SENSE          0x1A
#define SCSIOP_START_STOP_UNIT     0x1B
#define SCSIOP_STOP_PRINT          0x1B
#define SCSIOP_LOAD_UNLOAD         0x1B
#define SCSIOP_RECEIVE_DIAGNOSTIC  0x1C
#define SCSIOP_SEND_DIAGNOSTIC     0x1D
#define SCSIOP_MEDIUM_REMOVAL      0x1E
#define SCSIOP_READ_CAPACITY       0x25
#define SCSIOP_READ                0x28
#define SCSIOP_WRITE               0x2A
#define SCSIOP_SEEK                0x2B
#define SCSIOP_LOCATE              0x2B
#define SCSIOP_WRITE_VERIFY        0x2E
#define SCSIOP_VERIFY              0x2F
#define SCSIOP_SEARCH_DATA_HIGH    0x30
#define SCSIOP_SEARCH_DATA_EQUAL   0x31
#define SCSIOP_SEARCH_DATA_LOW     0x32
#define SCSIOP_SET_LIMITS          0x33
#define SCSIOP_READ_POSITION       0x34
#define SCSIOP_SYNCHRONIZE_CACHE   0x35
#define SCSIOP_COMPARE             0x39
#define SCSIOP_COPY_COMPARE        0x3A
#define SCSIOP_WRITE_DATA_BUFF     0x3B
#define SCSIOP_READ_DATA_BUFF      0x3C
#define SCSIOP_CHANGE_DEFINITION   0x40
#define SCSIOP_READ_SUB_CHANNEL    0x42
#define SCSIOP_READ_TOC            0x43
#define SCSIOP_READ_HEADER         0x44
#define SCSIOP_PLAY_AUDIO          0x45
#define SCSIOP_PLAY_AUDIO_MSF      0x47
#define SCSIOP_PLAY_TRACK_INDEX    0x48
#define SCSIOP_PLAY_TRACK_RELATIVE 0x49
#define SCSIOP_PAUSE_RESUME        0x4B
#define SCSIOP_LOG_SELECT          0x4C
#define SCSIOP_LOG_SENSE           0x4D


ULONG   DebugLevel = 1;
                            // 0 = Suppress All Messages
                            // 1 = Display & Fatal Error Message
                            // 2 = Warning & Debug Messages
                            // 3 = Informational Messages

//
// Bus Type
//

static char* BusType[] = {
    "UNKNOWN",  // 0x00
    "SCSI",
    "ATAPI",
    "ATA",
    "IEEE 1394",
    "SSA",
    "FIBRE",
    "USB",
    "RAID"
};

//
// SCSI Device Type
//

static char* DeviceType[] = {
    "Direct Access Device", // 0x00
    "Tape Device",          // 0x01
    "Printer Device",       // 0x02
    "Processor Device",     // 0x03
    "WORM Device",          // 0x04
    "CDROM Device",         // 0x05
    "Scanner Device",       // 0x06
    "Optical Disk",         // 0x07
    "Media Changer",        // 0x08
    "Comm. Device",         // 0x09
    "ASCIT8",               // 0x0A
    "ASCIT8",               // 0x0B
    "Array Device",         // 0x0C
    "Enclosure Device",     // 0x0D
    "RBC Device",           // 0x0E
    "Unknown Device"        // 0x0F
};


typedef struct _SCSI_PASS_THROUGH_WITH_BUFFERS {
    SCSI_PASS_THROUGH Spt;
    ULONG             Filler;      // realign buffers to double word boundary
    UCHAR             SenseBuf[32];
    UCHAR             DataBuf[512];
} SCSI_PASS_THROUGH_WITH_BUFFERS, *PSCSI_PASS_THROUGH_WITH_BUFFERS;


VOID PrintError( ULONG );
VOID PrintDataBuffer( PUCHAR, ULONG );
VOID PrintStatusResults( BOOL, DWORD, PSCSI_PASS_THROUGH_WITH_BUFFERS );
VOID PrintSenseInfo( PSCSI_PASS_THROUGH_WITH_BUFFERS );
BOOL GetRegistryProperty( HDEVINFO, DWORD );
BOOL GetDeviceProperty( HDEVINFO, DWORD );
VOID DebugPrint( USHORT, PCHAR, ... );

#endif    // _ENUMDISK_H_

/*++

Copyright (c) 1990-2000 Microsoft Corporation, All Rights Reserved

Module Name:

    Enumdisk.c

Abstract:

Please note that the objective of this sample is to demonstrate the 
techniques used. This is not a complete program to be used in 
commercial product.
 
The purpose of the sample program is to enumerates all available disk
devices and get the device property. It uses IOCTL_STORAGE_QUERY_PROPERTY 
to get the Bus and Device properties. It also opens the handle to the device 
and sends a SCSI Pass Through command.

See SDK & DDK Documentation for more information about the APIs, 
IOCTLs and data structures used.
    
Author:

    Raju Ramanathan     05/15/2000

Notes:


Revision History:

  Raju Ramanathan       05/29/2000    Completed the sample

--*/

#include <stdio.h> 
#include <stdlib.h> 
#include <stddef.h>
#include <windows.h>  
#include <devguid.h>    // Device guids
#include <setupapi.h>   // for SetupDiXxx functions.
#include <initguid.h>
//#include <cfgmgr32.h>   // for SetupDiXxx functions.
#include <ddk/ntdddisk.h>
#include <ddk/ntddscsi.h>
//#include <winioctl.h>
#if 0
#include <initguid.h>   // Guid definition
#include <devioctl.h>  
#include <ntddscsi.h>
#endif
#include <enum.h>

char *xGUID_DEVINTERFACE_DISK = "53f56307-b6bf-11d0-94f2-00a0c91efb8b";
//Guid MyDiskGUID = new Guid(xGUID_DEVINTERFACE_DISK);

VOID DebugPrint( USHORT DebugPrintLevel, PCHAR DebugMessage, ... )
/*++

Routine Description:

    This routine print the given string, if given debug level is <= to the
    current debug level.

Arguments:

    DebugPrintLevel - Debug level of the given message

    DebugMessage    - Message to be printed

Return Value:

  None

--*/
{

    va_list args;

    va_start(args, DebugMessage);

    if (DebugPrintLevel <= DebugLevel) {
        char buffer[128];
        (VOID) vsprintf(buffer, DebugMessage, args);
        printf( "%s", buffer );
    }

    va_end(args);
}


int __cdecl main()
/*++

Routine Description:

    This is the main function. It takes no arguments from the user.

Arguments:

    None

Return Value:

  Status

--*/
{
    HDEVINFO        hDevInfo, hIntDevInfo;
    DWORD           index;
    BOOL            status;



    //
    // Open the device using device interface registered by the driver
    //

    //
    // Get the interface device information set that contains all devices of event class.
    //

//                 (LPGUID)&DiskClassGuid,
    hIntDevInfo = SetupDiGetClassDevs (
                 &GUID_DEVINTERFACE_DISK,
                 NULL,                                   // Enumerator
                 NULL,                                   // Parent Window
                 (DIGCF_PRESENT | DIGCF_INTERFACEDEVICE  // Only Devices present & Interface class
                 ));

    if( hDevInfo == INVALID_HANDLE_VALUE ) {
        DebugPrint( 1, "SetupDiGetClassDevs failed with error: %d\n", GetLastError() );
        exit(1);
    }

    //
    //  Enumerate all the disk devices
    //
    index = 0;

    while (TRUE) 
    {
        DebugPrint( 1, "Properties for Device %d", index+1);
        status = GetRegistryProperty( hDevInfo, index );
        if ( status == FALSE ) {
            break;
        }

        status = GetDeviceProperty( hIntDevInfo, index );
        if ( status == FALSE ) {
            break;
        }
        index++;
    }
    DebugPrint( 1, "\r ***  End of Device List  *** \n");
    SetupDiDestroyDeviceInfoList(hDevInfo);
    SetupDiDestroyDeviceInfoList(hIntDevInfo);
    return 0;
}


BOOL GetRegistryProperty( HDEVINFO DevInfo, DWORD Index )
/*++

Routine Description:

    This routine enumerates the disk devices using the Setup class interface
    GUID GUID_DEVCLASS_DISKDRIVE. Gets the Device ID from the Registry 
    property.

Arguments:

    DevInfo - Handles to the device information list

    Index   - Device member 

Return Value:

  TRUE / FALSE. This decides whether to continue or not

--*/
{

    SP_DEVINFO_DATA deviceInfoData;
    DWORD           errorCode;
    DWORD           bufferSize = 0;
    DWORD           dataType;
    LPTSTR          buffer = NULL;
    BOOL            status;

    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    status = SetupDiEnumDeviceInfo(
                DevInfo,
                Index,
                &deviceInfoData);

    if ( status == FALSE ) {
        errorCode = GetLastError();
        if ( errorCode == ERROR_NO_MORE_ITEMS ) {
            DebugPrint( 2, "No more devices.\n");
        }
        else {
            DebugPrint( 1, "SetupDiEnumDeviceInfo failed with error: %d\n", errorCode );
        }
        return FALSE;
    }
        
    //
    // We won't know the size of the HardwareID buffer until we call
    // this function. So call it with a null to begin with, and then 
    // use the required buffer size to Alloc the necessary space.
    // Keep calling we have success or an unknown failure.
    //

    status = SetupDiGetDeviceRegistryProperty(
                DevInfo,
                &deviceInfoData,
                SPDRP_HARDWAREID,
                &dataType,
                (PBYTE)buffer,
                bufferSize,
                &bufferSize);

    if ( status == FALSE ) {
        errorCode = GetLastError();
        if ( errorCode != ERROR_INSUFFICIENT_BUFFER ) {
            if ( errorCode == ERROR_INVALID_DATA ) {
                //
                // May be a Legacy Device with no HardwareID. Continue.
                //
                return TRUE;
            }
            else {
                DebugPrint( 1, "SetupDiGetDeviceInterfaceDetail failed with error: %d\n", errorCode );
                return FALSE;
            }
        }
    }

    //
    // We need to change the buffer size.
    //

    buffer = LocalAlloc(LPTR, bufferSize);
    
    status = SetupDiGetDeviceRegistryProperty(
                DevInfo,
                &deviceInfoData,
                SPDRP_HARDWAREID,
                &dataType,
                (PBYTE)buffer,
                bufferSize,
                &bufferSize);

    if ( status == FALSE ) {
        errorCode = GetLastError();
        if ( errorCode == ERROR_INVALID_DATA ) {
            //
            // May be a Legacy Device with no HardwareID. Continue.
            //
            return TRUE;
        }
        else {
            DebugPrint( 1, "SetupDiGetDeviceInterfaceDetail failed with error: %d\n", errorCode );
            return FALSE;
        }
    }

    DebugPrint( 1, "\n\nDevice ID: %s\n",buffer );
    
    if (buffer) {
        LocalFree(buffer);
    }

    return TRUE;
}


BOOL GetDeviceProperty(HDEVINFO IntDevInfo, DWORD Index )
/*++

Routine Description:

    This routine enumerates the disk devices using the Device interface
    GUID DiskClassGuid. Gets the Adapter & Device property from the port
    driver. Then sends IOCTL through SPTI to get the device Inquiry data.

Arguments:

    IntDevInfo - Handles to the interface device information list

    Index      - Device member 

Return Value:

  TRUE / FALSE. This decides whether to continue or not

--*/
{
    SP_DEVICE_INTERFACE_DATA            interfaceData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA    interfaceDetailData = NULL;
    STORAGE_PROPERTY_QUERY              query;
    PSTORAGE_ADAPTER_DESCRIPTOR         adpDesc;
    PSTORAGE_DEVICE_DESCRIPTOR          devDesc;
    SCSI_PASS_THROUGH_WITH_BUFFERS      sptwb;
    HANDLE                              hDevice;
    BOOL                                status;
    PUCHAR                              p;
    UCHAR                               outBuf[512];
    ULONG                               length = 0,
                                        returned = 0,
                                        returnedLength;
    DWORD                               interfaceDetailDataSize,
                                        reqSize,
                                        errorCode, 
                                        i;


    interfaceData.cbSize = sizeof (SP_INTERFACE_DEVICE_DATA);

//                (LPGUID)&DiskClassGuid, // Interface registered by driver
    status = SetupDiEnumDeviceInterfaces ( 
                IntDevInfo,             // Interface Device Info handle
                0,                      // Device Info data
		&GUID_DEVINTERFACE_DISK,
                Index,                  // Member
                &interfaceData          // Device Interface Data
                );

    if ( status == FALSE ) {
        errorCode = GetLastError();
        if ( errorCode == ERROR_NO_MORE_ITEMS ) {
            DebugPrint( 2, "No more interfaces\n" );
        }
        else {
            DebugPrint( 1, "SetupDiEnumDeviceInterfaces failed with error: %d\n", errorCode  );
        }
        return FALSE;
    }
        
    //
    // Find out required buffer size, so pass NULL 
    //

    status = SetupDiGetDeviceInterfaceDetail (
                IntDevInfo,         // Interface Device info handle
                &interfaceData,     // Interface data for the event class
                NULL,               // Checking for buffer size
                0,                  // Checking for buffer size
                &reqSize,           // Buffer size required to get the detail data
                NULL                // Checking for buffer size
                );

    //
    // This call returns ERROR_INSUFFICIENT_BUFFER with reqSize 
    // set to the required buffer size. Ignore the above error and
    // pass a bigger buffer to get the detail data
    //

    if ( status == FALSE ) {
        errorCode = GetLastError();
        if ( errorCode != ERROR_INSUFFICIENT_BUFFER ) {
            DebugPrint( 1, "SetupDiGetDeviceInterfaceDetail failed with error: %d\n", errorCode   );
            return FALSE;
        }
    }

    //
    // Allocate memory to get the interface detail data
    // This contains the devicepath we need to open the device
    //

    interfaceDetailDataSize = reqSize;
    interfaceDetailData = malloc (interfaceDetailDataSize);
    if ( interfaceDetailData == NULL ) {
        DebugPrint( 1, "Unable to allocate memory to get the interface detail data.\n" );
        return FALSE;
    }
    interfaceDetailData->cbSize = sizeof (SP_INTERFACE_DEVICE_DETAIL_DATA);

    status = SetupDiGetDeviceInterfaceDetail (
                  IntDevInfo,               // Interface Device info handle
                  &interfaceData,           // Interface data for the event class
                  interfaceDetailData,      // Interface detail data
                  interfaceDetailDataSize,  // Interface detail data size
                  &reqSize,                 // Buffer size required to get the detail data
                  NULL);                    // Interface device info

    if ( status == FALSE ) {
        DebugPrint( 1, "Error in SetupDiGetDeviceInterfaceDetail failed with error: %d\n", GetLastError() );
        return FALSE;
    }

    //
    // Now we have the device path. Open the device interface
    // to send Pass Through command

    DebugPrint( 2, "Interface: %s\n", interfaceDetailData->DevicePath);

    hDevice = CreateFile(
                interfaceDetailData->DevicePath,    // device interface name
                GENERIC_READ | GENERIC_WRITE,       // dwDesiredAccess
                FILE_SHARE_READ | FILE_SHARE_WRITE, // dwShareMode
                NULL,                               // lpSecurityAttributes
                OPEN_EXISTING,                      // dwCreationDistribution
                0,                                  // dwFlagsAndAttributes
                NULL                                // hTemplateFile
                );
                
    //
    // We have the handle to talk to the device. 
    // So we can release the interfaceDetailData buffer
    //

    free (interfaceDetailData);

    if (hDevice == INVALID_HANDLE_VALUE) {
        DebugPrint( 1, "CreateFile failed with error: %d\n", GetLastError() );
        return TRUE;
    }

    query.PropertyId = StorageAdapterProperty;
    query.QueryType = PropertyStandardQuery;

    status = DeviceIoControl(
                        hDevice,                
                        IOCTL_STORAGE_QUERY_PROPERTY,
                        &query,
                        sizeof( STORAGE_PROPERTY_QUERY ),
                        &outBuf,                   
                        512,                      
                        &returnedLength,      
                        NULL                    
                        );
    if ( !status ) {
        DebugPrint( 1, "IOCTL failed with error code%d.\n\n", GetLastError() );
    }
    else {
        adpDesc = (PSTORAGE_ADAPTER_DESCRIPTOR) outBuf;
        DebugPrint( 1, "\nAdapter Properties\n");
        DebugPrint( 1, "------------------\n");
        DebugPrint( 1, "Bus Type       : %s\n", BusType[adpDesc->BusType]);
        DebugPrint( 1, "Max. Tr. Length: 0x%x\n", adpDesc->MaximumTransferLength );
        DebugPrint( 1, "Max. Phy. Pages: 0x%x\n", adpDesc->MaximumPhysicalPages );
        DebugPrint( 1, "Alignment Mask : 0x%x\n", adpDesc->AlignmentMask );

        query.PropertyId = StorageDeviceProperty;
        query.QueryType = PropertyStandardQuery;

        status = DeviceIoControl(
                            hDevice,                
                            IOCTL_STORAGE_QUERY_PROPERTY,
                            &query,
                            sizeof( STORAGE_PROPERTY_QUERY ),
                            &outBuf,                   
                            512,                      
                            &returnedLength,
                            NULL                    
                            );
        if ( !status ) {
            DebugPrint( 1, "IOCTL failed with error code%d.\n\n", GetLastError() );
        }
        else {
            DebugPrint( 1, "\nDevice Properties\n");
            DebugPrint( 1, "-----------------\n");
            devDesc = (PSTORAGE_DEVICE_DESCRIPTOR) outBuf;
            //
            // Our device table can handle only 16 devices.
            //
            DebugPrint( 1, "Device Type     : %s (0x%X)\n", 
                DeviceType[devDesc->DeviceType > 0x0F? 0x0F: devDesc->DeviceType ], devDesc->DeviceType);
            if ( devDesc->DeviceTypeModifier ) {
                DebugPrint( 1, "Device Modifier : 0x%x\n", devDesc->DeviceTypeModifier);
            }

            DebugPrint( 1, "Removable Media : %s\n", devDesc->RemovableMedia ? "Yes" : "No" );
            p = (PUCHAR) outBuf; 

            if ( devDesc->VendorIdOffset && p[devDesc->VendorIdOffset] ) {
                DebugPrint( 1, "Vendor ID       : " );
                for ( i = devDesc->VendorIdOffset; p[i] != (UCHAR) NULL && i < returnedLength; i++ ) {
                    DebugPrint( 1, "%c", p[i] );
                }
                DebugPrint( 1, "\n");
            }
            if ( devDesc->ProductIdOffset && p[devDesc->ProductIdOffset] ) {
                DebugPrint( 1, "Product ID      : " );
                for ( i = devDesc->ProductIdOffset; p[i] != (UCHAR) NULL && i < returnedLength; i++ ) {
                    DebugPrint( 1, "%c", p[i] );
                }
                DebugPrint( 1, "\n");
            }

            if ( devDesc->ProductRevisionOffset && p[devDesc->ProductRevisionOffset] ) {
                DebugPrint( 1, "Product Revision: " );
                for ( i = devDesc->ProductRevisionOffset; p[i] != (UCHAR) NULL && i < returnedLength; i++ ) {
                    DebugPrint( 1, "%c", p[i] );
                }
                DebugPrint( 1, "\n");
            }

            if ( devDesc->SerialNumberOffset && p[devDesc->SerialNumberOffset] ) {
                DebugPrint( 1, "Serial Number   : " );
                for ( i = devDesc->SerialNumberOffset; p[i] != (UCHAR) NULL && i < returnedLength; i++ ) {
                    DebugPrint( 1, "%c", p[i] );
                }
                DebugPrint( 1, "\n");
            }
        }
    }


    ZeroMemory(&sptwb,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));

    sptwb.Spt.Length = sizeof(SCSI_PASS_THROUGH);
    sptwb.Spt.PathId = 0;
    sptwb.Spt.TargetId = 1;
    sptwb.Spt.Lun = 0;
    sptwb.Spt.CdbLength = CDB6GENERIC_LENGTH;
    sptwb.Spt.SenseInfoLength = 24;
    sptwb.Spt.DataIn = SCSI_IOCTL_DATA_IN;
    sptwb.Spt.DataTransferLength = 192;
    sptwb.Spt.TimeOutValue = 2;
    sptwb.Spt.DataBufferOffset =
       offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,DataBuf);
    sptwb.Spt.SenseInfoOffset = 
       offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,SenseBuf);
    sptwb.Spt.Cdb[0] = SCSIOP_INQUIRY;
    sptwb.Spt.Cdb[4] = 192;
    length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,DataBuf) +
       sptwb.Spt.DataTransferLength;

    status = DeviceIoControl(hDevice,
                             IOCTL_SCSI_PASS_THROUGH,
                             &sptwb,
                             sizeof(SCSI_PASS_THROUGH),
                             &sptwb,
                             length,
                             &returned,
                             FALSE);

    PrintStatusResults(status, returned, &sptwb);

    //
    // Close handle the driver
    //
    if ( !CloseHandle(hDevice) )     {
        DebugPrint( 2, "Failed to close device.\n");
    }

    return TRUE;
}



VOID
PrintError(ULONG ErrorCode)
/*++

Routine Description:

    Prints formated error message

Arguments:

    ErrorCode   - Error code to print


Return Value:
    
      None
--*/
{
    UCHAR errorBuffer[80];
    ULONG count;

    count = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    ErrorCode,
                    0,
                    errorBuffer,
                    sizeof(errorBuffer),
                    NULL
                    );

    if (count != 0) {
        DebugPrint( 1, "%s\n", errorBuffer);
    } else {
        DebugPrint( 1, "Format message failed.  Error: %d\n", GetLastError());
    }
}

VOID
PrintDataBuffer(PUCHAR DataBuffer, ULONG BufferLength)
/*++

Routine Description:

    Prints the formated data buffer

Arguments:

    DataBuffer      - Buffer to be printed

    BufferLength    - Length of the buffer


Return Value:
    
      None
--*/
{
    ULONG cnt;

    DebugPrint( 3, "      00  01  02  03  04  05  06  07   08  09  0A  0B  0C  0D  0E  0F\n");
    DebugPrint( 3, "      ---------------------------------------------------------------\n");
    for (cnt = 0; cnt < BufferLength; cnt++) {
        if ((cnt) % 16 == 0) {
            DebugPrint( 3, " %03X  ",cnt);
            }
        DebugPrint( 3, "%02X  ", DataBuffer[cnt]);
        if ((cnt+1) % 8 == 0) {
            DebugPrint( 3, " ");
        }
        if ((cnt+1) % 16 == 0) {
            DebugPrint( 3, "\n");
        }
    }
    DebugPrint( 3, "\n\n");
}


VOID
PrintStatusResults( BOOL Status, DWORD Returned, PSCSI_PASS_THROUGH_WITH_BUFFERS Psptwb )
/*++

Routine Description:

    Prints the SCSI Inquiry data from the device

Arguments:

    Status      - Status of the DeviceIOControl

    Returned    - Number of bytes returned

    Psptwb      - SCSI pass through structure

Return Value:
    
      None
--*/
{
    ULONG   errorCode;
    USHORT  i, devType;

    DebugPrint( 1, "\nInquiry Data from Pass Through\n");
    DebugPrint( 1, "------------------------------\n");

    if (!Status ) {
        DebugPrint( 1, "Error: %d ", errorCode = GetLastError() );
        PrintError(errorCode);
        return;
    }
    if (Psptwb->Spt.ScsiStatus) {
        PrintSenseInfo(Psptwb);
        return;
    }
    else {
        devType = Psptwb->DataBuf[0] & 0x1f ;

        //
        // Our Device Table can handle only 16 devices.
        //
        DebugPrint( 1, "Device Type: %s (0x%X)\n", DeviceType[devType > 0x0F? 0x0F: devType], devType );

        DebugPrint( 1, "Vendor ID  : ");
        for (i = 8; i <= 15; i++) {
            DebugPrint( 1, "%c", Psptwb->DataBuf[i] );
        }
        DebugPrint( 1, "\nProduct ID : ");
        for (i = 16; i <= 31; i++) {
            DebugPrint( 1, "%c", Psptwb->DataBuf[i] );
        }
        DebugPrint( 1, "\nProduct Rev: ");
        for (i = 32; i <= 35; i++) {
            DebugPrint( 1, "%c", Psptwb->DataBuf[i] );
        }
        DebugPrint( 1, "\nVendor Str : ");
        for (i = 36; i <= 55; i++) {
            DebugPrint( 1, "%c", Psptwb->DataBuf[i] );
        }
        DebugPrint( 1, "\n\n");
        DebugPrint( 3, "Scsi status: %02Xh, Bytes returned: %Xh, ",
            Psptwb->Spt.ScsiStatus, Returned);
        DebugPrint( 3, "Data buffer length: %Xh\n\n\n",
            Psptwb->Spt.DataTransferLength);
        PrintDataBuffer( Psptwb->DataBuf, 192);
        DebugPrint( 1, "\n\n");
    }
}

VOID
PrintSenseInfo(PSCSI_PASS_THROUGH_WITH_BUFFERS Psptwb)
/*++

Routine Description:

    Prints the SCSI status and Sense Info from the device

Arguments:

    Psptwb      - Pass Through buffer that contains the Sense info


Return Value:
    
      None
--*/
{
    int i;

    DebugPrint( 1, "Scsi status: %02Xh\n\n", Psptwb->Spt.ScsiStatus);
    if (Psptwb->Spt.SenseInfoLength == 0) {
        return;
    }
    DebugPrint( 3, "Sense Info -- consult SCSI spec for details\n");
    DebugPrint( 3, "-------------------------------------------------------------\n");
    for (i=0; i < Psptwb->Spt.SenseInfoLength; i++) {
        DebugPrint( 3, "%02X ", Psptwb->SenseBuf[i]);
    }
    DebugPrint( 1, "\n\n");
}
#include <windows.h>
#include <winioctl.h>
BOOL GetDriveGeometry(DISK_GEOMETRY *pdg, char *drive)
{
  HANDLE hDevice;               // handle to the drive to be examined 
  BOOL bResult;                 // results flag
  DWORD junk;                   // discard results
char file[256];

	sprintf(file,"\\\\.\\%s",drive);
	printf("file: %s\n", file);
#if 0
  hDevice = CreateFile(TEXT("\\\\.\\PhysicalDrive0"),  // drive to open
#endif
  hDevice = CreateFile(file,  // drive to open
                    0,                // no access to the drive
                    FILE_SHARE_READ | // share mode
                    FILE_SHARE_WRITE, 
                    NULL,             // default security attributes
                    OPEN_EXISTING,    // disposition
                    0,                // file attributes
                    NULL);            // do not copy file attributes

  if (hDevice == INVALID_HANDLE_VALUE) // cannot open the drive
  {
    return (FALSE);
  }

  bResult = DeviceIoControl(hDevice,  // device to be queried
      IOCTL_DISK_GET_DRIVE_GEOMETRY,  // operation to perform
                             NULL, 0, // no input buffer
                            pdg, sizeof(*pdg),     // output buffer
                            &junk,                 // # bytes returned
                            (LPOVERLAPPED) NULL);  // synchronous I/O

  CloseHandle(hDevice);

  return (bResult);
}

int blah(char *drive)
{
  DISK_GEOMETRY pdg;            // disk drive geometry structure
  BOOL bResult;                 // generic results flag
  ULONGLONG DiskSize;           // size of the drive, in bytes

  bResult = GetDriveGeometry (&pdg, drive);

  if (bResult) 
  {
    printf("Cylinders = %lld\n", pdg.Cylinders);
    printf("Tracks/cylinder = %ld\n", (ULONG) pdg.TracksPerCylinder);
    printf("Sectors/track = %ld\n", (ULONG) pdg.SectorsPerTrack);
    printf("Bytes/sector = %ld\n", (ULONG) pdg.BytesPerSector);

    DiskSize = pdg.Cylinders.QuadPart * (ULONG)pdg.TracksPerCylinder *
      (ULONG)pdg.SectorsPerTrack * (ULONG)pdg.BytesPerSector;
    printf("Disk size = %I64d (Bytes) = %I64d (Gb)\n", DiskSize,
           DiskSize / (1024 * 1024 * 1024));
  } 
  else 
  {
    printf ("GetDriveGeometry failed. Error %ld.\n", GetLastError ());
  }

  return ((int)bResult);
}
#include <windows.h>
#include <stdio.h>

void DisplayVolumePaths(
        __in PWCHAR VolumeName
        )
{
    DWORD  CharCount = MAX_PATH + 1;
    PWCHAR Names     = NULL;
    PWCHAR NameIdx   = NULL;
    BOOL   Success   = FALSE;

    for (;;) 
    {
        //
        //  Allocate a buffer to hold the paths.
        Names = (PWCHAR) new BYTE [CharCount * sizeof(WCHAR)];

        if ( !Names ) 
        {
            //
            //  If memory can't be allocated, return.
            return;
        }

        //
        //  Obtain all of the paths
        //  for this volume.
        Success = GetVolumePathNamesForVolumeNameW(
            VolumeName, Names, CharCount, &CharCount
            );

        if ( Success ) 
        {
            break;
        }

        if ( GetLastError() != ERROR_MORE_DATA ) 
        {
            break;
        }

        //
        //  Try again with the
        //  new suggested size.
        delete [] Names;
        Names = NULL;
    }

    if ( Success )
    {
        //
        //  Display the various paths.
        for ( NameIdx = Names; 
              NameIdx[0] != L'\0'; 
              NameIdx += wcslen(NameIdx) + 1 ) 
        {
            wprintf(L"  %s", NameIdx);
        }
        wprintf(L"\n");
    }

    if ( Names != NULL ) 
    {
        delete [] Names;
        Names = NULL;
    }

    return;
}

void __cdecl wmain(void)
{
    DWORD  CharCount            = 0;
    WCHAR  DeviceName[MAX_PATH] = L"";
    DWORD  Error                = ERROR_SUCCESS;
    HANDLE FindHandle           = INVALID_HANDLE_VALUE;
    BOOL   Found                = FALSE;
    size_t Index                = 0;
    BOOL   Success              = FALSE;
    WCHAR  VolumeName[MAX_PATH] = L"";

    //
    //  Enumerate all volumes in the system.
    FindHandle = FindFirstVolumeW(VolumeName, ARRAYSIZE(VolumeName));

    if (FindHandle == INVALID_HANDLE_VALUE)
    {
        Error = GetLastError();
        wprintf(L"FindFirstVolumeW failed with error code %d\n", Error);
        return;
    }

    for (;;)
    {
        //
        //  Skip the \\?\ prefix and remove the trailing backslash.
        Index = wcslen(VolumeName) - 1;

        if (VolumeName[0]     != L'\\' ||
            VolumeName[1]     != L'\\' ||
            VolumeName[2]     != L'?'  ||
            VolumeName[3]     != L'\\' ||
            VolumeName[Index] != L'\\') 
        {
            Error = ERROR_BAD_PATHNAME;
            wprintf(L"FindFirstVolumeW/FindNextVolumeW returned a bad path: %s\n", VolumeName);
            break;
        }

        //
        //  QueryDosDeviceW doesn't allow a trailing backslash,
        //  so temporarily remove it.
        VolumeName[Index] = L'\0';

        CharCount = QueryDosDeviceW(&VolumeName[4], DeviceName, ARRAYSIZE(DeviceName)); 

        VolumeName[Index] = L'\\';

        if ( CharCount == 0 ) 
        {
            Error = GetLastError();
            wprintf(L"QueryDosDeviceW failed with error code %d\n", Error);
            break;
        }

        wprintf(L"\nFound a device:\n %s", DeviceName);
        wprintf(L"\nVolume name: %s", VolumeName);
        wprintf(L"\nPaths:");
        DisplayVolumePaths(VolumeName);

        //
        //  Move on to the next volume.
        Success = FindNextVolumeW(FindHandle, VolumeName, ARRAYSIZE(VolumeName));

        if ( !Success ) 
        {
            Error = GetLastError();

            if (Error != ERROR_NO_MORE_FILES) 
            {
                wprintf(L"FindNextVolumeW failed with error code %d\n", Error);
                break;
            }

            //
            //  Finished iterating
            //  through all the volumes.
            Error = ERROR_SUCCESS;
            break;
        }
    }

    FindVolumeClose(FindHandle);
    FindHandle = INVALID_HANDLE_VALUE;

    return;
}

#endif
#endif
