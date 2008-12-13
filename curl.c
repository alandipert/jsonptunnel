/*
 * curl.c
 *
 * Use curl/easy API to make
 * HTTP requests.
 */

#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>
#include "cgic.h"
#include "jsonptunnel.h"


/*
 * Take a struct extRequest and perform
 * the HTTP action specified in req->
 * extMethod.
 */
void postReq(struct extRequest *req) {

  CURL *curl;
  CURLcode res;

  struct curl_httppost *formpost=NULL;
  struct curl_httppost *lastptr=NULL;
  struct curl_slist *headerlist=NULL;
  static const char buf[] = "Expect:";

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
  headerlist = curl_slist_append(headerlist, buf);

  if(curl) {

    /* what URL that receives this POST */
    curl_easy_setopt(curl, CURLOPT_URL, req->url);

    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

    /* send everything to this function */
    //curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, cgiOut);

    res = curl_easy_perform(curl);

    /* always cleanup */
    curl_easy_cleanup(curl);

    /* then cleanup the formpost chain */
    curl_formfree(formpost);

    /* free slist */
    curl_slist_free_all (headerlist);
  }
}
