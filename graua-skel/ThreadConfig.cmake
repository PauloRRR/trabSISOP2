# - Config file for the Thread package
# It defines the following variables
#  THREAD_INCLUDE_DIRS - include directories for Thread
#  THREAD_LIBRARIES    - libraries to link against
#  THREAD_EXECUTABLE   - the test executable
 
# Compute paths
get_filename_component(THREAD_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
set(THREAD_INCLUDE_DIRS "/home/osboxes/Desktop/trabGrauA-SisopII/graua-skel;/home/osboxes/Desktop/trabGrauA-SisopII/graua-skel")
 
# Our library dependencies (contains definitions for IMPORTED targets)
if(NOT TARGET thread AND NOT Thread_BINARY_DIR)
    include("${THREAD_CMAKE_DIR}/ThreadTargets.cmake")
endif()
 
# These are IMPORTED targets created by ThreadTargets.cmake
set(THREAD_LIBRARIES thread)
