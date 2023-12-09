#!/bin/bash

# Save the project directory to a variable
project_dir="$HOME/ok_web_backend_memgraph"
vcpkg_dir="$project_dir/vcpkg"
build_dir="$project_dir/cmake-build-debug"

# Update package list
sudo apt update

# Install essential build tools
sudo apt install -y build-essential pkg-config autoconf automake git make cmake gdb clang libffi-dev zlib1g-dev libssl-dev gsasl gnutls-bin zip

# Clone the project repository
git clone https://github.com/kapilpipaliya/ok_web_backend_memgraph.git $project_dir

# Navigate to the project directory
cd $project_dir

# Clone vcpkg repository
git clone https://github.com/Microsoft/vcpkg.git  $vcpkg_dir

# Bootstrap vcpkg
sh $vcpkg_dir/bootstrap-vcpkg.sh -disableMetrics

# Navigate to vcpkg directory
cd $vcpkg_dir

# Install required packages using vcpkg
./vcpkg install drogon caf jsoncons date gtest cpp-jwt parallel-hashmap tsl-ordered-map

# Navigate back to the project directory
cd ..

# Copy json_encoder2.hpp to the appropriate directory
cp third_party/one_file_lib/json_encoder2.hpp $vcpkg_dir/installed/x64-linux/include/jsoncons/json_encoder2.hpp

# Create a build directory
mkdir $build_dir

# Navigate to the build directory
cd $build_dir

# Specify the build directory and toolchain file for cmake
cmake -B $build_dir -S . -DCMAKE_TOOLCHAIN_FILE="$vcpkg_dir/scripts/buildsystems/vcpkg.cmake"

# Build the project
cmake --build $build_dir
