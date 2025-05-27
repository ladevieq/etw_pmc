#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include <Windows.h>
#include <evntrace.h>
#include <evntprov.h>
#include <TraceLoggingProvider.h>

#include <strsafe.h>

// #define REAL_TIME_MODE

struct EventTracePropertyData {
    EVENT_TRACE_PROPERTIES props;
#if !defined(REAL_TIME_MODE)
    char log_file_name[128];
#endif
    char logger_name[1024];
    char buffer[64 * 1024U];
};

// TRACELOGGING_DEFINE_PROVIDER(
//     provider_handle,
//     "test_etw_provider",
//     ( 0x9c898339, 0x0c32, 0x5d5d, 0xa6, 0x87, 0x43, 0x19, 0x59, 0xce, 0xf0, 0x8d )
// );

void ErrorExit() 
{ 
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message and exit the process
    printf("%s\n", (char*)lpMsgBuf);

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR)lpMsgBuf) + 40) * sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, 
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("failed with error %d: %s"), 
        dw, lpMsgBuf); 
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(dw); 
}

int main() {
    // TraceLoggingRegister(provider_handle);

    LPSTR log_file_name = "test_etw_log.etl";
    LPSTR logger_name = "test_etw_logger";
    LPCSTR trace_name = "test_trace";
    struct EventTracePropertyData data = { 0U };
    // data.props.Wnode.BufferSize = sizeof(data);
    data.props.Wnode.BufferSize = offsetof(struct EventTracePropertyData, buffer);
    data.props.Wnode.Flags = WNODE_FLAG_TRACED_GUID;
    data.props.BufferSize = sizeof(data.buffer);
    data.props.MinimumBuffers = 4U;
    data.props.MaximumBuffers = 4U;
    data.props.EnableFlags = EVENT_TRACE_FLAG_PROFILE;
#if defined(REAL_TIME_MODE)
    data.props.LogFileMode = EVENT_TRACE_REAL_TIME_MODE;
#else
    data.props.LogFileNameOffset = offsetof(struct EventTracePropertyData, log_file_name);
#endif
    data.props.LoggerNameOffset = offsetof(struct EventTracePropertyData, logger_name);

    strncpy(data.log_file_name, log_file_name, strlen(log_file_name));
    // strncpy(data.logger_name, logger_name, strlen(logger_name));

    TRACEHANDLE trace_handle = 0U;
    // EVENT_TRACE_PROPERTIES trace_props = { 0U };
    if (StartTrace(&trace_handle, trace_name, &data.props) != ERROR_SUCCESS) {
        ErrorExit();
        return 1;
    }

    HRESULT hr = 0;
    char* profile_source_infos[sizeof(PROFILE_SOURCE_INFO) * 256U];
    ULONG bwrite = 0U;
    hr = TraceQueryInformation(0U, TraceProfileSourceListInfo, (void*)profile_source_infos, sizeof(profile_source_infos), &bwrite);

    // TraceSetInformation(trace_handle, TracePmcCounterListInfo, ) // PMCs to collect
    // TraceSetInformation(trace_handle, TracePmcEventListInfo) // Events on which we collect PMCs


    // EVENT_TRACE_LOGFILEA log = { 0U };
    // strncpy(log.LogFileName, log_file_name, strlen(log_file_name));
    // strncpy(log.LoggerName, logger_name, strlen(logger_name));
    // TRACEHANDLE pt_handle = OpenTraceA(&log);
    // if (pt_handle == INVALID_PROCESSTRACE_HANDLE) {
    //     ErrorExit();
    //     return 1;
    // }

    // EnableTraceEx2(trace_handle, provider)
    // uint32_t counter = 0U;

    // while (counter < ~0U) {
    //     // counter++;
    // }


    ControlTrace(trace_handle, trace_name, &data.props, EVENT_TRACE_CONTROL_STOP);
    // StopTrace(trace_handle, "TestInstance", &trace_props);

    PROFILE_SOURCE_INFO* cur_profile_source_info = (PROFILE_SOURCE_INFO*)&profile_source_infos[0U];
    while (1) {
        printf("%S\n", cur_profile_source_info->Description);

        if (cur_profile_source_info->NextEntryOffset == 0U) {
            break;
        }

        cur_profile_source_info = (PROFILE_SOURCE_INFO*)((char*)cur_profile_source_info + cur_profile_source_info->NextEntryOffset);
    }

    // TraceLoggingUnregister(provider_handle);

    return 0;
}
