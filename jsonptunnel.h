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
