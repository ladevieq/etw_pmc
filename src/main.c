#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

#include <Windows.h>

#define INITGUID
#include <evntrace.h>
#include <evntprov.h>
#include <evntcons.h>

#include "events.h"

typedef uint32_t u32;

// #define REAL_TIME_MODE

#define SOURCE_TIMER "Timer"
#define SOURCE_TIMER_INDEX 0U
#define SOURCE_TOTAL_ISSUES "TotalIssues"
#define SOURCE_TOTAL_ISSUES_INDEX 2U
#define SOURCE_BRANCH_INSTRUCTIONS "BranchInstructions"
#define SOURCE_BRANCH_INSTRUCTIONS_INDEX 6U

const char* PMC_SOURCES[] = {
    SOURCE_TIMER,
    "",
    SOURCE_TOTAL_ISSUES,
    "",
    "",
    "",
    SOURCE_BRANCH_INSTRUCTIONS,
    "",
    "DcacheMisses",
    "IcacheMisses",
    "CacheMisses",
    "BranchMispredictions",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "TotalCycles",
    "IcacheIssues",
    "DcacheAccesses",
    "",
    "",
    "",
    "",
    "InstructionsRetired",
    "IcacheL2Hit",
    "DcacheL2Hit",
    "RetiredMacroOps",
    "RetiredTakenBranchInstructions",
    "RetiredTakenBranchMispredictions",
    "RetiredFarControlTransfers",
    "RetiredNearReturns",
    "RetiredNearReturnMispredictions",
    "RetiredIndirectBranchMispredictions",
    "RetiredConditionalBranchInstructions",
    "RetiredConditionalBranchMispredictions",
    "RetiredFusedInstructions",
    "VariableTargetPredictions",
    "EarlyRedirects",
    "InefectiveSoftwarePrefetches",
    "TlbFlushes",
    "SmiReceived",
    "DemandDcacheFills",
    "RetiredLockInstructions",
    "BusLock",
    "Retiredx87Instructions",
    "RetiredMmxInstructions",
    "RetiredSseInstructions",
};
const u32 PMC_SOURCES_COUNT = 49U;

