# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.23

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
CMAKE_COMMAND = /home/melancholycy/SoftWare/clion-2022.2/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /home/melancholycy/SoftWare/clion-2022.2/bin/cmake/linux/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/melancholycy/myCode/SCU_DB/project2

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/melancholycy/myCode/SCU_DB/project2/cmake-build-debug

# Include any dependencies generated for this target.
include test/CMakeFiles/b_plus_tree_test.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include test/CMakeFiles/b_plus_tree_test.dir/compiler_depend.make

# Include the progress variables for this target.
include test/CMakeFiles/b_plus_tree_test.dir/progress.make

# Include the compile flags for this target's objects.
include test/CMakeFiles/b_plus_tree_test.dir/flags.make

test/CMakeFiles/b_plus_tree_test.dir/index/b_plus_tree_test.cpp.o: test/CMakeFiles/b_plus_tree_test.dir/flags.make
test/CMakeFiles/b_plus_tree_test.dir/index/b_plus_tree_test.cpp.o: ../test/index/b_plus_tree_test.cpp
test/CMakeFiles/b_plus_tree_test.dir/index/b_plus_tree_test.cpp.o: test/CMakeFiles/b_plus_tree_test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/melancholycy/myCode/SCU_DB/project2/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object test/CMakeFiles/b_plus_tree_test.dir/index/b_plus_tree_test.cpp.o"
	cd /home/melancholycy/myCode/SCU_DB/project2/cmake-build-debug/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT test/CMakeFiles/b_plus_tree_test.dir/index/b_plus_tree_test.cpp.o -MF CMakeFiles/b_plus_tree_test.dir/index/b_plus_tree_test.cpp.o.d -o CMakeFiles/b_plus_tree_test.dir/index/b_plus_tree_test.cpp.o -c /home/melancholycy/myCode/SCU_DB/project2/test/index/b_plus_tree_test.cpp

test/CMakeFiles/b_plus_tree_test.dir/index/b_plus_tree_test.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/b_plus_tree_test.dir/index/b_plus_tree_test.cpp.i"
	cd /home/melancholycy/myCode/SCU_DB/project2/cmake-build-debug/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/melancholycy/myCode/SCU_DB/project2/test/index/b_plus_tree_test.cpp > CMakeFiles/b_plus_tree_test.dir/index/b_plus_tree_test.cpp.i

test/CMakeFiles/b_plus_tree_test.dir/index/b_plus_tree_test.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/b_plus_tree_test.dir/index/b_plus_tree_test.cpp.s"
	cd /home/melancholycy/myCode/SCU_DB/project2/cmake-build-debug/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/melancholycy/myCode/SCU_DB/project2/test/index/b_plus_tree_test.cpp -o CMakeFiles/b_plus_tree_test.dir/index/b_plus_tree_test.cpp.s

# Object files for target b_plus_tree_test
b_plus_tree_test_OBJECTS = \
"CMakeFiles/b_plus_tree_test.dir/index/b_plus_tree_test.cpp.o"

# External object files for target b_plus_tree_test
b_plus_tree_test_EXTERNAL_OBJECTS =

test/b_plus_tree_test: test/CMakeFiles/b_plus_tree_test.dir/index/b_plus_tree_test.cpp.o
test/b_plus_tree_test: test/CMakeFiles/b_plus_tree_test.dir/build.make
test/b_plus_tree_test: lib/libvtable.so
test/b_plus_tree_test: lib/libsqlite3.so
test/b_plus_tree_test: lib/libgtest.so
test/b_plus_tree_test: test/CMakeFiles/b_plus_tree_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/melancholycy/myCode/SCU_DB/project2/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable b_plus_tree_test"
	cd /home/melancholycy/myCode/SCU_DB/project2/cmake-build-debug/test && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/b_plus_tree_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/CMakeFiles/b_plus_tree_test.dir/build: test/b_plus_tree_test
.PHONY : test/CMakeFiles/b_plus_tree_test.dir/build

test/CMakeFiles/b_plus_tree_test.dir/clean:
	cd /home/melancholycy/myCode/SCU_DB/project2/cmake-build-debug/test && $(CMAKE_COMMAND) -P CMakeFiles/b_plus_tree_test.dir/cmake_clean.cmake
.PHONY : test/CMakeFiles/b_plus_tree_test.dir/clean

test/CMakeFiles/b_plus_tree_test.dir/depend:
	cd /home/melancholycy/myCode/SCU_DB/project2/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/melancholycy/myCode/SCU_DB/project2 /home/melancholycy/myCode/SCU_DB/project2/test /home/melancholycy/myCode/SCU_DB/project2/cmake-build-debug /home/melancholycy/myCode/SCU_DB/project2/cmake-build-debug/test /home/melancholycy/myCode/SCU_DB/project2/cmake-build-debug/test/CMakeFiles/b_plus_tree_test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/CMakeFiles/b_plus_tree_test.dir/depend

