/*
 * curl.c
 *
 * Use curl/easy API to make
 * HTTP GET/POST requests.
 *
 * A lot of this was adapted from the CURL C api
 * examples:
 * http://curl.haxx.se/libcurl/c/example.html
 */

#include <string.h>
#include <stdlib.h>
#include "cgic/cgic.h"
#include "jsonptunnel.h"

/*
 * The fetch "route" function - sends
 * the request to the right METHOD and
 * checks for req->hash for cached object
 * handling.
 */
int doFetch(struct extRequest *req) {
  if(req->hash == 0) {
    if(req->method == METHOD_POST) {
      return doPostReq(req, cgiOut);
    } else {
      return doGetReq(req, cgiOut);
    }
  } else {

    /* Caching placeholder code */
    exitStatus(500, "Caching not implemented yet.");
    return 1;

    /*
     * TODO:
     *
     * 1. Check for a file named req->hash.
     * 2. If it exists, pipe it to cgiOut and return success.
     * 3. If it doesn't exist, create a new file named req->hash.
     *    Open a file descriptor for the new file, and pass this as
     *    the second parameter of the appropriate do*Req function.
     *    3a. Open the new file and pass its contents to cgiOut.
     *    3b. Return success.
     */
  }
}

/*
 * Used to build URL-encoded query strings.
 * curl_easy_escape is broken for some reason.
 */
char * buildQueryString(CURL * curl, struct extRequest *req) {

  int bufsize = 128;
  int spaceleft = bufsize;
  int urllen = strlen(req->url);
  int size = bufsize+urllen+1;
  char *q = (char*)malloc(sizeof(char)*(size));
  //char *escaped_url;
  strcpy(q, req->url);

  strcat(q, "?");
  spaceleft--;

  int name_len, val_len, i;
  for(i = 0; i < req->numargs; i++) {

    name_len = strlen(req->args[i]->argName);
    val_len = strlen(req->args[i]->argVal);

    while(spaceleft < name_len + val_len + 4) {
      if(realloc(q, size+bufsize) != q) {
        free(q);
        return NULL;
      }
      size += bufsize;
      spaceleft = bufsize;
    }

    strcat(q, req->args[i]->argName);
    strcat(q, "=");
    strcat(q, req->args[i]->argVal);

    if(i != req->numargs-1) {
      strcat(q, "&");
    }
  }

  //fprintf(cgiOut ,"%s\n", q);

  //escaped_url = curl_easy_escape(curl, q, 0);
  //free(q);
  //return escaped_url;

  return q;
}

/*
 * Take a struct extRequest and perform
 * an HTTP get.
 */
int doGetReq(struct extRequest *req, FILE *outputStream) {

  CURL *curl;
  CURLcode res;
  char *escaped_url;

  curl = curl_easy_init(); 

  if(curl) {

    cgiHeaderContentType("text/json");
    if((escaped_url = buildQueryString(curl, req)) == NULL) {
      curl_easy_cleanup(curl);
      return 0;
    }
    curl_easy_setopt(curl, CURLOPT_URL, escaped_url);

    //fprintf(cgiOut, escaped_url);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, outputStream);

    if(req->callback != NULL) {
      /* Append the callback string to output */
      fprintf(outputStream, "%s(", req->callback);
    }

    res = curl_easy_perform(curl);

    /* always cleanup */
    curl_easy_cleanup(curl);

    if(req->callback != NULL) {
      /* Append the callback string to output */
      fprintf(outputStream, ");");
    }

    //curl_free(escaped_url);
    free(escaped_url);

    /* Things went well, return 1 for good times. */
    return 1;

  } else {

    /* Ruh-roh, it's broken. */
    return 0;
  }
}

/*
 * Take a struct extRequest and perform
 * an HTTP post.
 */
int doPostReq(struct extRequest *req, FILE *outputStream) {

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
    //curl_easy_setopt(curl, CURLOPT_WRITEHEADER, cgiOut);
    cgiHeaderContentType("text/json");
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, outputStream);

    if(req->callback != NULL) {
      /* Append the callback string to output */
      fprintf(outputStream, "%s(", req->callback);
    }

    res = curl_easy_perform(curl);

    /* always cleanup */
    curl_easy_cleanup(curl);

    /* then cleanup the formpost chain */
    curl_formfree(formpost);

    /* free slist */
    curl_slist_free_all (headerlist);

    if(req->callback != NULL) {
      fprintf(outputStream, ");");
    }

    /* Things went well, return 1 for good times. */
    return 1;

  } else {

    /* Ruh-roh, it's broken. */
    return 0;
  }
}
