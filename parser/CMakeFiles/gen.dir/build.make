# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

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
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/hieunv/Desktop/popc2.8

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/hieunv/Desktop/popc2.8

# Utility rule file for gen.

# Include the progress variables for this target.
include parser/CMakeFiles/gen.dir/progress.make

parser/CMakeFiles/gen: parser/parser.y
	$(CMAKE_COMMAND) -E cmake_progress_report /home/hieunv/Desktop/popc2.8/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold "Generating parser files"
	cd /home/hieunv/Desktop/popc2.8/parser && /usr/bin/bison -y -b parser -d parser.y
	cd /home/hieunv/Desktop/popc2.8/parser && mv parser.tab.c parser.tab.cc
	cd /home/hieunv/Desktop/popc2.8/parser && /usr/bin/flex -oparser.yy.cc parser.lex

gen: parser/CMakeFiles/gen
gen: parser/CMakeFiles/gen.dir/build.make
.PHONY : gen

# Rule to build all files generated by this target.
parser/CMakeFiles/gen.dir/build: gen
.PHONY : parser/CMakeFiles/gen.dir/build

parser/CMakeFiles/gen.dir/clean:
	cd /home/hieunv/Desktop/popc2.8/parser && $(CMAKE_COMMAND) -P CMakeFiles/gen.dir/cmake_clean.cmake
.PHONY : parser/CMakeFiles/gen.dir/clean

parser/CMakeFiles/gen.dir/depend:
	cd /home/hieunv/Desktop/popc2.8 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/hieunv/Desktop/popc2.8 /home/hieunv/Desktop/popc2.8/parser /home/hieunv/Desktop/popc2.8 /home/hieunv/Desktop/popc2.8/parser /home/hieunv/Desktop/popc2.8/parser/CMakeFiles/gen.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : parser/CMakeFiles/gen.dir/depend

