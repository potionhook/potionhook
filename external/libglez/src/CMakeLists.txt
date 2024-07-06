target_sources(glez PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/draw.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/glez.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/font.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/texture.cpp")

add_subdirectory(detail)
add_subdirectory(picopng)