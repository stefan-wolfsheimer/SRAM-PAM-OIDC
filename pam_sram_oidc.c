#define PAM_SM_ACCOUNT
#define PAM_SM_AUTH
#define PAM_SM_PASSWORD
#define PAM_SM_SESSION
#include "defs.h"
#include "logging.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <security/pam_ext.h>
#include <security/pam_appl.h>
#include <security/pam_modules.h>

//#include "pam_sram_validate.h"

int pam_sm_open_session(UNUSED pam_handle_t *pamh, UNUSED int flags, UNUSED int argc, UNUSED const char **argv) {
    return (PAM_SUCCESS);
}

int pam_sm_close_session(UNUSED pam_handle_t *pamh, UNUSED int flags, UNUSED int argc, UNUSED const char **argv) {
    return (PAM_SUCCESS);
}

int pam_sm_acct_mgmt(UNUSED pam_handle_t *pamh, UNUSED int flags, UNUSED int argc, UNUSED const char **argv) {
    return (PAM_SUCCESS);
}

int pam_sm_authenticate(pam_handle_t *pamh, UNUSED int flags, int argc, const char **argv) {
    char *user = NULL;
    char *password = NULL;
    logging_pamh = pamh;
    int i;
    for(i = 0; i < argc; i++) {
        logging(LOG_ERR, argv[i]);
    }
    return PAM_AUTH_ERR;
    return (PAM_SUCCESS);
}

int pam_sm_setcred(UNUSED pam_handle_t *pamh, UNUSED int flags, UNUSED int argc, UNUSED const char **argv) {
    return (PAM_SUCCESS);
}

int pam_sm_chauthtok(UNUSED pam_handle_t *pamh, UNUSED int flags,UNUSED  int argc, UNUSED const char **argv) {
    return (PAM_SUCCESS);
}