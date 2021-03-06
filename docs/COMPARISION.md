# Comparison to SilkETW

[SilkETW](https://github.com/fireeye/SilkETW) is another Open Source tool designed to help understand the power of ETW and the data it can produce.

I tested it out when FuzzySec first released it, but came across a couple of issue that mean it didn't replace my personal tooling as a research platform.

I really want to stress, my goal and primary use case of Sealighter is as a *research* platform. SilkETW has some nice features to make it more suitable in a production environment. But for a platform to research new ETW, TraceLogging, and WPP providers and events, The following were challenges that led me to turn my tooling into a proper project:


## Production Vs Research
Sealighter is designed to help researchers understand and dig into ETW.
I do not advise anyone using it in a production environment to protect their network, mostly due to it being a single person growing/maintaing this
project. Use the ideas you learn from using Sealighter to enhanse your own tools or situational awareness.


## Parsing of some Events
Under the hood, Silk uses the .NET library [Microsoft.Diagnostics.Tracing.TraceEvent](https://github.com/microsoft/perfview/blob/master/documentation/TraceEvent/TraceEventLibrary.md) for its base ETW handling.
In the past, I've had issues with this library not fully parsing events, reporting only a fraction of the event's properties. An example of this was the `Event(1001)` in the `Microsoft-Windows-WFP` Provider.

Additionally, this library is only .NET, and only provides basic ETW Session controlling, meaning FuzzySec had to implement his own filtering logic.

Comparatively, Sealighter is built on the [KrabsETW](https://github.com/microsoft/krabsetw) Library. This library has both a C++ native library (with Sealighter uses), and a .NET wrapper. Additionally, it provides a lot of extremely useful and efficient event filtering options, which makes up the bulk of Sealighter's filtering (i.e. I didn't have to write them, just fix a couple of bugs in them, which I got merged back into Krabs).


## Filtering
ETW can be extremely verbose, making very difficult to find useful data in sea of events.
SilkETW, provides a few basic ways to filter events, alongside a powerful Yara rule matching engine.
The Yara rule matching is super powerful - run against the JSON serialised event, you can write any filter to match any part of the event. For Sealighter I decided this was not the right approach for two main reasons:


### 1. Performance
Silk has to first received the event, parse it into JSON, *then* run the Yara Scanning over it. This comes at a performance cost, as you are spending time parsing and converting events to JSON only to drop them.

Sealighter instead filters events as early as possible using a static list of filters you can read about [here](FILTERING.md), before converting them to JSON. This ensures much higher performance, particularly for high-volume providers. Additionally, Yara scanning the entire event it much less efficient compared to searching just the event property you want to filter.

### 2. Separating filters and config
Silk separates the Yara filters from the config, reading them in from a separate file/folder. I'm not a fan of this, and prefer to more tightly couple the Provider of interest, and the filter I am running of it, in a single place inside the config.


## Performance and resource usage
Silk is written in .NET, whereas Sealighter is in native C++.
For some high-volume providers, I have found a difference in memory and CPU usage, with the lower-level C++ performing better, mostly due to the reduced amount of memory allocating it does before filtering events.


## Opcode parsing wrong
This is an issue that is easily fixed, and I only realised as I was writing this I forgot to raise an issue on SilkETW's tracker, despite hitting it when I first checked it out, so I've made sure to [add it](https://github.com/fireeye/SilkETW/issues/13).

In Silk, when creating a filter by Opcode, Silk manually checks this Opcode is between 0-9.
However, Opcodes are a UCHAR, so they can actually be up to 255.


## Stack Traces
SilkETW doesn't enable reporting of Event Stack Traces, a very useful set of data to get in some circumstances.
Sealighter has the ability to do this for user mode providers by enabling the `report_stacktrace` option on a
per-provider basis

## Buffering
ETW can produce lots and lots of events. I added the ability to buffer many similar events in time period into
one event with a count.
