extern "C" {
    #include "sram_oidc_authenticate.h"
    #include "logging.h"
    #include <stdlib.h>
}
#include <iostream>
#include <fstream>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

#include "nlohmann/json.hpp"

#include "url_request.h"

nlohmann::json read_config_file(const std::string & fname) {
    std::ifstream ifs(fname.c_str());
    if (ifs.is_open()) {
        return nlohmann::json::parse(ifs);
    }
    else {
        throw std::runtime_error((std::string("Could not open file '") + fname + std::string("'")).c_str());
    }
}

inline std::string get_token_from_client(pam_handle_t *pamh) {
    const char * payload = "{\"retrieve\": \"/oauth2_access_token\"}";
    char *resp;
    int retval;
    retval = pam_prompt(pamh,
                        PAM_PROMPT_ECHO_ON,
                        &resp,
                        "%s",
                        payload);
    if (retval != PAM_SUCCESS)
        throw std::runtime_error("pam_prompt: failed");
    if (resp == NULL)
        throw std::runtime_error("pam_prompt: conversation error");
    return std::string(resp);
}

inline std::string get_value_from_map(const std::map<std::string, std::string> & m, const std::string & key) {
    auto itr = m.find(key);
    if(itr == m.end()) {
        return std::string("");
    }
    else {
        return itr->second;
    }    
}

inline std::string oidc_get_token(const std::string & callback,
                                   const std::string & redirectUri,
                                   const std::string & tokenEP,
                                   const std::string & client_id,
                                   const std::string & client_secret) {
    using json = nlohmann::json;
    auto m = url_parse_query(callback);
    json j = {
        {"grant_type", "authorization_code"},
        {"code", get_value_from_map(m, "code")},
        {"state", get_value_from_map(m, "state")},
        {"redirect_uri", redirectUri},
        {"scope", get_value_from_map(m, "scope")},
        {"client_id", client_id},
        {"client_secret", client_secret}};
    std::string text = url_request(tokenEP,
                                   "POST",
                                   {{"Content-Type", "application/json"}},
                                   j.dump());
    json resp = json::parse(text);
    return resp.value("access_token", "");
}

inline std::string oidc_authenticate(pam_handle_t *pamh, const nlohmann::json & cfg) {
    using random_generator = boost::uuids::random_generator;
    using uuid = boost::uuids::uuid; 
    random_generator generator;
    uuid _uuid = generator();
    std::string url = url_encode(cfg.value("authorization_ep", ""),
        std::map<std::string, std::string>{
            {"response_type", cfg.value("response_type", "code")},
            {"client_id", cfg.value("client_id", "")},
            {"redirect_uri", cfg.value("redirect_uri", "")},
            {"scope", cfg.value("scope", "")},
            {"access_type", cfg.value("access_type", "")},
            {"prompt", cfg.value("prompt", "")},
            {"state", boost::lexical_cast<std::string>(_uuid)}
        });
    int retval = pam_prompt(pamh,
                            PAM_TEXT_INFO,
                            NULL,
                            "%s",
                            "Copy the following URL to your web browser:");
    if (retval != PAM_SUCCESS) {
        throw std::runtime_error("pam_prompt: failed");
    }
    retval = pam_prompt(pamh,
                            PAM_TEXT_INFO,
                            NULL,
                            "%s",
                            url.c_str());
    if (retval != PAM_SUCCESS)
        throw std::runtime_error("pam_prompt: failed");

    char * resp;
    retval = pam_prompt(pamh, PAM_PROMPT_ECHO_ON, &resp, "url:");
    if (retval != PAM_SUCCESS)
        throw std::runtime_error("pam_prompt: failed");
    // Get a token
    return oidc_get_token(std::string(resp),
                          cfg.value("redirect_uri", ""),
                          cfg.value("token_ep", ""),
                          cfg.value("client_id", ""),
                          cfg.value("client_secret", ""));
}

nlohmann::json validate_token(const std::string & token, const std::string & introspectUrl) {
    using json = nlohmann::json;
    std::string header("Bearer ");
    header+= token;
    std::string res = url_request(introspectUrl, "GET", 
                                 {{"Authorization", header}},
                                 "");
    return json::parse(res);
}

bool check_user(const std::string & user,
                const std::string & remote_user) {
    std::size_t pos = remote_user.find('@');
    if(remote_user.compare(0, pos, user) == 0){
        return true;
    }
    else {
        return false;
    }
}

int sram_oidc_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    using json = nlohmann::json;
    logging_pamh = pamh;
    if(argc < 1) {
        logging(LOG_ERR, "pam_sram_oidc.so: missing first argument (SRAM configuration file)");
        return PAM_AUTH_ERR;
    }
    char *user = NULL;
    pam_get_user(pamh, (const char **) &user, "user: ");
    if (!user) {
        return PAM_USER_UNKNOWN;
    }

    json config;
    try {
        config = read_config_file(argv[0]);
        json::json_pointer result_ptr(config.value("result_field",
                                                   "/eduperson_principal_name/0"));
        std::string token = get_token_from_client(pamh);
        bool save_token = false;
        if(token.empty()) {
            save_token = true;
            token = oidc_authenticate(pamh, config);
        }
        auto result = validate_token(token, config.value("introspect_url", ""));
        if(!result.contains("sub")) {
            return PAM_AUTH_ERR;
        }
        if(result.contains(result_ptr)) {
            if(check_user(user, result.at(result_ptr).get<std::string>())) {
                return PAM_SUCCESS;
            }
        }
    } catch (const std::exception & ex) {
        logging(LOG_ERR, "pam_sram_oidc.so: failed to load %s", ex.what());
        return PAM_AUTH_ERR;
    }
    return PAM_AUTH_ERR;
}
