# Lander03 demo
add_executable(lander03 lander03.cpp)

# Link with raylib
target_link_libraries(lander03 raylib)

# Add platform-specific options
if (MSVC)
    # Warning level 4 but don't treat as errors
    target_compile_options(lander03 PRIVATE /W4)
else()
    # Extensive warnings but don't treat as errors
    target_compile_options(lander03 PRIVATE -Wall -Wextra)
endif()

# Installation rules
install(TARGETS lander03 DESTINATION bin)