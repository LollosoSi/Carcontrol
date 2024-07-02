set(project CarControl)

set(CMAKE_VS_INCLUDE_INSTALL_TO_DEFAULT_BUILD 1)

project(${project})

cmake_minimum_required(VERSION 3.12)

# iterate over a list of examples by filename
set(EXAMPLES_LIST
    main
)

add_compile_options(-Ofast -Wall) # passing the compiler a `-pthread` flag doesn't work here


################################################################################
### Find the pigpio shared libraries.
################################################################################

# Find the path to the pigpio includes.
find_path(pigpio_INCLUDE_DIR 
	NAMES pigpio.h pigpiod_if.h pigpiod_if2.h
	HINTS /usr/local/include)
	
# Find the pigpio libraries.
find_library(pigpio_LIBRARY 
	NAMES libpigpio.so
	HINTS /usr/local/lib)
find_library(pigpiod_if_LIBRARY 
	NAMES libpigpiod_if.so
	HINTS /usr/local/lib)
find_library(pigpiod_if2_LIBRARY 
	NAMES libpigpiod_if2.so
	HINTS /usr/local/lib)
    
# Set the pigpio variables to plural form to make them accessible for 
# the paramount cmake modules.
set(pigpio_INCLUDE_DIRS ${pigpio_INCLUDE_DIR})
set(pigpio_INCLUDES     ${pigpio_INCLUDE_DIR})

# Handle REQUIRED, QUIET, and version arguments 
# and set the <packagename>_FOUND variable.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(pigpio 
    DEFAULT_MSG 
    pigpio_INCLUDE_DIR pigpio_LIBRARY pigpiod_if_LIBRARY pigpiod_if2_LIBRARY)


# Aggiungo eseguibile
add_executable(${project} ${sorgenti})
# Seleziona startup project per visual studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${project})

target_sources(${project}
	PRIVATE
		main.cpp
		utils.h
		remote.h
		onboard.h
		sockets.h
		tickableinterface.h
		networkpackets.h
		joystick.h
)

target_link_libraries(${project} PUBLIC ${pigpio_LIBRARY} pthread)