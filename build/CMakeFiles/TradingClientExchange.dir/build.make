# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.28

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
CMAKE_COMMAND = /Users/kennethzhang/opt/miniconda3/lib/python3.9/site-packages/cmake/data/bin/cmake

# The command to remove a file.
RM = /Users/kennethzhang/opt/miniconda3/lib/python3.9/site-packages/cmake/data/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/kennethzhang/Desktop/TradingClientExchange

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/kennethzhang/Desktop/TradingClientExchange/build

# Include any dependencies generated for this target.
include CMakeFiles/TradingClientExchange.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/TradingClientExchange.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/TradingClientExchange.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/TradingClientExchange.dir/flags.make

CMakeFiles/TradingClientExchange.dir/TradingClientExchange_autogen/mocs_compilation.cpp.o: CMakeFiles/TradingClientExchange.dir/flags.make
CMakeFiles/TradingClientExchange.dir/TradingClientExchange_autogen/mocs_compilation.cpp.o: TradingClientExchange_autogen/mocs_compilation.cpp
CMakeFiles/TradingClientExchange.dir/TradingClientExchange_autogen/mocs_compilation.cpp.o: CMakeFiles/TradingClientExchange.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/kennethzhang/Desktop/TradingClientExchange/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/TradingClientExchange.dir/TradingClientExchange_autogen/mocs_compilation.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/TradingClientExchange.dir/TradingClientExchange_autogen/mocs_compilation.cpp.o -MF CMakeFiles/TradingClientExchange.dir/TradingClientExchange_autogen/mocs_compilation.cpp.o.d -o CMakeFiles/TradingClientExchange.dir/TradingClientExchange_autogen/mocs_compilation.cpp.o -c /Users/kennethzhang/Desktop/TradingClientExchange/build/TradingClientExchange_autogen/mocs_compilation.cpp

CMakeFiles/TradingClientExchange.dir/TradingClientExchange_autogen/mocs_compilation.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/TradingClientExchange.dir/TradingClientExchange_autogen/mocs_compilation.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/kennethzhang/Desktop/TradingClientExchange/build/TradingClientExchange_autogen/mocs_compilation.cpp > CMakeFiles/TradingClientExchange.dir/TradingClientExchange_autogen/mocs_compilation.cpp.i

CMakeFiles/TradingClientExchange.dir/TradingClientExchange_autogen/mocs_compilation.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/TradingClientExchange.dir/TradingClientExchange_autogen/mocs_compilation.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/kennethzhang/Desktop/TradingClientExchange/build/TradingClientExchange_autogen/mocs_compilation.cpp -o CMakeFiles/TradingClientExchange.dir/TradingClientExchange_autogen/mocs_compilation.cpp.s

CMakeFiles/TradingClientExchange.dir/src/main.cpp.o: CMakeFiles/TradingClientExchange.dir/flags.make
CMakeFiles/TradingClientExchange.dir/src/main.cpp.o: /Users/kennethzhang/Desktop/TradingClientExchange/src/main.cpp
CMakeFiles/TradingClientExchange.dir/src/main.cpp.o: CMakeFiles/TradingClientExchange.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/kennethzhang/Desktop/TradingClientExchange/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/TradingClientExchange.dir/src/main.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/TradingClientExchange.dir/src/main.cpp.o -MF CMakeFiles/TradingClientExchange.dir/src/main.cpp.o.d -o CMakeFiles/TradingClientExchange.dir/src/main.cpp.o -c /Users/kennethzhang/Desktop/TradingClientExchange/src/main.cpp

CMakeFiles/TradingClientExchange.dir/src/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/TradingClientExchange.dir/src/main.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/kennethzhang/Desktop/TradingClientExchange/src/main.cpp > CMakeFiles/TradingClientExchange.dir/src/main.cpp.i

CMakeFiles/TradingClientExchange.dir/src/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/TradingClientExchange.dir/src/main.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/kennethzhang/Desktop/TradingClientExchange/src/main.cpp -o CMakeFiles/TradingClientExchange.dir/src/main.cpp.s

# Object files for target TradingClientExchange
TradingClientExchange_OBJECTS = \
"CMakeFiles/TradingClientExchange.dir/TradingClientExchange_autogen/mocs_compilation.cpp.o" \
"CMakeFiles/TradingClientExchange.dir/src/main.cpp.o"

# External object files for target TradingClientExchange
TradingClientExchange_EXTERNAL_OBJECTS =

TradingClientExchange: CMakeFiles/TradingClientExchange.dir/TradingClientExchange_autogen/mocs_compilation.cpp.o
TradingClientExchange: CMakeFiles/TradingClientExchange.dir/src/main.cpp.o
TradingClientExchange: CMakeFiles/TradingClientExchange.dir/build.make
TradingClientExchange: libtce_core.dylib
TradingClientExchange: /opt/homebrew/lib/QtWidgets.framework/Versions/A/QtWidgets
TradingClientExchange: /opt/homebrew/lib/QtGui.framework/Versions/A/QtGui
TradingClientExchange: /opt/homebrew/lib/QtCore.framework/Versions/A/QtCore
TradingClientExchange: CMakeFiles/TradingClientExchange.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/Users/kennethzhang/Desktop/TradingClientExchange/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable TradingClientExchange"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/TradingClientExchange.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/TradingClientExchange.dir/build: TradingClientExchange
.PHONY : CMakeFiles/TradingClientExchange.dir/build

CMakeFiles/TradingClientExchange.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/TradingClientExchange.dir/cmake_clean.cmake
.PHONY : CMakeFiles/TradingClientExchange.dir/clean

CMakeFiles/TradingClientExchange.dir/depend:
	cd /Users/kennethzhang/Desktop/TradingClientExchange/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/kennethzhang/Desktop/TradingClientExchange /Users/kennethzhang/Desktop/TradingClientExchange /Users/kennethzhang/Desktop/TradingClientExchange/build /Users/kennethzhang/Desktop/TradingClientExchange/build /Users/kennethzhang/Desktop/TradingClientExchange/build/CMakeFiles/TradingClientExchange.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/TradingClientExchange.dir/depend

