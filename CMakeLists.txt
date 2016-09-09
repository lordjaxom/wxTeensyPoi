cmake_minimum_required(VERSION 3.5)
project(wxTeensyPoi)

# Environment Configuration
# set(wxWidgets_ROOT_DIR E:/MinGW/wxWidgets-3.1.0)
set(wxWidgets_ROOT_DIR C:/Users/lordjaxom/Programme/wxWidgets-3.1.0)

add_compile_options(-Wall -Wno-unused-variable -Wno-strict-aliasing -std=c++11)

set(SOURCE_FILES
    debug.cpp
    debug.hpp
    poi_accessor.cpp
    poi_accessor.hpp
    poi_commport.cpp
    poi_commport.hpp
    poi_communicator.cpp
    poi_communicator.hpp
    poi_exception.hpp
    poi_protocol.hpp
    poi_simulator.hpp
    poi_state.cpp
    poi_state.hpp
    poi_structure.cpp
    poi_structure.hpp
    wx_app.cpp
    wx_app.hpp
    wx_artprovider.cpp
    wx_artprovider.hpp
    wx_frame.cpp
    wx_frame.hpp
    wx_generated.cpp
    wx_generated.h
    wx_resource.rc)

find_package(Boost COMPONENTS system thread REQUIRED)
find_package(wxWidgets COMPONENTS core base REQUIRED)

add_executable(wxTeensyPoi ${SOURCE_FILES})
target_compile_definitions(wxTeensyPoi PRIVATE WIN32_LEAN_AND_MEAN)
target_include_directories(wxTeensyPoi PRIVATE ${Boost_INCLUDE_DIRS} ${wxWidgets_INCLUDE_DIRS} ../teensy_poi/src)
target_link_libraries(wxTeensyPoi ${Boost_LIBRARIES} ${wxWidgets_LIBRARIES})
if (WIN32)
    target_link_libraries(wxTeensyPoi version shlwapi setupapi)
endif()

add_custom_command(TARGET wxTeensyPoi POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/icons $<TARGET_FILE_DIR:wxTeensyPoi>/icons)
