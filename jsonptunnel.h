#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

/*
 * Supported HTTP methods.
 */
#define METHOD_GET 0
#define METHOD_POST 1

/*
 * Required parameters.
 */
#define URL_PARAM_NAME "extURL"
#define METHOD_PARAM_NAME "extMethod"

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
  int method;
  char *url;
  char *callback;
} extRequest;

/*
 * jsonptunnel.c
 */
void printParams(struct extRequest *req);
void exitStatus(int status, char *msg);
int parseMethod(struct extRequest *req);
int countArguments(void);
int parseArguments(struct extRequest *req);
int parseURL(struct extRequest *req);
void freeReq(struct extRequest *req); 
int initReq(struct extRequest *req); 
int cgiMain(void);

/*
 * curl.c
 */
char * buildQueryString(CURL *curl, struct extRequest *req);
int doFetch(struct extRequest *req);
int doPostReq(struct extRequest *req); 
int doGetReq(struct extRequest *req); 
