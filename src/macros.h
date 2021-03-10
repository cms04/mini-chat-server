#ifndef MACROS_H
#define MACROS_H

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#define PRINT_ERROR(function) fprintf(stderr, "ERROR: %s() failed: %s\n", function, strerror(errno)); \
                              return EXIT_FAILURE

#endif
