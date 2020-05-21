
SET(HEADER_PATH ${CMAKE_SOURCE_DIR}/include/vsgsandbox)

set(HEADERS
    ${HEADER_PATH}/Export.h
 )

set(SOURCES
  jpeg/ReaderWriterJPEG.cpp
  ReaderWriter_sandbox/ImageTranslator.cpp
)


add_library(vsgsandbox ${HEADERS} ${SOURCES})




set_property(TARGET vsgsandbox PROPERTY VERSION ${VSGSANDBOX_VERSION_MAJOR}.${VSGSANDBOX_VERSION_MINOR}.${VSGSANDBOX_VERSION_PATCH})
set_property(TARGET vsgsandbox PROPERTY SOVERSION ${VSGSANDBOX_SOVERSION})
set_property(TARGET vsgsandbox PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET vsgsandbox PROPERTY CXX_STANDARD 17)

target_include_directories(vsgsandbox PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(vsgsandbox
    PUBLIC
    vsg::vsg
    ${JPEG_LIBRARIES}
)


install(TARGETS vsgsandbox EXPORT vsgsandbox
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
)

if (BUILD_SHARED_LIBS)
    target_compile_definitions(vsgsandbox INTERFACE VSGSANDBOX_SHARED_LIBRARY)
endif()


install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/vsgsandbox DESTINATION include)

