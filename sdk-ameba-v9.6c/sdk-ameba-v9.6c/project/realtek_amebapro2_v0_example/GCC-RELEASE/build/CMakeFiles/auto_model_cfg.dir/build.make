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
CMAKE_SOURCE_DIR = /home/hdj/amb82-mini-RTL8735B/sdk-ameba-v9.6c-hard-spi/sdk-ameba-v9.6c/project/realtek_amebapro2_v0_example/GCC-RELEASE

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/hdj/amb82-mini-RTL8735B/sdk-ameba-v9.6c-hard-spi/sdk-ameba-v9.6c/project/realtek_amebapro2_v0_example/GCC-RELEASE/build

# Utility rule file for auto_model_cfg.

# Include any custom commands dependencies for this target.
include CMakeFiles/auto_model_cfg.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/auto_model_cfg.dir/progress.make

CMakeFiles/auto_model_cfg: ../mp/amebapro2_fwfs_nn_models.json
	/usr/bin/cmake -E cmake_echo_color --cyan skip\ model\ config
	/usr/bin/cmake -E copy /home/hdj/amb82-mini-RTL8735B/sdk-ameba-v9.6c-hard-spi/sdk-ameba-v9.6c/project/realtek_amebapro2_v0_example/GCC-RELEASE/../GCC-RELEASE/mp/amebapro2_fwfs_nn_models.json amebapro2_fwfs_nn_models.json

auto_model_cfg: CMakeFiles/auto_model_cfg
auto_model_cfg: CMakeFiles/auto_model_cfg.dir/build.make
.PHONY : auto_model_cfg

# Rule to build all files generated by this target.
CMakeFiles/auto_model_cfg.dir/build: auto_model_cfg
.PHONY : CMakeFiles/auto_model_cfg.dir/build

CMakeFiles/auto_model_cfg.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/auto_model_cfg.dir/cmake_clean.cmake
.PHONY : CMakeFiles/auto_model_cfg.dir/clean

CMakeFiles/auto_model_cfg.dir/depend:
	cd /home/hdj/amb82-mini-RTL8735B/sdk-ameba-v9.6c-hard-spi/sdk-ameba-v9.6c/project/realtek_amebapro2_v0_example/GCC-RELEASE/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/hdj/amb82-mini-RTL8735B/sdk-ameba-v9.6c-hard-spi/sdk-ameba-v9.6c/project/realtek_amebapro2_v0_example/GCC-RELEASE /home/hdj/amb82-mini-RTL8735B/sdk-ameba-v9.6c-hard-spi/sdk-ameba-v9.6c/project/realtek_amebapro2_v0_example/GCC-RELEASE /home/hdj/amb82-mini-RTL8735B/sdk-ameba-v9.6c-hard-spi/sdk-ameba-v9.6c/project/realtek_amebapro2_v0_example/GCC-RELEASE/build /home/hdj/amb82-mini-RTL8735B/sdk-ameba-v9.6c-hard-spi/sdk-ameba-v9.6c/project/realtek_amebapro2_v0_example/GCC-RELEASE/build /home/hdj/amb82-mini-RTL8735B/sdk-ameba-v9.6c-hard-spi/sdk-ameba-v9.6c/project/realtek_amebapro2_v0_example/GCC-RELEASE/build/CMakeFiles/auto_model_cfg.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/auto_model_cfg.dir/depend

