# Lander02 demo
add_executable(lander02 lander02.cpp)

# Link with raylib
target_link_libraries(lander02 raylib)

# Add platform-specific options
if (MSVC)
    # Warning level 4 but don't treat as errors
    target_compile_options(lander02 PRIVATE /W4)
else()
    # Extensive warnings but don't treat as errors
    target_compile_options(lander02 PRIVATE -Wall -Wextra)
endif()

# Installation rules
install(TARGETS lander02 DESTINATION bin)