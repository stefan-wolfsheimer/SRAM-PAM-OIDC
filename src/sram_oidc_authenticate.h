#pragma once

#include <security/pam_ext.h>
#include <security/pam_appl.h>
#include <security/pam_modules.h>

int sram_oidc_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv);