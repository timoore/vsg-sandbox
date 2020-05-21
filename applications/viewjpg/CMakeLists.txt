set(SOURCES
    viewjpg.cpp
)

add_executable(viewjpg ${SOURCES})

target_include_directories(viewjpg PRIVATE
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
)

set_target_properties(viewjpg PROPERTIES OUTPUT_NAME viewjpg)

target_link_libraries(viewjpg
  vsgsandbox
  vsg::vsg
)

install(TARGETS viewjpg
        RUNTIME DESTINATION bin
)
