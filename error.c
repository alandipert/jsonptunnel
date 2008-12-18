#include <stdio.h>
#include "cgic/cgic.h"
#include "jsonptunnel.h"

/*
 * Send a '400' 'Bad Request' HTTP error code and exit.
 */
void exitStatus(int status, char *msg) {
  cgiHeaderStatus(status, msg);
  fprintf(cgiOut, "There was an error processing your request: %s\n", msg);
}
