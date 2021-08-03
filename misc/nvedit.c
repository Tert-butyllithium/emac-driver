// #include <string.h>

#include "../include/common.h"

static char keys[20][20];
static char vals[20][20];

int env_set(const char* name, const char* val) {
  int idx = 0;
  printf("env_set: name: %s, val: %s\n", name, val ? val : "NULL");
  for (; idx < 19; idx++) {
    if (keys[idx][0]) {
      if (strcmp(name, keys[idx]) == 0) {
        break;
      }
    }
  }
  strcpy(keys[idx], name);
  strcpy(vals[idx], val);
  return 0;
}


char* env_get(const char* name) {
  int idx = 0;
  for (; idx < 20; idx++) {
    if (keys[idx][0]) {
      if (strcmp(name, keys[idx]) == 0) {
        printf("env_get: name: %s, val: %s\n", name, vals[idx]);
        return vals[idx];
      }
    }
  }
  return NULL;
}