# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.29

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
CMAKE_COMMAND = /snap/clion/296/bin/cmake/linux/x64/bin/cmake

# The command to remove a file.
RM = /snap/clion/296/bin/cmake/linux/x64/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/robbelehaen/OperatingSystemsGT/clion/lab3

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/robbelehaen/OperatingSystemsGT/clion/lab3/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/lab3.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/lab3.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/lab3.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/lab3.dir/flags.make

CMakeFiles/lab3.dir/dplist.c.o: CMakeFiles/lab3.dir/flags.make
CMakeFiles/lab3.dir/dplist.c.o: /home/robbelehaen/OperatingSystemsGT/clion/lab3/dplist.c
CMakeFiles/lab3.dir/dplist.c.o: CMakeFiles/lab3.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/robbelehaen/OperatingSystemsGT/clion/lab3/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/lab3.dir/dplist.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/lab3.dir/dplist.c.o -MF CMakeFiles/lab3.dir/dplist.c.o.d -o CMakeFiles/lab3.dir/dplist.c.o -c /home/robbelehaen/OperatingSystemsGT/clion/lab3/dplist.c

CMakeFiles/lab3.dir/dplist.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/lab3.dir/dplist.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/robbelehaen/OperatingSystemsGT/clion/lab3/dplist.c > CMakeFiles/lab3.dir/dplist.c.i

CMakeFiles/lab3.dir/dplist.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/lab3.dir/dplist.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/robbelehaen/OperatingSystemsGT/clion/lab3/dplist.c -o CMakeFiles/lab3.dir/dplist.c.s

CMakeFiles/lab3.dir/ourtest.c.o: CMakeFiles/lab3.dir/flags.make
CMakeFiles/lab3.dir/ourtest.c.o: /home/robbelehaen/OperatingSystemsGT/clion/lab3/ourtest.c
CMakeFiles/lab3.dir/ourtest.c.o: CMakeFiles/lab3.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/robbelehaen/OperatingSystemsGT/clion/lab3/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/lab3.dir/ourtest.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/lab3.dir/ourtest.c.o -MF CMakeFiles/lab3.dir/ourtest.c.o.d -o CMakeFiles/lab3.dir/ourtest.c.o -c /home/robbelehaen/OperatingSystemsGT/clion/lab3/ourtest.c

CMakeFiles/lab3.dir/ourtest.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/lab3.dir/ourtest.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/robbelehaen/OperatingSystemsGT/clion/lab3/ourtest.c > CMakeFiles/lab3.dir/ourtest.c.i

CMakeFiles/lab3.dir/ourtest.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/lab3.dir/ourtest.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/robbelehaen/OperatingSystemsGT/clion/lab3/ourtest.c -o CMakeFiles/lab3.dir/ourtest.c.s

CMakeFiles/lab3.dir/dplist_test.c.o: CMakeFiles/lab3.dir/flags.make
CMakeFiles/lab3.dir/dplist_test.c.o: /home/robbelehaen/OperatingSystemsGT/clion/lab3/dplist_test.c
CMakeFiles/lab3.dir/dplist_test.c.o: CMakeFiles/lab3.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/robbelehaen/OperatingSystemsGT/clion/lab3/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object CMakeFiles/lab3.dir/dplist_test.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/lab3.dir/dplist_test.c.o -MF CMakeFiles/lab3.dir/dplist_test.c.o.d -o CMakeFiles/lab3.dir/dplist_test.c.o -c /home/robbelehaen/OperatingSystemsGT/clion/lab3/dplist_test.c

CMakeFiles/lab3.dir/dplist_test.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/lab3.dir/dplist_test.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/robbelehaen/OperatingSystemsGT/clion/lab3/dplist_test.c > CMakeFiles/lab3.dir/dplist_test.c.i

CMakeFiles/lab3.dir/dplist_test.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/lab3.dir/dplist_test.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/robbelehaen/OperatingSystemsGT/clion/lab3/dplist_test.c -o CMakeFiles/lab3.dir/dplist_test.c.s

CMakeFiles/lab3.dir/randomtest.c.o: CMakeFiles/lab3.dir/flags.make
CMakeFiles/lab3.dir/randomtest.c.o: /home/robbelehaen/OperatingSystemsGT/clion/lab3/randomtest.c
CMakeFiles/lab3.dir/randomtest.c.o: CMakeFiles/lab3.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/robbelehaen/OperatingSystemsGT/clion/lab3/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building C object CMakeFiles/lab3.dir/randomtest.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/lab3.dir/randomtest.c.o -MF CMakeFiles/lab3.dir/randomtest.c.o.d -o CMakeFiles/lab3.dir/randomtest.c.o -c /home/robbelehaen/OperatingSystemsGT/clion/lab3/randomtest.c

CMakeFiles/lab3.dir/randomtest.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/lab3.dir/randomtest.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/robbelehaen/OperatingSystemsGT/clion/lab3/randomtest.c > CMakeFiles/lab3.dir/randomtest.c.i

CMakeFiles/lab3.dir/randomtest.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/lab3.dir/randomtest.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/robbelehaen/OperatingSystemsGT/clion/lab3/randomtest.c -o CMakeFiles/lab3.dir/randomtest.c.s

# Object files for target lab3
lab3_OBJECTS = \
"CMakeFiles/lab3.dir/dplist.c.o" \
"CMakeFiles/lab3.dir/ourtest.c.o" \
"CMakeFiles/lab3.dir/dplist_test.c.o" \
"CMakeFiles/lab3.dir/randomtest.c.o"

# External object files for target lab3
lab3_EXTERNAL_OBJECTS =

lab3: CMakeFiles/lab3.dir/dplist.c.o
lab3: CMakeFiles/lab3.dir/ourtest.c.o
lab3: CMakeFiles/lab3.dir/dplist_test.c.o
lab3: CMakeFiles/lab3.dir/randomtest.c.o
lab3: CMakeFiles/lab3.dir/build.make
lab3: CMakeFiles/lab3.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/robbelehaen/OperatingSystemsGT/clion/lab3/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Linking C executable lab3"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/lab3.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/lab3.dir/build: lab3
.PHONY : CMakeFiles/lab3.dir/build

CMakeFiles/lab3.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/lab3.dir/cmake_clean.cmake
.PHONY : CMakeFiles/lab3.dir/clean

CMakeFiles/lab3.dir/depend:
	cd /home/robbelehaen/OperatingSystemsGT/clion/lab3/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/robbelehaen/OperatingSystemsGT/clion/lab3 /home/robbelehaen/OperatingSystemsGT/clion/lab3 /home/robbelehaen/OperatingSystemsGT/clion/lab3/cmake-build-debug /home/robbelehaen/OperatingSystemsGT/clion/lab3/cmake-build-debug /home/robbelehaen/OperatingSystemsGT/clion/lab3/cmake-build-debug/CMakeFiles/lab3.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/lab3.dir/depend