void log_error(DWORD dw) {
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

struct event_trace_property_data {
    EVENT_TRACE_PROPERTIES_V2 props;
#if !defined(REAL_TIME_MODE)
    char log_file_name[128];
#endif
    char logger_name[1024];
    char buffer[256 * 1024U];
} data = {
    .props = {
        .Wnode = {
            .BufferSize = offsetof(struct event_trace_property_data, buffer),
            .Flags = WNODE_FLAG_TRACED_GUID | WNODE_FLAG_VERSIONED_PROPERTIES,
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
        .LogFileNameOffset = offsetof(struct event_trace_property_data, log_file_name),
#endif
        .LoggerNameOffset = offsetof(struct event_trace_property_data, logger_name),
    },
#if !defined(REAL_TIME_MODE)
    .log_file_name = "test_etw_log.etl",
#endif
    .logger_name = KERNEL_LOGGER_NAMEA,
};

DWORD pid = 0U;

void PeventRecordCallback(PEVENT_RECORD pEventRecord) {
    // if (pEventRecord->EventHeader.ProcessId != pid) {
    //     return;
    // }
    EVENT_HEADER* header = &pEventRecord->EventHeader;
    u32* sources = (u32*)pEventRecord->UserContext;

    PEVENT_HEADER_EXTENDED_DATA_ITEM extended_data = pEventRecord->ExtendedData;
    PEVENT_HEADER_EXTENDED_DATA_ITEM cur_data = extended_data;
    for (u32 index = 0U; index < pEventRecord->ExtendedDataCount; index++, cur_data++) {
        if (cur_data->ExtType == EVENT_HEADER_EXT_TYPE_PMC_COUNTERS) {
            printf("PID: %lx, TID: %lx\n", header->ProcessId, header->ThreadId);
            ULONG64* pmc = (ULONG64*)cur_data->DataPtr;
            size_t pmc_count = cur_data->DataSize / sizeof(pmc[0U]);
            for (u32 pmc_index = 0U; pmc_index < pmc_count; pmc_index++, pmc++) {
                printf("%s: %llu\n", PMC_SOURCES[sources[pmc_index]], *pmc);
            }
        }
    }
}

EVENT_TRACE_LOGFILEA trace_log = {
#if defined(REAL_TIME_MODE)
    .LoggerName = KERNEL_LOGGER_NAMEA,
    .ProcessTraceMode = PROCESS_TRACE_MODE_REAL_TIME | PROCESS_TRACE_MODE_EVENT_RECORD,
#else
    .LogFileName = "test_etw_log.etl",
    .ProcessTraceMode = PROCESS_TRACE_MODE_EVENT_RECORD,
#endif
    .EventRecordCallback = &PeventRecordCallback,
    // .LogFileMode = EVENT_TRACE_SYSTEM_LOGGER_MODE,
};

u32 end_trace(TRACEHANDLE handle) {
    ULONG res = 0U;
    if ((res = ControlTraceA(handle, KERNEL_LOGGER_NAMEA, (EVENT_TRACE_PROPERTIES*)&data.props, EVENT_TRACE_CONTROL_STOP)) != ERROR_SUCCESS) {
        log_error(res);
        return 1U;
    }
    return 0U;
}

u32 begin_trace(TRACEHANDLE* handle, u32* pmc_sources, u32 sources_size) {
    ULONG res = 0U;

    data.props.Wnode.Guid = SystemTraceControlGuid;
    if ((res = StartTraceA(handle, KERNEL_LOGGER_NAMEA, (EVENT_TRACE_PROPERTIES*)&data.props)) != ERROR_SUCCESS) {
        log_error(res);
        return 1U;
    }

    if ((res = TraceSetInformation(*handle, TracePmcCounterListInfo, pmc_sources, sources_size)) != ERROR_SUCCESS) { // PMCs to collect
        log_error(res);
        end_trace(*handle);
        return 1U;
    }

    return 0U;
}

u32 begin_PMCs_capture(TRACEHANDLE handle) {
    CLASSIC_EVENT_ID events[] = {
        EventProcessPMC,
        EventProcessPMCRundown,
    };
    ULONG res = 0U;
    if ((res = TraceSetInformation(handle, TracePmcEventListInfo, &events, sizeof(events))) != ERROR_SUCCESS) { // Events on which we collect PMCs
        log_error(res);
        end_trace(handle);
        return 1U;
    }
    return 0U;
}

u32 end_PMCs_capture(TRACEHANDLE handle) {
    ULONG res = 0U;
    // TODO: Make it stop
    // if ((res = TraceSetInformation(handle, TracePmcEventListInfo, NULL, 0U)) != ERROR_SUCCESS) {
    //     log_error(res);
    //     end_trace(handle);
    //     return 1U;
    // }
    return 0U;
}

u32 PMC_sources_from_names(const char** names, u32 names_count, u32* sources, u32* sources_count) {
    for (u32 name_index = 0U; name_index < names_count; name_index++) {
        for (u32 list_index = 0U; list_index < PMC_SOURCES_COUNT; list_index++) {
            if (strcmp(names[name_index], PMC_SOURCES[list_index]) == 0U) {
                sources[*sources_count] = list_index;
                (*sources_count)++;
            }
        }
    }

    return (names_count != *sources_count);
}

u32 list_sources() {
    char* profile_source_infos[sizeof(PROFILE_SOURCE_INFO) * 256U];
    ULONG bwrite = 0U;
    ULONG res = 0U;
    if ((res = TraceQueryInformation(0U, TraceProfileSourceListInfo, (void*)profile_source_infos, sizeof(profile_source_infos), &bwrite)) != ERROR_SUCCESS) {
        log_error(res);
        return 1U;
    }

    PROFILE_SOURCE_INFO* cur_profile_source_info = (PROFILE_SOURCE_INFO*)&profile_source_infos[0U];
    while (1) {
        printf("%lu: %S\n", cur_profile_source_info->Source, cur_profile_source_info->Description);

        if (cur_profile_source_info->NextEntryOffset == 0U) {
            break;
        }

        cur_profile_source_info = (PROFILE_SOURCE_INFO*)((char*)cur_profile_source_info + cur_profile_source_info->NextEntryOffset);
    }

    return 0U;
}

int main() {
    pid = GetProcessId(GetCurrentProcess());

    TRACEHANDLE handle;
    u32 PMCs[] = {
        SOURCE_TOTAL_ISSUES_INDEX,
        SOURCE_BRANCH_INSTRUCTIONS_INDEX,
    };
    static_assert(sizeof(PMCs) / sizeof(PMCs[0U]) <= 4U, "Too much PMCs");
    if (begin_trace(&handle, PMCs, sizeof(PMCs))) {
        end_trace(handle);
        return 1;
    }

    ULONG res = 0U;
#ifdef REAL_TIME_MODE
    TRACEHANDLE pt_handle = OpenTraceA(&trace_log);
    if (pt_handle == INVALID_PROCESSTRACE_HANDLE) {
        log_error(GetLastError());
        end_trace(handle);
    }

    if ((res = ProcessTrace(&pt_handle, 1U, NULL, NULL)) != ERROR_SUCCESS) {
        log_error(res);
        end_trace(handle);
    }
#endif

    // ULONG masks[8U] = {
    //     // EVENT_TRACE_FLAG_PROFILE |
    //     EVENT_TRACE_FLAG_INTERRUPT |
    //     EVENT_TRACE_FLAG_SYSTEMCALL |
    //     EVENT_TRACE_FLAG_DPC |
    //     EVENT_TRACE_FLAG_THREAD |
    //     EVENT_TRACE_FLAG_CSWITCH |
    //     EVENT_TRACE_FLAG_DISPATCHER |
    //     EVENT_TRACE_FLAG_PROCESS |
    //     EVENT_TRACE_FLAG_PROCESS_COUNTERS,
    //     // 0x20000400, // PERF_PERF_PMC
    //     0U,
    //     0U,
    //     0U,
    //     0U,
    //     0U,
    //     0U,
    // };
    // if ((res = TraceSetInformation(trace_handle, TraceSystemTraceEnableFlagsInfo, &masks, sizeof(masks))) != ERROR_SUCCESS) {
    //     log_error(res);
    //     end_trace(handle);
    // }

    // if ((res = EnableTraceEx2(trace_handle, &SystemProcessProviderGuid, EVENT_CONTROL_CODE_ENABLE_PROVIDER, TRACE_LEVEL_VERBOSE, ~0U, 0U, 0U, NULL)) != ERROR_SUCCESS) {
    //     log_error(res);
    //     end_trace(handle);
    // }

    if (begin_PMCs_capture(handle)) {
        end_trace(handle);
        return 1;
    }

    int i = 0;
    float f = 0.5f;
    while (i < 4) {
        Sleep(1000U);
        f = sinf(f);
        i++;
    }

    if (end_PMCs_capture(handle)) {
        end_trace(handle);
        return 1;
    }

    // if ((res = EnableTraceEx2(trace_handle, &SystemProcessProviderGuid, EVENT_CONTROL_CODE_DISABLE_PROVIDER, TRACE_LEVEL_VERBOSE, ~0U, 0U, 0U, NULL)) != ERROR_SUCCESS) {
    //     log_error(res);
    //     end_trace(handle);
    // }

    if (end_trace(handle)) {
        return 1;
    }

#if !defined(REAL_TIME_MODE)
    trace_log.Context = PMCs;
    TRACEHANDLE pt_handle = OpenTraceA(&trace_log);
    if (pt_handle == INVALID_PROCESSTRACE_HANDLE) {
        log_error(GetLastError());
    }

    if ((res = ProcessTrace(&pt_handle, 1U, NULL, NULL)) != ERROR_SUCCESS) {
        log_error(res);
    }
#endif

    if ((res = CloseTrace(pt_handle)) != ERROR_SUCCESS) {
        log_error(res);
        return 1;
    }

    return 0;
}
