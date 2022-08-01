cmake_minimum_required(VERSION 3.9)

project(pam_sram_oidc VERSION 0.1.0 DESCRIPTION "PAM module")

include_directories(${CMAKE_SOURCE_DIR}/nlohmann-json/single_include)
set (CMAKE_CXX_STANDARD 17)

# Declare the library target.
add_library(pam_sram_oidc
    SHARED
    pam_sram_oidc.c
    logging.c
    pam_sm_authenticate.cpp
)

add_executable(testme testme.cpp)

# todo other os than ubuntu
install(TARGETS pam_sram_oidc
        LIBRARY
        DESTINATION "/usr/lib/x86_64-linux-gnu/security/"
)
