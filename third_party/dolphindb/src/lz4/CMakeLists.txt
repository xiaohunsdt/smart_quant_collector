target_include_directories(${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include/lz4
)

target_sources(${PROJECT_NAME} PRIVATE
    lz4.c
)
