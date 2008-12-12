#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
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

int parseURL(struct extRequest *req) {
  //parse URL
  int urlLength = 0;
  if(cgiFormStringSpaceNeeded(URL_PARAM_NAME, &urlLength) == cgiFormSuccess) {
    if(urlLength > 0) {
      req->url = (char*)malloc(sizeof(char)*urlLength);
      if(cgiFormString(URL_PARAM_NAME, req->url, urlLength) == cgiFormSuccess) {
        return 1;
      } else {
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

int cgiMain() {

  struct extRequest req;

  if(!parseURL(&req)) {
    exit500("Error parsing URL.\n");
    return EXIT_FAILURE;
  } 

  if(!parseMethod(&req)) {
    exit500("Error parsing HTTP method\n");
    return EXIT_FAILURE;
  }

  printParams(&req);

	return EXIT_SUCCESS;

}
