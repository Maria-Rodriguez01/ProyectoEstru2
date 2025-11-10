# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\ProyectoEstru2MariaRodriguez_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\ProyectoEstru2MariaRodriguez_autogen.dir\\ParseCache.txt"
  "ProyectoEstru2MariaRodriguez_autogen"
  )
endif()
