cmake_minimum_required(VERSION 3.10)
project(ssl_config_tests)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -g")

# SSL配置单元测试
add_executable(ssl_config_test ssl_config_test.c)

# 编译选项
target_compile_options(ssl_config_test PRIVATE ${CMAKE_C_FLAGS})

# 链接选项（仅链接必要库）
target_link_options(ssl_config_test PRIVATE)

# 测试套件
enable_testing()
add_test(NAME ssl_config_test COMMAND ssl_config_test)
