#if USE_FASTCGI 
#include <fcgi_stdio.h>
#else
#include <stdio.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>
#include "cgic/cgic.h"
#include "jsonptunnel.h"

size_t write_function( void *ptr, size_t size, size_t nmemb, void *stream) {
  return fwrite(ptr, size, nmemb, cgiOut);
}

int doPostReq(struct extRequest *req) {

  cgiHeaderContentType("text/json");

  CURL *curl;
  CURLcode res;

  struct curl_httppost *formpost=NULL;
  struct curl_httppost *lastptr=NULL;

  curl_global_init(CURL_GLOBAL_ALL);

  int i;
  for(i = 0; i < req->numargs; i++) {
    curl_formadd(&formpost,
                 &lastptr,
                 CURLFORM_COPYNAME, req->args[i]->argName,
                 CURLFORM_COPYCONTENTS, req->args[i]->argVal,
                 CURLFORM_END);
  }

  curl = curl_easy_init();

  if(curl) {

    curl_easy_setopt(curl, CURLOPT_URL, req->url);
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_function);

    if(req->callback != NULL) {
      fprintf(cgiOut, "%s(", req->callback);
    }

    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    curl_formfree(formpost);

    if(req->callback != NULL) {
      fprintf(cgiOut, ");");
    }

    return 1;

  } else {

    return 0;
  }
}
