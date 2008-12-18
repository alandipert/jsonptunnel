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

  return 1;

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

  //subtract 'api' variables
  if(strstr(cgiQueryString,URL_PARAM_NAME))
    count--;

  if(strstr(cgiQueryString,CALLBACK_PARAM_NAME))
    count--;

  fprintf(stdout, "%i variables were passed.", count);
  return count;
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
       strcmp(*arrayStep, CALLBACK_PARAM_NAME) != 0) {

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

