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

add_executable(${PROJECT_NAME} main.cc)

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
target_link_libraries(${PROJECT_NAME} PRIVATE CAF::io CAF::core CAF::test CAF::openssl)

find_package(jsoncons CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE jsoncons)

find_package(date CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE date::date date::date-tz)

find_package(GTest CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME}  PRIVATE GTest::gmock GTest::gtest GTest::gmock_main GTest::gtest_main)

find_package(cpp-jwt CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE cpp-jwt::cpp-jwt)

find_path(PARALLEL_HASHMAP_INCLUDE_DIRS "parallel_hashmap/btree.h")
target_include_directories(${PROJECT_NAME} PRIVATE ${PARALLEL_HASHMAP_INCLUDE_DIRS})

find_package(tsl-ordered-map CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE tsl::ordered_map)

find_package(Boost REQUIRED COMPONENTS)
target_link_libraries(${PROJECT_NAME} PRIVATE ${Boost_LIBRARIES})

find_package(absl CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE absl::str_format)

add_subdirectory(third_party/velocypack)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/third_party/velocypack/include")
target_link_libraries(${PROJECT_NAME} PRIVATE velocypack)

add_subdirectory(third_party/mgclient)
target_link_libraries(${PROJECT_NAME} PRIVATE mgclient-static)
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/third_party/mgclient/mgclient_cpp/include")


# nghtt2:
# https://github.com/microsoft/vcpkg/issues/6951#issuecomment-533268989
find_path(nghttp2_INCLUDE_DIRS nghttp2.h PATH_SUFFIXES nghttp2)

set(nghttp2_INCLUDE_DIRS ${nghttp2_INCLUDE_DIRS}/../)
include(SelectLibraryConfigurations)
find_library(nghttp2_LIBRARY_RELEASE NAMES nghttp2)
find_library(nghttp2_LIBRARY_DEBUG NAMES nghttp2 HINTS debug)
select_library_configurations(nghttp2)

target_include_directories(${PROJECT_NAME} PRIVATE ${nghttp2_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${nghttp2_LIBRARY_DEBUG})

# llhttp:
SET(LLHTTP_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/llhttp/src/api.c
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/llhttp/src/http.c
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/llhttp/src/llhttp.c
)

add_library(llhttp STATIC
  ${LLHTTP_SOURCES}
)

target_include_directories(llhttp PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/third_party/llhttp/include")

# fuerte:
add_library(fuerte STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/fuerte/src/connection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/fuerte/src/ConnectionBuilder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/fuerte/src/helper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/fuerte/src/http.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/fuerte/src/H1Connection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/fuerte/src/H2Connection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/fuerte/src/jwt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/fuerte/src/loop.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/fuerte/src/message.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/fuerte/src/requests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/fuerte/src/types.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/fuerte/src/vst.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/fuerte/src/VstConnection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/fuerte/src/connection.cpp
  )

target_link_libraries(fuerte PUBLIC ${nghttp2_LIBRARY_DEBUG})
target_link_libraries(fuerte PUBLIC llhttp)
# target_link_libraries(fuerte PUBLIC boost_system)
target_include_directories(fuerte SYSTEM PUBLIC "${Boost_INCLUDE_DIR}") # /home/k/ok_web_backend/vcpkg/installed/x64-linux/include
target_link_libraries(fuerte PUBLIC velocypack)

include(CheckTypeSize)
check_type_size("ssize_t" SIZEOF_SSIZE_T)
if(SIZEOF_SSIZE_T STREQUAL "")
  # ssize_t is a signed type in POSIX storing at least -1.
  # Set it to "int" to match the behavior of AC_TYPE_SSIZE_T (autotools).
  target_compile_definitions(fuerte PRIVATE ssize_t=int)
endif()

target_include_directories(fuerte SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/third_party/fuerte/include")
target_include_directories(fuerte PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/third_party/llhttp/include")
# target_include_directories(fuerte SYSTEM PUBLIC "${OPENSSL_INCLUDE_DIR}")
target_link_libraries(${PROJECT_NAME} PRIVATE fuerte)

include_directories(inc)
include_directories(SYSTEM third_party/one_file_lib)
include_directories(SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/third_party/fuerte/include")

find_package(vmime REQUIRED) #note wmime dynamically link current system icu libraries. when system is upgraded just recomile it.
#note vmime must compile with correct stdlib
target_link_libraries(${PROJECT_NAME} PRIVATE vmime)

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

aux_source_directory(controllers CTL_SRC)
aux_source_directory(filters FILTER_SRC)
aux_source_directory(plugins PLUGIN_SRC)
aux_source_directory(models MODEL_SRC)
aux_source_directory(src/actor_system CAF_SRC)
aux_source_directory(src/db DB_SRC)
aux_source_directory(src/utils UTILS_SRC)
aux_source_directory(src/table TABLE_SRC)
aux_source_directory(src/mutate MUTATE_SRC)
aux_source_directory(src MAIN_SRC)

aux_source_directory(third_party/pystring UTILS_SRC)
include_directories(third_party/pystring)


drogon_create_views(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/views
                    ${CMAKE_CURRENT_BINARY_DIR})
# use the following line to create views with namespaces.
# drogon_create_views(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/views
#                     ${CMAKE_CURRENT_BINARY_DIR} TRUE)

target_include_directories(${PROJECT_NAME}
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                                   ${CMAKE_CURRENT_SOURCE_DIR}/models)
target_sources(${PROJECT_NAME}
               PRIVATE
               ${SRC_DIR}
               ${CTL_SRC}
               ${FILTER_SRC}
               ${PLUGIN_SRC}
               ${MODEL_SRC}
               ${UTILS_SRC}
               ${DB_SRC}
               ${CAF_SRC}
               ${TABLE_SRC}
               ${MUTATE_SRC}
               ${MAIN_SRC}
               )
# ##############################################################################
# uncomment the following line for dynamically loading views 
# set_property(TARGET ${PROJECT_NAME} PROPERTY ENABLE_EXPORTS ON)

# ##############################################################################

add_subdirectory(test)
