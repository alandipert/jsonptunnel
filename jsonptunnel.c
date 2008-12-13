/*
 * jsonptunnel.c
 *
 * Use libcgic to parse a GET request
 * and construct an extRequest struct.
 * Pass the struct to the curl functions
 * and print the result of the remote 
 * HTTP action to cgiOut (stdout).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cgic.h"
#include "jsonptunnel.h"

/*
 * The main entrypoint function.
 * libcgic takes care of an actual
 * main().
 */
int cgiMain(void) {

  /* This struct will contain everything about the request. */
  struct extRequest req;

  /* Attempt to load the request with the right information */
  if(!initReq(&req)) {
    return EXIT_FAILURE;
  }

  /* Print the passed variables and arguments. */
  printParams(&req);

  /* Free all the strings malloced inside of req */
  freeReq(&req);

  /* Clean exit */
	return EXIT_SUCCESS;

}

/*
 * Print the contentns of an extRequest
 * and call postReq, also printing the
 * data from the remote site.
 *
 * This is just for debugging.
 */
void printParams(struct extRequest *req) {

	cgiHeaderContentType("text/plain");
	//cgiHeaderContentType("text/json");
  fprintf(cgiOut, "Target URL: %s\n", req->url);
  fprintf(cgiOut, "Target method: ");
  
  switch(req->method) {
    case METHOD_GET: fprintf(cgiOut, "GET\n");
                     break;
    case METHOD_POST:fprintf(cgiOut, "POST\n");
                     break;
    default:         break;
  }

  int i;
  for(i = 0; i < req->numargs; i++) {
    fprintf(cgiOut, "%i. \"%s\" = \"%s\"\n", i+1, req->args[i]->argName,req->args[i]->argVal);
  }

  fprintf(cgiOut, "CURL output:\n");

  postReq(req);

}

/*
 * Send a '400' 'Bad Request' HTTP error code and exit.
 */
void exit400(char *msg) {
  cgiHeaderStatus(400, msg);
  fprintf(cgiOut, "There was an error processing your request: %s\n", msg);
}

/*
 * Look for the external request method
 * and load it into req.
 */
int parseMethod(struct extRequest *req) {

  int methodLength = 5;
  char method[methodLength];

  if(cgiFormString(METHOD_PARAM_NAME, method, methodLength) == cgiFormSuccess) {
    if(strcmp(&method[0], "GET") == 0) {
      req->method = METHOD_GET; 
    } else if(strcmp(&method[0], "POST") == 0) {
      req->method = METHOD_POST;
    } else {
      return 0;
    }
  } else {
    return 0;
  }

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
  //parse URL
  int urlLength = 0;
  if(cgiFormStringSpaceNeeded(URL_PARAM_NAME, &urlLength) == cgiFormSuccess) {
    if(urlLength > 0) {
      req->url = (char*)malloc(sizeof(char)*urlLength);
      if(cgiFormString(URL_PARAM_NAME, req->url, urlLength) == cgiFormSuccess) {
        return 1;
      } else {
        free(req->url);
        return 0;
      }
    } else {
      return 0;
    }
  } else {
    return 0;
  }

  return 1;
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
  free(req->url);
  free(req->args);
}

/*
 * Try to populate a req struct with
 * all the various arguments and
 * parameters.
 */
int initReq(struct extRequest *req) {

  if(!parseURL(req)) {
    exit400("Error parsing URL.\n");
    return 0;
  } 

  if(!parseMethod(req)) {
    exit400("Error parsing HTTP method.\n");
    return 0;
  }

  if(!parseArguments(req)) {
    exit400("Error parsing passed arguments or values.\n");
    return 0;
  }

  return 1;

}

