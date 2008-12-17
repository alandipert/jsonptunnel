//#include <stdio.h>
#include <fcgi_stdio.h>
#include "cgic/cgic.h"
#include "jsonptunnel.h"

void debuglog(char *msg) {
  FILE *logfile = fopen("log", "a");
  fprintf(logfile, msg);
  fclose(logfile);
}

FILE * logptr() {
  return fopen("log", "a");
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

  fprintf(cgiOut, "Hash: %lu\n", req->hash);
  //fprintf(cgiOut, "CURL output:\n");
//
  //doFetch(req);

}
