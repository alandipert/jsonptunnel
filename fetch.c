/*
 * fetch.c
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
#include <sys/param.h>
#include "cgic/cgic.h"
#include "jsonptunnel.h"

int returnFile(char *filename) {

  /* The file to pipe out */
  FILE *cache_file;

  /* A nice big buffer size */
  int buf_size = 512;
  char buf[buf_size];
  int *bufptr = &buf[0];
  int obj_size = sizeof(char);

  /* The number of objects read */
  int read;

  if ((cache_file = fopen(filename, "r")) == NULL) {
    return 0;
  }

  while(!feof(cache_file)) {
    read = fread(bufptr, obj_size, buf_size, cache_file);
    fwrite(bufptr, obj_size, read, cgiOut);
  }

  fclose(cache_file);

  return 1;

}

/*
 * The fetch "route" function - sends
 * the request to the right METHOD and
 * checks for req->hash for cached object
 * handling.
 */
int doFetch(struct extRequest *req) {
  cgiHeaderContentType("text/json");
  if(req->hash == 0) {
    if(req->method == METHOD_POST) {
      return doPostReq(req, cgiOut);
    } else {
      return doGetReq(req, cgiOut);
    }
  } else {
    char *cached_file_name = get_cached_filename(req);
    if(cached_file_name == NULL) {

      /* The name of the cached object we're creating */
      char cache_name[MAXPATHLEN];
      sprintf(cache_name, "%lu", req->hash);
      char *fullpath = (char*)malloc(sizeof(char)*(strlen(CACHE_DIR)+strlen(&cache_name[0])+1));
      strcpy(fullpath, CACHE_DIR);
      strcat(fullpath, &cache_name[0]);
      FILE *cache_file;

      if ((cache_file = fopen(fullpath, "a")) == NULL) {
        free(fullpath);
        return 0;
      }

      if(req->method == METHOD_POST) {
        doPostReq(req, cache_file);
      } else {
        doGetReq(req, cache_file);
      }

      fflush(cache_file);
      fclose(cache_file);

      //read from a file that already exists
      returnFile(fullpath);
      //fprintf(cgiOut, "path: %s", fullpath);

      free(fullpath);

      return 1;

    } else {
      //fprintf(cgiOut, "A cached object was found.\n");
      //read from a file that already exists
      returnFile(cached_file_name);
      return 1;
    }



    /* Caching placeholder code */
    //exitStatus(500, "Caching not implemented yet.  Try your request again without setting extCache.");
    //return 1;

    /*
     * TODO:
     *
     * 1. Check for a file named req->hash.
     * 2. If it exists, pipe it to cgiOut and return success.
     * 3. 
     *    3a. If it doesn't exist, check the lock file to see if 
     *    a cached object is in the process of being created.
     *    3b. If it is, sleep for a period of time and check again.
     *    3c. If it is not, create a new file named req->hash.  
     *    Write the hash to the lock.
     *    3d. Open a file descriptor for the new file, and pass this as
     *    the second parameter of the appropriate do*Req function.
     *    3e. Write the hash to a lock file.
     *    3f. Open the new file and pass its contents to cgiOut.
     *    3g. Return success.
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

    //cgiHeaderContentType("text/json");
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
  //struct curl_slist *headerlist=NULL;
  //static const char buf[] = "Expect:";

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
  //headerlist = curl_slist_append(headerlist, buf);

  if(curl) {

    /* what URL that receives this POST */
    curl_easy_setopt(curl, CURLOPT_URL, req->url);

    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

    /* send everything to this function */
    //curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);
    //curl_easy_setopt(curl, CURLOPT_WRITEHEADER, cgiOut);
    //cgiHeaderContentType("text/json");
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
    //curl_slist_free_all (headerlist);

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
