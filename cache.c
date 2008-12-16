/*
 * This file handles getting and putting cached objects,
 * identified by the filename of the FNV hash
 * of the query string.
 */

#include <sys/types.h>
#include <sys/param.h>
#include <dirent.h>
//#include <stdio.h>
#include <fcgi_stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cgic/cgic.h"
#include "jsonptunnel.h"

/*
 * Check for a cached version of this request,
 * and if it exists, return its filename. 
 * If it doesn't exist, return NULL.
 */
char * get_cached_filename(struct extRequest *req) {

  /* A list of directory entries. */
  struct dirent **namelist;

  /* The number of directory entries returned by scandir() */
  int num_entries;

  /* The name of the cached object we're looking for */
  char cache_name[MAXPATHLEN];

  /* The filename to return on success */
  char *filename = NULL;

  /* Find the number of files in the cache directory */
  num_entries = scandir(CACHE_DIR, &namelist, NULL, NULL);

  /* If there are none, return NULL */
  if(num_entries == 0) {
    return NULL;
  }

  /* Use sprintf to make a string out of our hash. */
  sprintf(cache_name, "%lu", req->hash);

  /*
   * Iterate over the filenames in the cache
   * directory, looking for a match.
   */
  int i;
  for(i = 0; i < num_entries; i++) {

    if(strcmp(namelist[i]->d_name, &cache_name[0]) == 0) {
      filename = (char*)malloc(sizeof(char)*(strlen(namelist[i]->d_name)+1));
      strcpy(filename, namelist[i]->d_name);
    }

    free(namelist[i]);
  }

  if(num_entries > 0) {
    free(namelist);
  }

  char *fullpath = NULL;

  if(filename != NULL) {

    /* Make a string for the cache dir; filename gets appended. */
    char *cachedir = (char*)malloc(sizeof(char)*(strlen(CACHE_DIR)+1));
    cachedir = strcpy(cachedir, CACHE_DIR);

    /* Append filename to the cachedir path */
    fullpath = (char*)malloc(sizeof(char)*(strlen(filename)+strlen(cachedir)+1));
    strcpy(fullpath, cachedir);
    strcat(fullpath, filename);

    free(cachedir);
    free(filename);
  }

  return fullpath;

}

/* 
 * Crude implementation of 32-bit FNV-1 hash algorithm,
 * see http://www.isthe.com/chongo/tech/comp/fnv/ for details
 * about the magic numbers.
 *
 * From cgit: http://hjemli.net/git/cgit/
 * See also: http://en.wikipedia.org/wiki/Fowler_Noll_Vo_hash
 */

#define FNV_OFFSET 0x811c9dc5
#define FNV_PRIME  0x01000193

unsigned long hash_str(const char *str) {

	unsigned long h = FNV_OFFSET;
	unsigned char *s = (unsigned char *)str;

	if (!s)
		return h;

	while(*s) {
		h *= FNV_PRIME;
		h ^= *s++;
	}
	return h;
}
