cmake_minimum_required(VERSION 3.8)
project(demo_navigation)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(demo_mathlib 1.1.0 REQUIRED)

add_executable(navigation_node src/navigation_node.cpp)
target_link_libraries(navigation_node demo_mathlib::demo_mathlib)
ament_target_dependencies(navigation_node rclcpp geometry_msgs)

install(TARGETS navigation_node DESTINATION lib/${PROJECT_NAME})
ament_package()
