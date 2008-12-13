#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cgic.h"
#include "jsonptunnel.h"

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

void exit500(char *msg) {
  cgiHeaderStatus(500, msg);
  fprintf(cgiOut, "There was an error processing your request: %s\n", msg);
}

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

struct extArg * makeArg(char *name, char *value) {
  struct extArg *arg = (struct extArg *)malloc(sizeof(struct extArg));

  arg->argName = (char*)malloc(strlen(name)+1);
  arg->argVal = (char*)malloc(strlen(value)+1);

  strcpy(arg->argName, name);
  strcpy(arg->argVal, value);

  return arg;
}

int parseArguments(struct extRequest *req) {

  char **array, **arrayStep;
  if (cgiFormEntries(&array) != cgiFormSuccess) {
    return 0;
  }

  int num_args = countArguments();
  int arg_num = 0;

  req->numargs = num_args;
  req->args = (struct extArg **)malloc(sizeof(struct extArg)*num_args);

  arrayStep = array;
  int val_size = 0;
  char *val;
  while (*arrayStep) {

    if(strcmp(*arrayStep, URL_PARAM_NAME) != 0 &&
       strcmp(*arrayStep, METHOD_PARAM_NAME) != 0) {

      cgiFormStringSpaceNeeded(*arrayStep, &val_size);
      val = (char*)malloc(sizeof(char)*val_size);
      cgiFormString(*arrayStep, val, val_size);
      req->args[arg_num++] = makeArg(*arrayStep, val);
      free(val);

    }
    arrayStep++;
  }
  cgiStringArrayFree(array);

  return 1;
}

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

int cgiMain(void) {

  struct extRequest req;

  if(!parseURL(&req)) {
    exit500("Error parsing URL.\n");
    return EXIT_FAILURE;
  } 

  if(!parseMethod(&req)) {
    exit500("Error parsing HTTP method.\n");
    return EXIT_FAILURE;
  }

  if(!parseArguments(&req)) {
    exit500("Error parsing passed arguments or values.\n");
    return EXIT_FAILURE;
  }

  printParams(&req);

  freeReq(&req);

	return EXIT_SUCCESS;

}
