#!/bin/bash

# Save the project directory to a variable
project_dir="$HOME/ok_web_backend_memgraph"
vcpkg_dir="$project_dir/vcpkg"
build_dir="$project_dir/cmake-build-debug"

# Update package list
sudo apt update

# Install essential build tools and dependencies
sudo apt install -y build-essential pkg-config autoconf automake git make cmake gdb clang libffi-dev zlib1g-dev libssl-dev gsasl gnutls-bin zip

# Clone or pull the project repository
if [ -d "$project_dir" ]; then
    # If the project directory exists, pull the latest changes
    echo "Updating existing project..."
    cd $project_dir
    git pull
else
    # If the project directory doesn't exist, clone the repository
    echo "Cloning new project..."
    git clone https://github.com/kapilpipaliya/ok_web_backend_memgraph.git $project_dir
fi


# Navigate to the project directory
cd $project_dir

# Clone or pull the vcpkg repository
if [ -d "$vcpkg_dir" ]; then
    # If the vcpkg directory exists, pull the latest changes
    echo "Updating existing vcpkg..."
    cd $vcpkg_dir
    git pull
else
    # If the vcpkg directory doesn't exist, clone the repository
    echo "Cloning new vcpkg..."
    git clone https://github.com/Microsoft/vcpkg.git $vcpkg_dir
fi

# Bootstrap vcpkg
sh $vcpkg_dir/bootstrap-vcpkg.sh -disableMetrics

# Navigate to vcpkg directory
cd $vcpkg_dir

# Install required packages using vcpkg
./vcpkg install drogon caf jsoncons date gtest cpp-jwt parallel-hashmap tsl-ordered-map

# Navigate back to the project directory
cd $project_dir

# Copy json_encoder2.hpp to the appropriate directory
cp $project_dir/third_party/one_file_lib/json_encoder2.hpp $vcpkg_dir/installed/x64-linux/include/jsoncons/json_encoder2.hpp

# Create a build directory
mkdir -p $build_dir

# Navigate to the build directory (commented out since not necessary)
#cd $build_dir

# Specify the build directory and toolchain file for cmake
cmake -B $build_dir -S . -DCMAKE_TOOLCHAIN_FILE="$vcpkg_dir/scripts/buildsystems/vcpkg.cmake"

# Build the project
cmake --build $build_dir


# Compile the server_agent project

# Save the project directory to a variable
server_agent_project_dir="$HOME/server_agent"
server_agent_build_dir="$server_agent_project_dir/cmake-build-debug"

# Clone or pull the server_agent project repository
if [ -d "$server_agent_project_dir" ]; then
    # If the server_agent project directory exists, pull the latest changes
    echo "Updating existing server_agent project..."
    cd $server_agent_project_dir
    git pull
else
    # If the server_agent project directory doesn't exist, clone the repository
    echo "Cloning new server_agent project..."
    git clone https://github.com/kapilpipaliya/server_agent.git $server_agent_project_dir
fi

# Navigate to the server_agent project directory
cd $server_agent_project_dir

# Create a build directory for server_agent
mkdir -p $server_agent_build_dir

# Specify the build directory and toolchain file for cmake
cmake -B $server_agent_build_dir -S . -DCMAKE_TOOLCHAIN_FILE="$vcpkg_dir/scripts/buildsystems/vcpkg.cmake"

# Build the server_agent project
cmake --build $server_agent_build_dir
