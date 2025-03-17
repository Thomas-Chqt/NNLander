# Lander01 demo
add_executable(lander01 main.cpp)

# Link with raylib
target_link_libraries(lander01 raylib)

# Add platform-specific options
if (MSVC)
    # Warning level 4 but don't treat as errors
    target_compile_options(lander01 PRIVATE /W4)
else()
    # Extensive warnings but don't treat as errors
    target_compile_options(lander01 PRIVATE -Wall -Wextra)
endif()

# Installation rules
install(TARGETS lander01 DESTINATION bin) 