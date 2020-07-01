# Set project name
project (first_project)
# Define target name
set (TARGET_NAME fpbin)

# Set CMake minimum version and CMake policies (required by Urho3D)
cmake_minimum_required (VERSION 3.2.3)
if (COMMAND cmake_policy)
	# Libraries linked via full path no longer produce linker search paths
	cmake_policy (SET CMP0003 NEW)
	# INTERFACE_LINK_LIBRARIES defines the link interface
	cmake_policy (SET CMP0022 NEW)
	# Disallow use of the LOCATION target property - so we set to OLD as we still need it
	cmake_policy (SET CMP0026 OLD)
	# MACOSX RPATH is enabled by default
	cmake_policy (SET CMP0042 NEW)
	# Honor the visibility properties for SHARED target types only
	cmake_policy (SET CMP0063 OLD)
endif ()

# Set CMake modules search path
set (CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake/Modules)

# Include UrhoCommon.cmake module after setting project name
include (UrhoCommon)

# Find project source files
# Note: this is what define_source_files() does, but that points to project root for source.
file (GLOB CPP_FILES source/*.cpp)
file (GLOB H_FILES source/*.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Find assets when structure under ./bin/ differs
#define_resource_dirs ()

# Setup target with resource copying
setup_main_executable ()
