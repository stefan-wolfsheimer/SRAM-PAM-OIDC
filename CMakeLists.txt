cmake_minimum_required(VERSION 3.0)

project(pam_sram_oidc VERSION 0.1.0 DESCRIPTION "PAM module")

include_directories(${CMAKE_SOURCE_DIR}/nlohmann-json/single_include)
set (CMAKE_CXX_STANDARD 17)

# Declare the library target.
add_library(pam_sram_oidc
    SHARED
    src/pam_sram_oidc.c
    src/logging.c
    src/sram_oidc_authenticate.cpp
    src/url_request.cpp    
)

target_link_libraries(pam_sram_oidc -lcurl)

SET_TARGET_PROPERTIES(pam_sram_oidc PROPERTIES PREFIX "")

add_executable(
  sram_pamtester
  src/pamtester.cpp
)
target_link_libraries(
  sram_pamtester -lpam
)

# todo other os than ubuntu
install(TARGETS pam_sram_oidc
        LIBRARY
        DESTINATION "/usr/lib/x86_64-linux-gnu/security/"
)

install(TARGETS sram_pamtester
        LIBRARY
        DESTINATION "/usr/bin"
)

set(CPACK_PACKAGING_INSTALL_PREFIX "/")
install_files(/etc/pam.d FILES docker/sram)
install_files(/etc/ FILES docker/sram_config.json.template)

set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

set(CPACK_DEBIAN_PACKAGE_NAME "sram-pam-oidc")
set(CPACK_DEBIAN_PACKAGE_VERSION 0.0.1)
set(CPACK_DEBIAN_PACKAGE_RELEASE 1)

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "OIDC workflow for PAM")

set(CPACK_DEBIAN_PACKAGE_DEPENDS "libpam-dev, libcurl4-gnutls-dev")
set(CPACK_PACKAGE_CONTACT "SURF")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Stefan Wolfsheimer")
set(CPACK_PACKAGE_DIRECTORY "/packages")

include(CPack)
