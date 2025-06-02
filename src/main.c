#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include <Windows.h>
#include <evntrace.h>
#include <evntprov.h>

// #define REAL_TIME_MODE

struct EventTracePropertyData {
    EVENT_TRACE_PROPERTIES_V2 props;
#if !defined(REAL_TIME_MODE)
    char log_file_name[128];
#endif
    char logger_name[1024];
    char buffer[64 * 1024U];
} data = {
    .props = {
        .Wnode = {
            .BufferSize = offsetof(struct EventTracePropertyData, buffer),
            .Flags = WNODE_FLAG_TRACED_GUID | WNODE_FLAG_VERSIONED_PROPERTIES,
            .Guid = { 0x9E814AAD, 0x3204, 0x11D2, 0x9A, 0x82, 0x00, 0x60, 0x08, 0xA8, 0x69, 0x39 },
        },
        .BufferSize = sizeof(data.buffer),
        .MinimumBuffers = 4U,
        .MaximumBuffers = 4U,
        .EnableFlags =  // EVENT_TRACE_FLAG_PROFILE |
                        EVENT_TRACE_FLAG_INTERRUPT |
                        EVENT_TRACE_FLAG_SYSTEMCALL |
                        EVENT_TRACE_FLAG_DPC |
                        EVENT_TRACE_FLAG_THREAD |
                        EVENT_TRACE_FLAG_CSWITCH |
                        EVENT_TRACE_FLAG_DISPATCHER |
                        EVENT_TRACE_FLAG_PROCESS |
                        EVENT_TRACE_FLAG_PROCESS_COUNTERS,
        .LogFileMode = EVENT_TRACE_SYSTEM_LOGGER_MODE,
#if defined(REAL_TIME_MODE)
        .LogFileMode = EVENT_TRACE_REAL_TIME_MODE,
#else
        .LogFileNameOffset = offsetof(struct EventTracePropertyData, log_file_name),
#endif
        .LoggerNameOffset = offsetof(struct EventTracePropertyData, logger_name),
    },
    .log_file_name = "test_etw_log.etl",
    .logger_name = KERNEL_LOGGER_NAMEA,
};

GUID system_profile_provider_guid = { 0xbfeb0324, 0x1cee, 0x496f, 0xa4, 0x09, 0x2a, 0xc2, 0xb4, 0x8a, 0x63, 0x22 };
GUID thread_provider_guid = { 0x3d6fa8d1, 0xfe05, 0x11d0, 0x9d, 0xda, 0x00, 0xc0, 0x4f, 0xd7, 0xba, 0x7c };
GUID process_provider_guid = { 0x3d6fa8d0, 0xfe05, 0x11d0, 0x9d, 0xda, 0x00, 0xc0, 0x4f, 0xd7, 0xba, 0x7c };
GUID perf_info_provider_guid = { 0xce1dbfb4, 0x137e, 0x4da6, 0x87, 0xb0, 0x3f, 0x59, 0xaa, 0x10, 0x2c, 0xbc };

void LogError(DWORD dw) 
{ 
    // Retrieve the system error message for the last-error code
    LPVOID lpMsgBuf;

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
    printf("error %lu: %s\n", dw, (char*)lpMsgBuf);

    LocalFree(lpMsgBuf);
}

