-- Reusable Event project definition
-- Can be included by parent projects via: include("path/to/event-lib/event-project.lua")
-- This will also include the Log project dependency

local event_lib_dir = path.getdirectory(_SCRIPT)
local event_lib_src = event_lib_dir .. "/event-lib"
local log_lib_dir = event_lib_dir .. "/dep/log-lib"

include(log_lib_dir .. "/log-project.lua")

project("Event")
    kind("StaticLib")
    language("C++")
    cppdialect("C++23")
    objdir("obj/%{prj.name}/%{cfg.buildcfg}")
    targetdir("bin/%{prj.name}/%{cfg.buildcfg}")

    files({
        event_lib_src .. "/src/**.cpp",
        event_lib_src .. "/src/**.h",
        event_lib_src .. "/include/**.h"
    })

    includedirs({
        log_lib_dir .. "/log-lib/include",
        event_lib_src .. "/include",
        event_lib_src .. "/src"
    })

    links({ "Log" })

    pchheader(path.getabsolute(event_lib_src .. "/src/general/pch.h"))
    pchsource(event_lib_src .. "/src/general/pch.cpp")
