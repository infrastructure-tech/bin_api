# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /mnt/env/local

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /mnt/env/local

# Include any dependencies generated for this target.
include CMakeFiles/api_v1.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/api_v1.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/api_v1.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/api_v1.dir/flags.make

CMakeFiles/api_v1.dir/mnt/env/src/main.cpp.o: CMakeFiles/api_v1.dir/flags.make
CMakeFiles/api_v1.dir/mnt/env/src/main.cpp.o: /mnt/env/src/main.cpp
CMakeFiles/api_v1.dir/mnt/env/src/main.cpp.o: CMakeFiles/api_v1.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/env/local/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/api_v1.dir/mnt/env/src/main.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/api_v1.dir/mnt/env/src/main.cpp.o -MF CMakeFiles/api_v1.dir/mnt/env/src/main.cpp.o.d -o CMakeFiles/api_v1.dir/mnt/env/src/main.cpp.o -c /mnt/env/src/main.cpp

CMakeFiles/api_v1.dir/mnt/env/src/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/api_v1.dir/mnt/env/src/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/env/src/main.cpp > CMakeFiles/api_v1.dir/mnt/env/src/main.cpp.i

CMakeFiles/api_v1.dir/mnt/env/src/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/api_v1.dir/mnt/env/src/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/env/src/main.cpp -o CMakeFiles/api_v1.dir/mnt/env/src/main.cpp.s

# Object files for target api_v1
api_v1_OBJECTS = \
"CMakeFiles/api_v1.dir/mnt/env/src/main.cpp.o"

# External object files for target api_v1
api_v1_EXTERNAL_OBJECTS =

out/api_v1: CMakeFiles/api_v1.dir/mnt/env/src/main.cpp.o
out/api_v1: CMakeFiles/api_v1.dir/build.make
out/api_v1: CMakeFiles/api_v1.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/mnt/env/local/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable out/api_v1"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/api_v1.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/api_v1.dir/build: out/api_v1
.PHONY : CMakeFiles/api_v1.dir/build

CMakeFiles/api_v1.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/api_v1.dir/cmake_clean.cmake
.PHONY : CMakeFiles/api_v1.dir/clean

CMakeFiles/api_v1.dir/depend:
	cd /mnt/env/local && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /mnt/env/local /mnt/env/local /mnt/env/local /mnt/env/local /mnt/env/local/CMakeFiles/api_v1.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/api_v1.dir/depend
