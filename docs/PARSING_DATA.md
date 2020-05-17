# Parsing Data
 - [Event Format](#Event%20Format)
 - [Parsing Data](#Parsing%20Data)


## Event Format
Events are outputted as JSON, for example:
```json
{
    "header": {
        "activity_id": "{00000000-0000-0000-0000-000000000000}",
        "event_flags": 576,
        "event_id": 1,
        "event_name": "",
        "event_opcode": 1,
        "event_version": 3,
        "process_id": 17964,
        "provider_name": "Microsoft-Windows-Kernel-Process",
        "task_name": "ProcessStart",
        "thread_id": 25932,
        "timestamp": "2020-05-17 11:54:24Z"
    },
    "properties": {
        "CreateTime": "2020-05-17 11:54:24Z",
        "Flags": 0,
        "ImageChecksum": 219193,
        "ImageName": "\\Device\\HarddiskVolume4\\Windows\\System32\\notepad.exe",
        "MandatoryLabel": "Mandatory Label\\Medium Mandatory Level",
        "PackageFullName": "",
        "PackageRelativeAppId": "",
        "ParentProcessID": 17964,
        "ParentProcessSequenceNumber": 15206,
        "ProcessID": 25752,
        "ProcessSequenceNumber": 33013,
        "ProcessTokenElevationType": 3,
        "ProcessTokenIsElevated": 0,
        "SessionID": 4,
        "TimeDateStamp": 587902357
    },
    "property_types": {
        "CreateTime": "FILETIME",
        "Flags": "UINT32",
        "ImageChecksum": "UINT32",
        "ImageName": "STRINGW",
        "MandatoryLabel": "SID",
        "PackageFullName": "STRINGW",
        "PackageRelativeAppId": "STRINGW",
        "ParentProcessID": "UINT32",
        "ParentProcessSequenceNumber": "UINT64",
        "ProcessID": "UINT32",
        "ProcessSequenceNumber": "UINT64",
        "ProcessTokenElevationType": "UINT32",
        "ProcessTokenIsElevated": "UINT32",
        "SessionID": "UINT32",
        "TimeDateStamp": "UINT32"
    }
}
```

There are 3 sections to the JSON:
 - [header](#header)
 - [properties](#properties)
 - [property_types](#property_types)

## header
This section is the same for every event, and contains the event metadata.
It will always contains these fields, which are taken from the Event Header:
- activity_id
- event_flags
- event_id
- event_name
- event_opcode
- event_version
- process_id
- provider_name
- task_name
- thread_id
- timestamp

## properties
This is the meat of the event. Unique to every event type, and sometimes even events of the same event ID can have different properties.

Sealighter will attempt to parse every property of every event based upon the property's `TDH_INTYPE`. If the Event does not supply this information, or it's `INTYPE` isn't a simple number, GUID, or string, the property data will be the hex encoded bytes.

## property_types
To assist in writing [filters](FILTERING.md), we also log the `TDH_INTYPE` of each property. This can also help if you do wish to write your own code to parse specific events.


_____________


# Parsing Data

## Parsing Output Files
If the `output_format` was `file`, the events written to the file on a single line, 1 event per line. You can parse this file with Python:
```python
import json
events = list()

with open("output.json", "r") as f:
    for line in f:
        events.append(json.loads(line))
```

## Parsing Event Logs
If the `output_format` was `event_log`, the events are written to the `Sealighter/Operational` Event Log. You can use PowerShell to get and parse these Events:
```powershell
$events = Get-WinEvent -LogName "Sealighter/Operational"
```

The event `.message` contains the full JSON, but you can also access the event's `.Properties` array.
This array has the JSON at index 0, then the rest of the `header` as the remaining properites, i.e.:
```
0. json
1. activity_id
2. event_flags
3. event_id
4. event_name
5. event_opcode
6. event_version
7. process_id
8. provider_name
9. task_name
10. thread_id
11. timestamp
```
For example in PowerShell
```powershell
foreach ($event in $events) {
    $provider_name = $event.Properties[8].Value
    $task_name = $event.Properties[9].Value
    Write-Host "$provider_name - $task_name"
}
```
But you and also just read in the message as JSON:
```powershell
foreach ($event in $events) {
    $event_json = ConvertFrom-Json $event.Message
    $provider_name = $event_json.header.provider_name
    $task_name = $event_json.header.task_name
    Write-Host "$provider_name - $task_name"
}
```


## With Splunk or Kibana
You may also wish to ingest the data into a Splunk or ELK stack.

You can either write to a file, then upload it, or for streaming write to the event log and use a forwarder like WinLogbeat to forward the data.


For super quick experimentations, I quickly setup a temporary Splunk instance in a docker container by:
```bash
docker run --rm --name splunk -d -p 8000:8000 -e 'SPLUNK_START_ARGS=--accept-license' -e 'SPLUNK_PASSWORD=sealighter' splunk/splunk:latest
```
After the container started up, I ran Sealighter writing to a file, then afterwards injested the file and started searching. When I was done for the day, I tore down the container so I never hit the upload limits.