int main() {
    LPSTR log_file_name = "test_etw_log.etl";
    TRACEHANDLE trace_handle = 0U;
    ULONG res = 0U;
    if ((res = StartTraceA(&trace_handle, KERNEL_LOGGER_NAMEA, (EVENT_TRACE_PROPERTIES*)&data.props)) != ERROR_SUCCESS) {
        LogError(res);
        goto close_trace;
    }

    char* profile_source_infos[sizeof(PROFILE_SOURCE_INFO) * 256U];
    ULONG bwrite = 0U;
    if ((res = TraceQueryInformation(0U, TraceProfileSourceListInfo, (void*)profile_source_infos, sizeof(profile_source_infos), &bwrite)) != ERROR_SUCCESS) {
        LogError(res);
        goto close_trace;
    }

    ULONG masks[8U] = {
        // EVENT_TRACE_FLAG_PROFILE |
        EVENT_TRACE_FLAG_INTERRUPT |
        EVENT_TRACE_FLAG_SYSTEMCALL |
        EVENT_TRACE_FLAG_DPC |
        EVENT_TRACE_FLAG_THREAD |
        EVENT_TRACE_FLAG_CSWITCH |
        EVENT_TRACE_FLAG_DISPATCHER |
        EVENT_TRACE_FLAG_PROCESS |
        EVENT_TRACE_FLAG_PROCESS_COUNTERS,
        0x20000400, // PERF_PERF_PMC
        0U,
        0U,
        0U,
        0U,
        0U,
        0U,
    };
    if ((res = TraceSetInformation(trace_handle, TraceSystemTraceEnableFlagsInfo, &masks, sizeof(masks))) != ERROR_SUCCESS) {
        LogError(res);
        goto close_trace;
    }

    uint32_t pmc_sources[] = { 6U, 19U, }; // instructions count, branch count
    if ((res = TraceSetInformation(trace_handle, TracePmcCounterListInfo, pmc_sources, sizeof(pmc_sources))) != ERROR_SUCCESS) { // PMCs to collect
        LogError(res);
        goto close_trace;
    }

    CLASSIC_EVENT_ID events[] = {
        {
            .EventGuid = perf_info_provider_guid,
            .Type = 46U,
            .Reserved = { 0U },
        },
        {
            .EventGuid = perf_info_provider_guid,
            .Type = 51U,
            .Reserved = { 0U },
        },
        {
            .EventGuid = perf_info_provider_guid,
            .Type = 52U,
            .Reserved = { 0U },
        },
        {
            .EventGuid = perf_info_provider_guid,
            .Type = 66U,
            .Reserved = { 0U },
        },
        {
            .EventGuid = perf_info_provider_guid,
            .Type = 67U,
            .Reserved = { 0U },
        },
        {
            .EventGuid = perf_info_provider_guid,
            .Type = 68U,
            .Reserved = { 0U },
        },
        {
            .EventGuid = perf_info_provider_guid,
            .Type = 69U,
            .Reserved = { 0U },
        },
        {
            .EventGuid = thread_provider_guid,
            .Type = 1U,
            .Reserved = { 0U },
        },
        {
            .EventGuid = thread_provider_guid,
            .Type = 2U,
            .Reserved = { 0U },
        },
        {
            .EventGuid = thread_provider_guid,
            .Type = 3U,
            .Reserved = { 0U },
        },
        {
            .EventGuid = thread_provider_guid,
            .Type = 4U,
            .Reserved = { 0U },
        },
        {
            .EventGuid = thread_provider_guid,
            .Type = 36U,
            .Reserved = { 0U },
        },
        {
            .EventGuid = thread_provider_guid,
            .Type = 50U,
            .Reserved = { 0U },
        },
        {
            .EventGuid = process_provider_guid,
            .Type = 1U,
            .Reserved = { 0U },
        },
        {
            .EventGuid = process_provider_guid,
            .Type = 2U,
            .Reserved = { 0U },
        },
        {
            .EventGuid = process_provider_guid,
            .Type = 3U,
            .Reserved = { 0U },
        },
         {
            .EventGuid = process_provider_guid,
            .Type = 4U,
            .Reserved = { 0U },
        },
        {
            .EventGuid = process_provider_guid,
            .Type = 32U,
            .Reserved = { 0U },
        },
        {
            .EventGuid = process_provider_guid,
            .Type = 33U,
            .Reserved = { 0U },
        },
        {
            .EventGuid = process_provider_guid,
            .Type = 39U,
            .Reserved = { 0U },
        },
    };
    uint32_t index = 0U;
    if ((res = TraceSetInformation(trace_handle, TracePmcEventListInfo, &events[index], sizeof(events))) != ERROR_SUCCESS) { // Events on which we collect PMCs
        LogError(res);
        goto close_trace;
    }
    // Looks like we aren't allowed to use system providers without some special previlegies, to be investigated
    // EnableTraceEx2(trace_handle, &system_profile_provider_guid, EVENT_CONTROL_CODE_ENABLE_PROVIDER, TRACE_LEVEL_VERBOSE, ~0U, 0U, 0U, NULL);
    if ((res = EnableTraceEx2(trace_handle, &perf_info_provider_guid, EVENT_CONTROL_CODE_ENABLE_PROVIDER, TRACE_LEVEL_VERBOSE, ~0U, 0U, 0U, NULL)) != ERROR_SUCCESS) {
        LogError(res);
        goto close_trace;
    }
    if ((res = EnableTraceEx2(trace_handle, &process_provider_guid, EVENT_CONTROL_CODE_ENABLE_PROVIDER, TRACE_LEVEL_VERBOSE, ~0U, 0U, 0U, NULL)) != ERROR_SUCCESS) {
        LogError(res);
        goto close_trace;
    }
    if ((res = EnableTraceEx2(trace_handle, &thread_provider_guid, EVENT_CONTROL_CODE_ENABLE_PROVIDER, TRACE_LEVEL_VERBOSE, ~0U, 0U, 0U, NULL)) != ERROR_SUCCESS) {
        LogError(res);
        goto close_trace;
    }

    int i = 0;
    float f = 0.5f;
    while (i < 4) {
        Sleep(1000U);
        f = sinf(f);
        i++;
    }


    // EVENT_TRACE_LOGFILEA log = { 0U };
    // strncpy(log.LogFileName, log_file_name, strlen(log_file_name));
    // strncpy(log.LoggerName, logger_name, strlen(logger_name));
    // TRACEHANDLE pt_handle = OpenTraceA(&log);
    // if (pt_handle == INVALID_PROCESSTRACE_HANDLE) {
    //     ErrorExit();
    //     return 1;
    // }
    // uint32_t counter = 0U;

    // while (counter < ~0U) {
    //     // counter++;
    // }


    if ((res = EnableTraceEx2(trace_handle, &thread_provider_guid, EVENT_CONTROL_CODE_DISABLE_PROVIDER, TRACE_LEVEL_VERBOSE, ~0U, 0U, 0U, NULL)) != ERROR_SUCCESS) {
        LogError(res);
        goto close_trace;
    }
    if ((res = EnableTraceEx2(trace_handle, &process_provider_guid, EVENT_CONTROL_CODE_DISABLE_PROVIDER, TRACE_LEVEL_VERBOSE, ~0U, 0U, 0U, NULL)) != ERROR_SUCCESS) {
        LogError(res);
        goto close_trace;
    }
    if ((res = EnableTraceEx2(trace_handle, &perf_info_provider_guid, EVENT_CONTROL_CODE_DISABLE_PROVIDER, TRACE_LEVEL_VERBOSE, ~0U, 0U, 0U, NULL)) != ERROR_SUCCESS) {
        LogError(res);
        goto close_trace;
        goto close_trace;
    }


    PROFILE_SOURCE_INFO* cur_profile_source_info = (PROFILE_SOURCE_INFO*)&profile_source_infos[0U];
    while (1) {
        printf("%S\n", cur_profile_source_info->Description);

        if (cur_profile_source_info->NextEntryOffset == 0U) {
            break;
        }

        cur_profile_source_info = (PROFILE_SOURCE_INFO*)((char*)cur_profile_source_info + cur_profile_source_info->NextEntryOffset);
    }

close_trace:
    if ((res = ControlTraceA(trace_handle, KERNEL_LOGGER_NAMEA, (EVENT_TRACE_PROPERTIES*)&data.props, EVENT_TRACE_CONTROL_STOP)) != ERROR_SUCCESS) {
        LogError(res);
        return 1;
    }

    return 0;
}
