set(EXEC_NAME test)


add_executable(${EXEC_NAME}
    test.cpp
    ${CMAKE_SOURCE_DIR}/hw/camera.cpp
)

target_include_directories(${EXEC_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/hw
)

target_link_libraries(${EXEC_NAME}

)

set_target_properties(${EXEC_NAME}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/test/"
)