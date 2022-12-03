#Quickstart: Building with CMake
#https://google.github.io/googletest/quickstart-cmake.html
#https://github.com/google/googletest/blob/main/googletest/README.md

add_executable(hello_test gtest.cc)
target_include_directories(hello_test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/mgclient/mgclient_cpp/include")
target_link_libraries(
  hello_test
  GTest::gtest_main
  Drogon::Drogon
  mgclient-static
)



include(GoogleTest)
gtest_discover_tests(hello_test)
