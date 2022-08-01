/***********************************************************
 * 
 **/

//#include "auth_check_wrapper.hpp"
#include <string>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <termios.h>

#include <string>
#include <utility>
#include <security/pam_appl.h>
//#include "ipam_client.hpp"
//#include "pam_auth_check_exception.hpp"
#include <stdexcept>
#include <string>
#include <sstream>
#include <security/pam_appl.h>

//struct pam_response_t;

void promptEchoOff(const char * msg, struct pam_response * resp)
{
    std::cout << msg << std::endl;
    resp->resp_retcode = 0;
    resp->resp = strdup(getpass(""));
}

void promptEchoOn(const char * msg, struct pam_response * resp)
{
    std::cout << msg << std::endl;
    resp->resp_retcode = 0;
    std::size_t len = 0;
    resp->resp = NULL;
    if(getline(&resp->resp, &len, stdin) == -1)
    {
      throw ::std::runtime_error("failed to read message");
    }
    if(resp->resp)
    {
      resp->resp[strlen(resp->resp)-1] = '\0';
    }
}

void errorMsg(const char * msg)
{
    std::cout << msg << std::endl;
}

void textInfo(const char * msg)
{
    std::cout << msg << std::endl;
}

int pam_conversation(int n,
                     const struct pam_message **msg,
                     struct pam_response **resp,
                     void *data)
{
  struct pam_response *aresp;
  if (n <= 0 || n > PAM_MAX_NUM_MSG)
  {
    return PAM_CONV_ERR;
  }
  if ((aresp = (pam_response*)calloc(n, sizeof *aresp)) == NULL)
  {
    return PAM_BUF_ERR;
  }
  try
  {
    for(int i = 0; i < n; ++i)
    {
      aresp[i].resp_retcode = 0;
      aresp[i].resp = NULL;
      switch (msg[i]->msg_style)
      {
      case PAM_PROMPT_ECHO_OFF:
        promptEchoOff(msg[i]->msg, &aresp[i]);
        break;
      case PAM_PROMPT_ECHO_ON:
        promptEchoOn(msg[i]->msg, &aresp[i]);
        break;
      case PAM_ERROR_MSG:
        errorMsg(msg[i]->msg);
        break;
      case PAM_TEXT_INFO:
        textInfo(msg[i]->msg);
        break;
      default:
        throw std::invalid_argument(std::string("invalid PAM message style ") +
                                    std::to_string(msg[i]->msg_style));
      }
    }
  }
  catch(std::exception & ex)
  {
    // cleanup
    for (int i = 0; i < n; ++i)
    {
      if (aresp[i].resp != NULL)
      {
        memset(aresp[i].resp, 0, strlen(aresp[i].resp));
        free(aresp[i].resp);
      }
    }
    memset(aresp, 0, n * sizeof(*aresp));
    *resp = NULL;
    return PAM_CONV_ERR;
  }
  *resp = aresp;
  return PAM_SUCCESS;
}

bool pam_auth_check(const std::string & pam_service, bool verbose)
{
  pam_handle_t *pamh = nullptr;
  pam_conv conv = { pam_conversation, nullptr };
  const int retval_pam_start = pam_start(pam_service.c_str(),
                                         nullptr,
                                         &conv,
                                         &pamh );
  if(verbose)
  {
    std::cout << "pam_start: " << retval_pam_start << std::endl;
  }
  if(retval_pam_start != PAM_SUCCESS)
  {
    throw std::runtime_error("pam_start_error");
  }
  const int retval_pam_authenticate = pam_authenticate( pamh, 0 );
  if(verbose)
  {
    if(retval_pam_authenticate == PAM_SUCCESS)
    {
      std::cout << "pam_authenticate: PAM_SUCCESS" << std::endl;
    }
    else
    {
      std::cout << "pam_authenticate: " << retval_pam_authenticate
                << ": "
                << pam_strerror(pamh, retval_pam_authenticate)
                << std::endl;
    }
  }
  if(pam_end( pamh, retval_pam_authenticate ) != PAM_SUCCESS)
  {
    pamh = NULL;
    throw std::runtime_error("irodsPamAuthCheck: failed to release authenticator");
  }
  if(retval_pam_authenticate != PAM_AUTH_ERR &&
     retval_pam_authenticate != PAM_SUCCESS &&
     retval_pam_authenticate != PAM_USER_UNKNOWN)
  {
    std::stringstream ss;
    ss << "pam_authenticate: " << retval_pam_authenticate
       << ": "
       << pam_strerror(pamh, retval_pam_authenticate)
       << std::endl;
       
    throw std::runtime_error(ss.str().c_str());
  }
  return retval_pam_authenticate == PAM_SUCCESS;
}

/**
 * Parse a string value from argv[i]
 *
 * \param argc total number of arguments
 * \param const char ** argv argument values
 * \param int & i current possition. (the value is incremented by 1
 * \param bool & argError set to true if an error occurred
 * \return the value parsed from argv[i]
 */
static std::string parseString(int argc, const char ** argv, int & i, bool & argError)
{
  ++i;
  if(i < argc)
  {
    return std::string(argv[i]);
  }
  else
  {
    std::cerr << "missing argument " << argv[i-1] << " N" << std::endl;
    argError = true;
  }
  return std::string("");
}

/**
 * Execute PAM conversation on command line
 */
int main(int argc, const char ** argv)
{
    /* set default arguments */
    std::string pamStackName = "sram";
    bool printHelp = false;
    bool argError = false;
    bool verbose = false;

    /* parse and validate arguments  */
    for(int i = 0; i < argc; ++i)
    {
        std::string arg(argv[i]);
        if(arg == "--stack")
        {
            pamStackName = parseString(argc, argv, i, argError);
        }
        else if(arg == "--help" || arg == "-h")
        {
            printHelp = true;
        }
        else if(arg == "--verbose" || arg == "-v")
        {
            verbose = true;
        }
    }
    if(printHelp || argError)
    {
        std::cout << argv[0] << "[OPTIONS]" << std::endl;
        std::cout << "OPTIONS:" << std::endl;
        std::cout << "--stack PAM_STACK_NAME" << std::endl;
        std::cout << "--verbose|-v" << std::endl;
        std::cout << "--help|-h" << std::endl;
        if(argError)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }

    /* run PAM conversation  */
    bool result = false;
    result = pam_auth_check(pamStackName, verbose);
 
    if(result)
    {
        std::cout << "Authenticated" << std::endl;
    }
    else
    {
        std::cout << "Not Authenticated" << std::endl;
    }
    return 0;
}