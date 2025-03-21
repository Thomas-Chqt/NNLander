# Lander04 demo
add_executable(lander04 lander04.cpp)

# Link with raylib
target_link_libraries(lander04 raylib)

# Add platform-specific options
if (MSVC)
    # Warning level 4 but don't treat as errors
    target_compile_options(lander04 PRIVATE /W4)
else()
    # Extensive warnings but don't treat as errors
    target_compile_options(lander04 PRIVATE -Wall -Wextra)
endif()

# Installation rules
install(TARGETS lander04 DESTINATION bin) 