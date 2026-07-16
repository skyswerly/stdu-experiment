# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "")
  file(REMOVE_RECURSE
  "CMakeFiles\\simulated_os_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\simulated_os_autogen.dir\\ParseCache.txt"
  "simulated_os_autogen"
  )
endif()
