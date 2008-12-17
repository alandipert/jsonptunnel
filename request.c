//#include <stdio.h>
#include <fcgi_stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cgic/cgic.h"
#include "jsonptunnel.h"

/*
 * Try to populate a req struct with
 * all the various arguments and
 * parameters.
 */
int initReq(struct extRequest *req) {

  req->url = NULL;
  req->args = NULL;
  req->numargs = 0;

  if(!parseURL(req)) {
    exitStatus(400, "Error parsing URL.\n");
    return 0;
  } 

  if(!parseMethod(req)) {
    exitStatus(400, "Error parsing HTTP method.\n");
    return 0;
  }

  if(!parseArguments(req)) {
    exitStatus(400, "Error parsing passed arguments or values.\n");
    return 0;
  }

  /*
   * The callback function is not required,
   * because some JSON-producing webservices
   * provide their own parameter for this.
   */
  parseCallback(req);

  /*
   * If extCache is set to anything, create
   * a hash using hash_str in cache.c and 
   * set req->hash to this number.
   *
   * extCache is optional.
   */
  parseCache(req);

  return 1;

}
/*
 * Look for the external request method
 * and load it into req.
 */
int parseMethod(struct extRequest *req) {

  int return_status = 0;
  int methodLength = 5;
  char method[methodLength];

  if(cgiFormString(METHOD_PARAM_NAME, method, methodLength) == cgiFormSuccess) {
    if(strcmp(&method[0], "GET") == 0) {
      req->method = METHOD_GET; 
      return_status = 1;
    } else if(strcmp(&method[0], "POST") == 0) {
      return_status = 1;
      req->method = METHOD_POST;
    } 
  }

  return return_status;
}

/*
 * Count the number of arguments other than
 * extURL, extMethod, and extCallback.
 */
int countArguments(void) {

  int count = 0;
  char **array, **arrayStep;
  if(cgiFormEntries(&array) != cgiFormSuccess) {
    return 0;
  }

  arrayStep = array;
  while(*arrayStep) {
    arrayStep++;
    count++;
  }

  cgiStringArrayFree(array);

  //We subtract two for extURL and extMethod
  return count-2;

}

/*
 * Given name and value strings, make an extArg
 * struct.  This will be added to the array of
 * extArgs in the request struct.
 */
struct extArg * makeArg(char *name, char *value) {

  struct extArg *arg = (struct extArg *)malloc(sizeof(struct extArg));

  /* Make enough room in the struct for the strings and a NUL char. */
  arg->argName = (char*)malloc(strlen(name)+1);
  arg->argVal = (char*)malloc(strlen(value)+1);

  /* Copy the values over */
  strcpy(arg->argName, name);
  strcpy(arg->argVal, value);

  return arg;
}

/*
 * Check for an "extCache" parameter
 * and don't worry about its value.  If it's set,
 * we'll figured out the FNV hash of the query string
 * and store it in req.  Then later before curl gets called
 * we'll check for a cached object to return.
 *
 * If it's not set, we set req->hash to 0 and don't
 * cache anything.
 */
int parseCache(struct extRequest *req) {
  int return_status = 0;
  int cacheLength = 0;
  if(cgiFormStringSpaceNeeded(CACHE_PARAM_NAME, &cacheLength) == cgiFormSuccess) {
    req->hash = hash_str(cgiQueryString);
    return_status = 1;
  } else {
    req->hash = 0;
  }

  return return_status;
}

/*
 * Parse out the callback method and
 * add it to the req struct.
 */
int parseCallback(struct extRequest *req) {
  int return_status = 0;
  int callbackLength = 0;
  if(cgiFormStringSpaceNeeded(CALLBACK_PARAM_NAME, &callbackLength) == cgiFormSuccess) {
    if(callbackLength > 0) {
      req->callback = (char*)malloc(sizeof(char)*callbackLength);
      if(cgiFormString(CALLBACK_PARAM_NAME, req->callback, callbackLength) == cgiFormSuccess) {
        return_status = 1;
      } else {
        free(req->callback);
      }
    } 
  }

  /*
   * Set req->callback to NULL
   * so that it's not printed
   * in curl.c
   */
  if(!return_status) {
    req->callback = NULL;
  }

  return return_status;
  
}

/*
 * Iterate through the passed name=value pairs
 * and construct extArg structs for each of them.
 *
 * Add them to the array of extArg structs inside
 * the req struct 
 */
int parseArguments(struct extRequest *req) {

  char **array, **arrayStep;
  if (cgiFormEntries(&array) != cgiFormSuccess) {
    return 0;
  }

  int num_args = countArguments();
  int arg_num = 0;

  req->numargs = num_args;
  /* make req->args big enough to hold all the extArg structs */
  req->args = (struct extArg **)malloc(sizeof(struct extArg)*num_args);

  arrayStep = array;
  int val_size = 0;
  char *val;
  while (*arrayStep) {

    /* If the name is not a URL, method, or callback, we need to add it */
    if(strcmp(*arrayStep, URL_PARAM_NAME) != 0 &&
       strcmp(*arrayStep, METHOD_PARAM_NAME) != 0) {

      /* Get the length of the passed string so our buffer is big enough. */
      cgiFormStringSpaceNeeded(*arrayStep, &val_size);
      val = (char*)malloc(sizeof(char)*val_size);
      cgiFormString(*arrayStep, val, val_size);
      req->args[arg_num++] = makeArg(*arrayStep, val);
      /* The val has been passed to makeArg, so free it */
      free(val);

    }
    arrayStep++;
  }
  cgiStringArrayFree(array);

  return 1;
}

/*
 * Determine the URL to send data to.
 */
int parseURL(struct extRequest *req) {

  int return_status = 0;
  int urlLength = 0;

  if(cgiFormStringSpaceNeeded(URL_PARAM_NAME, &urlLength) == cgiFormSuccess) {
    if(urlLength > 0) {
      req->url = (char*)malloc(sizeof(char)*urlLength);
      if(cgiFormString(URL_PARAM_NAME, req->url, urlLength) == cgiFormSuccess) {
        /* Success */
        return_status = 1;
      } else {
        free(req->url);
      }
    } 
  }

  return return_status;
}

/*
 * Free all the malloc'd strings inside of
 * a req struct.
 */
void freeReq(struct extRequest *req) {
  //free argument strings
  int i;
  for(i = 0; i < req->numargs; i++) {
    free(req->args[i]->argName);
    free(req->args[i]->argVal);
  }

  //free URL and method 
  if(req->url != NULL) 
    free(req->url);

  if(req->args != NULL)
    free(req->args);
}

