#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

/*
 * The directory to store cached objects.
 */
#define CACHE_DIR "cache"

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
#define CACHE_PARAM_NAME "extCache"

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
  unsigned long hash;
} extRequest;

/*
 * jsonptunnel.c
 */
int cgiMain(void);
int countArguments(void);
int initReq(struct extRequest *req); 
int parseArguments(struct extRequest *req);
int parseCache(struct extRequest *req); 
int parseMethod(struct extRequest *req);
int parseURL(struct extRequest *req);
void exitStatus(int status, char *msg);
void freeReq(struct extRequest *req); 
void printParams(struct extRequest *req);

/*
 * curl.c
 */
char * buildQueryString(CURL *curl, struct extRequest *req);
int doFetch(struct extRequest *req);
int doGetReq(struct extRequest *req, FILE *outputStream); 
int doPostReq(struct extRequest *req, FILE *outputStream); 

/*
 * cache.c
 */
char * get_cached_filename(struct extRequest *req);
unsigned long hash_str(const char *str);
