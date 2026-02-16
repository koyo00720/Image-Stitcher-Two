# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "CMakeFiles\\Image_Stitch_Two_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\Image_Stitch_Two_autogen.dir\\ParseCache.txt"
  "Image_Stitch_Two_autogen"
  )
endif()
