# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.11

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
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
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/zhurish/workspace/working/SWPlatform/source/component/ssh/test/libssh-0.9.5

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/zhurish/workspace/working/SWPlatform/source/component/ssh/test/libssh-0.9.5/build

# Include any dependencies generated for this target.
include examples/CMakeFiles/samplesshd-cb.dir/depend.make

# Include the progress variables for this target.
include examples/CMakeFiles/samplesshd-cb.dir/progress.make

# Include the compile flags for this target's objects.
include examples/CMakeFiles/samplesshd-cb.dir/flags.make

examples/CMakeFiles/samplesshd-cb.dir/samplesshd-cb.c.o: examples/CMakeFiles/samplesshd-cb.dir/flags.make
examples/CMakeFiles/samplesshd-cb.dir/samplesshd-cb.c.o: ../examples/samplesshd-cb.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/zhurish/workspace/working/SWPlatform/source/component/ssh/test/libssh-0.9.5/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object examples/CMakeFiles/samplesshd-cb.dir/samplesshd-cb.c.o"
	cd /home/zhurish/workspace/working/SWPlatform/source/component/ssh/test/libssh-0.9.5/build/examples && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/samplesshd-cb.dir/samplesshd-cb.c.o   -c /home/zhurish/workspace/working/SWPlatform/source/component/ssh/test/libssh-0.9.5/examples/samplesshd-cb.c

examples/CMakeFiles/samplesshd-cb.dir/samplesshd-cb.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/samplesshd-cb.dir/samplesshd-cb.c.i"
	cd /home/zhurish/workspace/working/SWPlatform/source/component/ssh/test/libssh-0.9.5/build/examples && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/zhurish/workspace/working/SWPlatform/source/component/ssh/test/libssh-0.9.5/examples/samplesshd-cb.c > CMakeFiles/samplesshd-cb.dir/samplesshd-cb.c.i

examples/CMakeFiles/samplesshd-cb.dir/samplesshd-cb.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/samplesshd-cb.dir/samplesshd-cb.c.s"
	cd /home/zhurish/workspace/working/SWPlatform/source/component/ssh/test/libssh-0.9.5/build/examples && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/zhurish/workspace/working/SWPlatform/source/component/ssh/test/libssh-0.9.5/examples/samplesshd-cb.c -o CMakeFiles/samplesshd-cb.dir/samplesshd-cb.c.s

# Object files for target samplesshd-cb
samplesshd__cb_OBJECTS = \
"CMakeFiles/samplesshd-cb.dir/samplesshd-cb.c.o"

# External object files for target samplesshd-cb
samplesshd__cb_EXTERNAL_OBJECTS =

examples/samplesshd-cb: examples/CMakeFiles/samplesshd-cb.dir/samplesshd-cb.c.o
examples/samplesshd-cb: examples/CMakeFiles/samplesshd-cb.dir/build.make
examples/samplesshd-cb: lib/libssh.so.4.8.6
examples/samplesshd-cb: examples/CMakeFiles/samplesshd-cb.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/zhurish/workspace/working/SWPlatform/source/component/ssh/test/libssh-0.9.5/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable samplesshd-cb"
	cd /home/zhurish/workspace/working/SWPlatform/source/component/ssh/test/libssh-0.9.5/build/examples && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/samplesshd-cb.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
examples/CMakeFiles/samplesshd-cb.dir/build: examples/samplesshd-cb

.PHONY : examples/CMakeFiles/samplesshd-cb.dir/build

examples/CMakeFiles/samplesshd-cb.dir/clean:
	cd /home/zhurish/workspace/working/SWPlatform/source/component/ssh/test/libssh-0.9.5/build/examples && $(CMAKE_COMMAND) -P CMakeFiles/samplesshd-cb.dir/cmake_clean.cmake
.PHONY : examples/CMakeFiles/samplesshd-cb.dir/clean

examples/CMakeFiles/samplesshd-cb.dir/depend:
	cd /home/zhurish/workspace/working/SWPlatform/source/component/ssh/test/libssh-0.9.5/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/zhurish/workspace/working/SWPlatform/source/component/ssh/test/libssh-0.9.5 /home/zhurish/workspace/working/SWPlatform/source/component/ssh/test/libssh-0.9.5/examples /home/zhurish/workspace/working/SWPlatform/source/component/ssh/test/libssh-0.9.5/build /home/zhurish/workspace/working/SWPlatform/source/component/ssh/test/libssh-0.9.5/build/examples /home/zhurish/workspace/working/SWPlatform/source/component/ssh/test/libssh-0.9.5/build/examples/CMakeFiles/samplesshd-cb.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : examples/CMakeFiles/samplesshd-cb.dir/depend
