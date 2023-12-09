#Quickstart: Building with CMake
#https://google.github.io/googletest/quickstart-cmake.html
#https://github.com/google/googletest/blob/main/googletest/README.md

add_executable(hello_test gtest.cc)
target_sources(hello_test
               PRIVATE
               ${CTL_SRC}
               ${FILTER_SRC}
               ${PLUGIN_SRC}
               ${MODEL_SRC}
               ${UTILS_SRC}
               ${DB_SRC}
               ${CAF_SRC}
               ${MAIN_SRC}
               )

target_include_directories(hello_test
                           PRIVATE ..
                                   ../models
                                   ../third_party/mgclient/mgclient_cpp/include
                               )
target_link_libraries(

  hello_test
#  GTest::gtest_main
  GTest::gtest
  Drogon::Drogon
  mgclient-static
)
target_link_libraries(hello_test  CAF::io CAF::core CAF::openssl)



include(GoogleTest)
gtest_discover_tests(hello_test)
