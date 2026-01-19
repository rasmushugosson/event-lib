-- Reusable Event project definition
-- Can be included by parent projects via: include("path/to/event-lib/event-project.lua")
-- This will also include the Log project dependency (if not already included)

local event_lib_dir = path.getdirectory(_SCRIPT)
local event_lib_src = event_lib_dir .. "/event-lib"
local log_lib_dir = event_lib_dir .. "/dep/log-lib"

-- Only include log-lib if not already included by parent project
if not LOG_LIB_INCLUDED then
    include(log_lib_dir .. "/log-project.lua")
end

-- Use exported path from log-lib if available, otherwise use local path
local log_include_dir = LOG_LIB_INCLUDE_DIR or (log_lib_dir .. "/log-lib/include")

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
        log_include_dir,
        event_lib_src .. "/include",
        event_lib_src .. "/src"
    })

    links({ "Log" })

    pchheader(path.getabsolute(event_lib_src .. "/src/general/pch.h"))
    pchsource(event_lib_src .. "/src/general/pch.cpp")
