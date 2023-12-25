cmake_minimum_required(VERSION 3.5)

#https://vcpkg.io/en/docs/users/integration.html#using-an-environment-variable-instead-of-a-command-line-option
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()
enable_language(C)
project(ok_web_backend CXX)
set(CMAKE_CXX_STANDARD 20)

include(CheckIncludeFileCXX)

check_include_file_cxx(any HAS_ANY)
check_include_file_cxx(string_view HAS_STRING_VIEW)
check_include_file_cxx(coroutine HAS_COROUTINE)
if (NOT "${CMAKE_CXX_STANDARD}" STREQUAL "")
    # Do nothing
elseif (HAS_ANY AND HAS_STRING_VIEW AND HAS_COROUTINE)
    set(CMAKE_CXX_STANDARD 20)
elseif (HAS_ANY AND HAS_STRING_VIEW)
    set(CMAKE_CXX_STANDARD 17)
else ()
    set(CMAKE_CXX_STANDARD 14)
endif ()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(${PROJECT_NAME} main.cc
    docs/ubuntu.sh)

# ##############################################################################
# If you include the drogon source code locally in your project, use this method
# to add drogon 
# add_subdirectory(drogon) 
# target_link_libraries(${PROJECT_NAME} PRIVATE drogon)
#
# and comment out the following lines
find_package(Drogon CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Drogon::Drogon)

find_package(CAF CONFIG REQUIRED)
# tried everything to load --logger.console.verbosity=trace --verbosity=trace from command line argument but not working.
add_definitions(-DCAF_LOG_LEVEL=15)
target_link_libraries(${PROJECT_NAME} PRIVATE CAF::io CAF::core CAF::openssl)

find_package(jsoncons CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE jsoncons)

find_package(date CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE date::date date::date-tz)

find_package(GTest CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME}  PRIVATE GTest::gmock GTest::gtest GTest::gmock_main GTest::gtest_main)

find_package(cpp-jwt CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE cpp-jwt::cpp-jwt)

#find_path(PARALLEL_HASHMAP_INCLUDE_DIRS "parallel_hashmap/btree.h")
#target_include_directories(${PROJECT_NAME} PRIVATE ${PARALLEL_HASHMAP_INCLUDE_DIRS})

#find_package(tsl-ordered-map CONFIG REQUIRED)
#target_link_libraries(${PROJECT_NAME} PRIVATE tsl::ordered_map)

#find_package(Boost REQUIRED COMPONENTS)
#target_link_libraries(${PROJECT_NAME} PRIVATE ${Boost_LIBRARIES})

#find_package(absl CONFIG REQUIRED)
#target_link_libraries(${PROJECT_NAME} PRIVATE absl::str_format)

add_subdirectory(third_party/mgclient)
target_link_libraries(${PROJECT_NAME} PRIVATE mgclient-static)
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/third_party/mgclient/mgclient_cpp/include")

include_directories(inc)
include_directories(SYSTEM third_party/one_file_lib)

# ##############################################################################

if (CMAKE_CXX_STANDARD LESS 17)
    # With C++14, use boost to support any, string_view and filesystem
    message(STATUS "use c++14")
    find_package(Boost 1.61.0 REQUIRED)
    target_link_libraries(${PROJECT_NAME} PUBLIC Boost::boost)
elseif (CMAKE_CXX_STANDARD LESS 20)
    message(STATUS "use c++17")
else ()
    message(STATUS "use c++20")
endif ()

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/controllers CTL_SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/filters FILTER_SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/plugins PLUGIN_SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/models MODEL_SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/actor_system CAF_SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/db DB_SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/lib LIB_SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/utils UTILS_SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src MAIN_SRC)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/pystring UTILS_SRC)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/pystring)

# we are not using drogon_views
#drogon_create_views(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/views
#                    ${CMAKE_CURRENT_BINARY_DIR})
# use the following line to create views with namespaces.
# drogon_create_views(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/views
#                     ${CMAKE_CURRENT_BINARY_DIR} TRUE)

target_include_directories(${PROJECT_NAME}
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                                   ${CMAKE_CURRENT_SOURCE_DIR}/models)
target_sources(${PROJECT_NAME}
               PRIVATE
               ${CTL_SRC}
               ${FILTER_SRC}
               ${PLUGIN_SRC}
               ${MODEL_SRC}
               ${UTILS_SRC}
               ${DB_SRC}
               ${LIB_SRC}
               ${CAF_SRC}
               ${MAIN_SRC}
               )
# ##############################################################################
# uncomment the following line for dynamically loading views 
# set_property(TARGET ${PROJECT_NAME} PROPERTY ENABLE_EXPORTS ON)

# ##############################################################################

add_subdirectory(test)
add_subdirectory(gtest)
