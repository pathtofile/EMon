# Configuration File

The Sealighter config file is how you specify what events from what providers to log, how to log them, and other ETW session properties.

The file is in JSON. An example config file looks like this:
```json
{
    "session_properties": {
        "session_name": "My-Process-Trace",
        "output_format": "stdout",
        "buffering_timout_seconds":  10
    },
    "user_traces": [
        {
            "trace_name": "proc_trace",
            "provider_name": "Microsoft-Windows-Kernel-Process",
            "keywords_any": 16
        },
        {
            "trace_name": "guid_trace",
            "provider_name": "{382b5e24-181e-417f-a8d6-2155f749e724}",
            "filters": {
                "any_of": {
                    "opcode_is": [1, 2]
                }
            },
            "buffers": [
                {
                    "event_id": 1,
                    "max_before_buffering": 1,
                    "fields": [
                        "ImageName"
                    ]
                }
            ]
        },
    ],
    "kernel_traces": [
        {
            "trace_name": "kernel_proc_trace",
            "provider_name": "process",
        }
    ]
}
```

Config Files have 3 Parts:
 - [session_properties](#session_properties)
 - [user_traces](#user_traces)
 - [kernel_traces](#kernel_traces)

_____________

# session_properties
These are where you specify properties of the ETW Session, e.g:
```json
"session_properties": {
    "session_name": "My-Trace",
    "output_format": "stdout",
    "output_filename": "path/to/output.json",
},
```
You can specify the following options:

### session_name
The name of the ETW Session.
Default: Sealighter

### output_format
Where to output the events to. Can be one of:
 - stdout
 - event_log
 - file

If specifying file, also specify `output_filename`:
```json
"session_properties": {
    "output_format": "stdout",
    "output_filename": "path/to/output.json",
},
```

### output_filename
If outputting to a file, the path to write the output events to.

The following are advanced session properties:
### buffer_size
The Size of the in-memory buffer.
Default: 256

### minimum_buffers
Minimum Buffers to allocate. Default 12

### maximum_buffers
Max Buffers to allocate. Default 48

### flush_timer
Buffer Flush timer in seconds. Default 1

### buffering_timout_seconds
If using [Buffering](BUFFERING.md), this specifies how often to flush
the events, reporting on a group of events as one with a `buffered_count`.
If using buffering, default is 30 seconds.

_____________

# user_traces
This is an array of the User mode or WPP providers you want to subscribe to, e.g.:
```json
"user_traces": [
    {
        "trace_name": "proc_trace",
        "provider_name": "Microsoft-Windows-Kernel-Process",
        "keywords_any": 16
    },
    {
        "trace_name": "guid_trace",
        "provider_name": "{382b5e24-181e-417f-a8d6-2155f749e724}",
        "report_stacktrace": true,
        "filters": {
            "any_of": {
                "opcode_is": [1, 2]
            }
        }
    },
]
```

User Providers have the following options, all are optional except for `trace_name` and `provider_name`:

### trace_name
Unique Name to give this provider, that will appear in the reported events.
If running multiple `user_traces` that use the same provider, this will tell you which set of
filters the event hit on.

### provider_name
The name or GUID of the Provider to enable.
For WPP Traces, this *must* be the GUID.

### keywords_any
Only report on Events that has these at least some of these keyword flags. See [Scenarios](SCENARIOS.md) for examples on finding information on a provider's keywords.

Whilst you can also use filters to filter based on keywords (filters explainer later), the `keywords_any` filtering happens in the Kernel, instead of in user land inside Sealighter, and is therefore much more efficient to filter.

It is advices to `keywords_any` as much as possible to ensure you don't drop any events.

### keywords_all
Similar to `keywords_any`, but an event must match all the keywords.

If neither `keywords_any` or `keywords_all` is specified, all events will be passed onto the filters to be reported on.

`keywords_any` and `keywords_all` take precedence of filters.


### Level
Only report if events are at least this logging level.
Like the `keywords_*` options, it is more efficient use this instead of a Filter, and this will take precedence of a Filter

### trace_flags
Any advanced flags

### report_stacktrace
If set to `true`, events will also include a stack trace array of the memory addresses
of functions that generated the event.

### filters
An array of filters to further filter the events to report on. These can be quite complex, so read the [Filtering](FILTERING.md) section for details.

### buffers
Buffering enables the reporting of many similar events in a time period as one with a count.
For details, read [Buffering](BUFFERING.md).

_____________

# kernel_traces

This is an array of the special sub-providers of the Special `NT Kernel Trace` that you wish to log, e.g.:
```json
"kernel_traces": [
    {
        "trace_name": "kernel_proc_trace",
        "provider_name": "process",
        "filters": {
            "any_of": {
                "opcode_is": [1, 2]
            }
        }
    },
    {
        "trace_name": "kernel_image_trace",
        "provider_name": "image_load",
    }
]
```
Kernel Providers have three options, all are required:

### trace_name
Unique Name to give this provider, that will appear in header of reported events.
If running multiple `user_traces` that use the same provider, this will tell you which set of
filters the event hit on.

### provider_name
The kernel provider to log. Must be one of:
- process
- thread
- image_load
- process_counter
- context_switch
- dpc
- interrupt
- system_call
- disk_io
- disk_file_io
- disk_init_io
- thread_dispatch
- memory_page_fault
- memory_hard_fault
- virtual_alloc
- network_tcpip
- registry
- alpc
- split_io
- driver
- profile
- file_io
- file_init_io
- debug_print
- vamap_provider
- object_manager


### filters
Like `user_traces`, this is a list of filters to filter the events to report on. These can be quite complex, so read the [Filtering](FILTERING.md) section for details.

### buffers
Like `user_traces`, buffering enables the reporting of many similar events in a time period as one with a count.
For details, read [Buffering](BUFFERING.md).
