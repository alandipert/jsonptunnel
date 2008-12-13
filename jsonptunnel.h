#define METHOD_GET 0
#define METHOD_POST 1

#define URL_PARAM_NAME "extURL"
#define METHOD_PARAM_NAME "extMethod"

struct extArg {
  char *argName;
  char *argVal;
} extArg;

struct extRequest {
  char *url;
  struct extArg **args;
  int numargs;
  int method;
} extRequest;

//jsonptunnel.c
void printParams(struct extRequest *req);
void exit500(char *msg);
int parseMethod(struct extRequest *req);
int countArguments(void);
int parseArguments(struct extRequest *req);
int parseURL(struct extRequest *req);
void freeReq(struct extRequest *req); 
int cgiMain(void);

//curl.c
void postReq(struct extRequest *req); 
