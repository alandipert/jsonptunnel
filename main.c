/*
 * main.c
 *
 * Use libcgic to parse a GET request
 * and construct an extRequest struct.
 * Pass the struct to the curl functions
 * and print the result of the remote 
 * HTTP action to cgiOut (stdout).
 */

#include <fcgi_stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cgic/cgic.h"
#include "jsonptunnel.h"

/*
 * The main entrypoint function.
 * libcgic takes care of an actual
 * main().
 */
int cgiMain(void) {

  /* This struct will contain everything about the request. */
  struct extRequest req;

  /* Attempt to load the request with the right information */
  if(!initReq(&req)) {
    freeReq(&req);
    return EXIT_FAILURE;
  }

  /* 
   * Print the passed variables and arguments. 
   * For debugging purposes - maybe this should be an extParam?
   */
  //printParams(&req);

  /* 
   * Requests the URL with curl and appends the extCallback
   * function to the end of the output.
   */
  if(!doPostReq(&req)) {
    freeReq(&req);
    exitStatus(500, "There was an error fetching the remote data.");
    return EXIT_FAILURE;
  }

  /* Free all the strings malloced inside of req */
  freeReq(&req);

  /* Clean exit */
  return EXIT_SUCCESS;

}

