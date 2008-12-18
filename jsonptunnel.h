#include <fcgi_stdio.h>
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

/*
 * Required parameters.
 */
#define URL_PARAM_NAME "extURL"

/*
 * Optional parameters
 */
#define CALLBACK_PARAM_NAME "extCallback"

/*
 * Represents a name=value pair.
 */
struct extArg {
  char *argName;
  char *argVal;
} extArg;

/*
 * Represents a 'request'
 */
struct extRequest {
  struct extArg **args;
  int numargs;
  char *url;
  char *callback;
} extRequest;

/*
 * main.c
 */
int cgiMain(void);

/*
 * debug.c
 */
void debuglog(char *msg);
FILE *logptr();
void printParams(struct extRequest *req);

/*
 * error.c
 */
void exitStatus(int status, char *msg);

/*
 * request.c
 */
int countArguments(void);
int initReq(struct extRequest *req);
int parseArguments(struct extRequest *req);
int parseCallback(struct extRequest *req);
int parseURL(struct extRequest *req);
struct extArg * makeArg(char *name, char *value);
void freeReq(struct extRequest *req);

/*
 * fetch.c
 */
size_t write_function( void *ptr, size_t size, size_t nmemb, void *stream);
int doPostReq(struct extRequest *req);
