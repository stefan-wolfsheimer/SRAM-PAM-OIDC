extern "C" {
#include <curl/curl.h>
#include <string.h>
}

#include "url_request.h"
#include <stdexcept>

extern "C" {
typedef struct {
	char* payload;
	size_t size;
} CURL_FETCH;

static size_t curl_callback(void* contents, size_t size, size_t nmemb, void* userp) {
	size_t realsize = size * nmemb;         /* calculate buffer size */
	CURL_FETCH *p = (CURL_FETCH *) userp;   /* cast pointer to fetch struct */

	/* expand buffer */
    char * old_payload = p->payload;
	p->payload = (char*)realloc(old_payload, p->size + realsize + 1);

	/* check buffer */
	if (p->payload == NULL) {
        
        /* free buffer */
    	if(old_payload) {
            free(old_payload);
        }
        p->size = 0;
		/* return */
		return (size_t) -1;
	}

	/* copy contents to buffer */
	memcpy(p->payload+p->size, contents, realsize);

	/* set new buffer size */
	p->size += realsize;

	/* ensure null termination */
	p->payload[p->size] = 0;

	/* return size */
	return realsize;
}

} // extern "C"

static std::string _url_encode_string(CURL* curl, const std::string & str) {
    char * esc_str = curl_easy_escape(curl, str.c_str(), str.size());
    std::string ret(esc_str);
    curl_free(esc_str);
    return ret;
}

static std::string _url_decode_string(const char * str, size_t len) {
    char *decoded = curl_unescape(str, len);
    std::string ret(decoded);
    curl_free(decoded);
    return ret;
}

std::string url_encode(const std::string & base_url, const nlohmann::json & j) {
    CURL* curl;
    curl = curl_easy_init();
    std::string ret = base_url;
    bool first = true;
    for (auto it = j.begin(); it != j.end(); ++it) {
        if(first) {
            ret+= '?';
            first = false;
        }
        else {
            ret+= '&';
        }
        ret += _url_encode_string(curl, it.key()) + "=" + _url_encode_string(curl, it.value());
    }
    curl_easy_cleanup(curl);
    return ret;
}

std::string url_encode(const std::string & base_url,
                       const std::map<std::string, std::string> & args) {
    CURL* curl;
    curl = curl_easy_init();
    std::string ret = base_url;
    bool first = true;
    for (const auto & p : args) {
        if(first) {
            ret+= '?';
            first = false;
        }
        else {
            ret+= '&';
        }
        ret += _url_encode_string(curl, p.first) + "=" + _url_encode_string(curl, p.second);
    }
    curl_easy_cleanup(curl);
    return ret;
}

std::map<std::string, std::string> url_parse_query(const std::string & url) {
    std::size_t tok = url.find('?');
    std::size_t ntok;
    std::size_t tok_eq;
    std::size_t size;
    std::map<std::string, std::string> ret;
    size = url.size();
    if(ntok == std::string::npos) {
        ntok = size;
    }
    // edge cases ?a=&=b&a=b
    // ?a&
    while(tok+1 < size) {
        ntok = url.find('&', tok + 1);
        if(ntok == std::string::npos) {
            ntok = size;
        }
        tok_eq = url.find('=', tok + 1);
        if(tok_eq < ntok) {
            ret.insert(std::make_pair(_url_decode_string(url.c_str() + tok + 1, tok_eq - tok - 1),
                                      _url_decode_string(url.c_str() + tok_eq + 1, ntok - tok_eq - 1)));
        }
        else {
            ret.insert(std::make_pair(_url_decode_string(url.c_str() + tok + 1, tok_eq - tok - 1), ""));
        }
        tok = ntok;
    }
    return ret;
}

std::string url_request(const std::string & url,
                        const std::string & method,
                        const std::map<std::string, std::string> & headers,
                        const std::string & data) {
	CURL* curl;
    CURL_FETCH fetcher;

    // Prepare Headers...
	struct curl_slist* request_headers = NULL;

    for(const auto & h : headers) {
        std::string tmp = h.first;
        tmp += ": ";
        tmp += h.second;
        request_headers = curl_slist_append(request_headers, tmp.c_str());
    }

    fetcher.payload = NULL;
	fetcher.size = 0;
    
    // Prepare API request...
	curl = curl_easy_init();
	if (curl) {
		long response_code, perform_status, response_info_code;

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, request_headers);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_callback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&fetcher);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "pam_sram_validate");
        if(data.size()) {
		    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
		    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.size());
        }
		// Perform the request
        perform_status = curl_easy_perform(curl);
        
       
        std::string result(fetcher.size ? fetcher.payload : "");
        if(fetcher.payload) {
            free(fetcher.payload);
        }
        if (perform_status != CURLE_OK) {
            curl_easy_cleanup(curl);
            throw std::runtime_error((std::string("curl_easy_perform(curl) != CURL_OK") +
                                      std::string(" url ") + url + 
                                      std::string(" method ") + method + 
                                      std::string(" data ") + result).c_str());
		}

		// Check response
        response_info_code = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
		if (response_info_code != CURLE_OK) {
            curl_easy_cleanup(curl);
            throw std::runtime_error((std::string("curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE) != CURL_OK") +
                                      std::string(" url ") + url + 
                                      std::string(" method ") + method +
                                      std::string(" data ") + result).c_str());
		}
        // always cleanup
        curl_easy_cleanup(curl);

		// Check response
        if (response_code < 200 || response_code >= 300) {
            throw std::runtime_error((std::string("curl failed (status code) ") + std::to_string(response_code) +
                                      std::string(" url ") + url + 
                                      std::string(" method ") + method +
                                      std::string(" data ") + result).c_str());
		}
        return result;
	}
    return "";
}
