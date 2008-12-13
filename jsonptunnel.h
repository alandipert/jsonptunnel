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
#define CALLBACK_PARAM_NAME "callBack"

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
  char *url;
  struct extArg **args;
  int numargs;
  int method;
} extRequest;

/*
 * jsonptunnel.c
 */
void printParams(struct extRequest *req);
void exit400(char *msg);
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
void postReq(struct extRequest *req); 
