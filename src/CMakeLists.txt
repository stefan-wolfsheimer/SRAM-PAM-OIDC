cmake_minimum_required(VERSION 3.0)

project(pam_sram_oidc VERSION 0.1.0 DESCRIPTION "PAM module")

include_directories(${CMAKE_SOURCE_DIR}/nlohmann-json/single_include)
set (CMAKE_CXX_STANDARD 17)

# Declare the library target.
add_library(pam_sram_oidc
    SHARED
    pam_sram_oidc.c
    logging.c
    sram_oidc_authenticate.cpp
    url_request.cpp    
)

target_link_libraries(pam_sram_oidc -lcurl)

SET_TARGET_PROPERTIES(pam_sram_oidc PROPERTIES PREFIX "")

add_executable(
  pamtester
  pamtester.cpp
)
target_link_libraries(
  pamtester -lpam
)

# todo other os than ubuntu
install(TARGETS pam_sram_oidc
        LIBRARY
        DESTINATION "/usr/lib/x86_64-linux-gnu/security/"
)
