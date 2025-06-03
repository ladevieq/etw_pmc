#include <evntrace.h>

#define THREAD_EVENTS_GUID    { 0x3d6fa8d1,    0xfe05,    0x11d0,    0x9d, 0xda, 0x00, 0xc0, 0x4f, 0xd7, 0xba, 0x7c }
#define PROCESS_EVENTS_GUID   { 0x3d6fa8d0,    0xfe05,    0x11d0,    0x9d, 0xda, 0x00, 0xc0, 0x4f, 0xd7, 0xba, 0x7c }
#define PERF_INFO_EVENTS_GUID { 0xce1dbfb4,    0x137e,    0x4da6,    0x87, 0xb0, 0x3f, 0x59, 0xaa, 0x10, 0x2c, 0xbc }


CLASSIC_EVENT_ID EventSampledProfile = {
    .EventGuid = PERF_INFO_EVENTS_GUID,
    .Type = 46U,
    .Reserved = { 0U },
};
CLASSIC_EVENT_ID EventSysCallEnter = {
    .EventGuid = PERF_INFO_EVENTS_GUID,
    .Type = 51U,
    .Reserved = { 0U },
};
CLASSIC_EVENT_ID EventSysCallExit = {
    .EventGuid = PERF_INFO_EVENTS_GUID,
    .Type = 52U,
    .Reserved = { 0U },
};
CLASSIC_EVENT_ID EventThreadedDPC = {
    .EventGuid = PERF_INFO_EVENTS_GUID,
    .Type = 66U,
    .Reserved = { 0U },
};
CLASSIC_EVENT_ID EventISR = {
    .EventGuid = PERF_INFO_EVENTS_GUID,
    .Type = 67U,
    .Reserved = { 0U },
};
CLASSIC_EVENT_ID EventDPC = {
    .EventGuid = PERF_INFO_EVENTS_GUID,
    .Type = 68U,
    .Reserved = { 0U },
};
CLASSIC_EVENT_ID EventDPCTimer = {
    .EventGuid = PERF_INFO_EVENTS_GUID,
    .Type = 69U,
    .Reserved = { 0U },
};
CLASSIC_EVENT_ID EventThreadStart = {
    .EventGuid = THREAD_EVENTS_GUID,
    .Type = 1U,
    .Reserved = { 0U },
};
CLASSIC_EVENT_ID EventThreadEnd = {
    .EventGuid = THREAD_EVENTS_GUID,
    .Type = 2U,
    .Reserved = { 0U },
};
CLASSIC_EVENT_ID EventDataCollectionStart = {
    .EventGuid = THREAD_EVENTS_GUID,
    .Type = 3U,
    .Reserved = { 0U },
};
CLASSIC_EVENT_ID EventThreadDataCollectionEnd = {
    .EventGuid = THREAD_EVENTS_GUID,
    .Type = 4U,
    .Reserved = { 0U },
};
CLASSIC_EVENT_ID EventThreadContextSwitch = {
    .EventGuid = THREAD_EVENTS_GUID,
    .Type = 36U,
    .Reserved = { 0U },
};
CLASSIC_EVENT_ID EventThreadReady = {
    .EventGuid = THREAD_EVENTS_GUID,
    .Type = 50U,
    .Reserved = { 0U },
};
CLASSIC_EVENT_ID EventProcessStart = {
    .EventGuid = PROCESS_EVENTS_GUID,
    .Type = 1U,
    .Reserved = { 0U },
};
CLASSIC_EVENT_ID EventProcessEnd = {
    .EventGuid = PROCESS_EVENTS_GUID,
    .Type = 2U,
    .Reserved = { 0U },
};
CLASSIC_EVENT_ID EventProcessDataCollectionStart = {
    .EventGuid = PROCESS_EVENTS_GUID,
    .Type = 3U,
    .Reserved = { 0U },
};
CLASSIC_EVENT_ID EventProcessDataCollectionEnd = {
    .EventGuid = PROCESS_EVENTS_GUID,
    .Type = 4U,
    .Reserved = { 0U },
};
CLASSIC_EVENT_ID EventProcessPMC = {
    .EventGuid = PROCESS_EVENTS_GUID,
    .Type = 32U,
    .Reserved = { 0U },
};
CLASSIC_EVENT_ID EventProcessPMCRundown = {
    .EventGuid = PROCESS_EVENTS_GUID,
    .Type = 33U,
    .Reserved = { 0U },
};
CLASSIC_EVENT_ID EventProcessDefunct = {
    .EventGuid = PROCESS_EVENTS_GUID,
    .Type = 39U,
    .Reserved = { 0U },
};
