#ifndef _SIMPLE_TILES_TEST_H
#define _SIMPLE_TILES_TEST_H

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "simple_tiles.h"

// simple testing macro that prints the name of the test
// and a cute checkmark if it passed
#define test(fn)                     \
  printf("\x1b[33m" #fn "\x1b[0m "); \
  test_##fn();                       \
  puts("\x1b[1;32m✓ \x1b[0m");

// Proto definition.
#define TASK(name) void run_task_##name()

// List of tasks to run.
TASK(list);
TASK(vector_layer);
TASK(raster_layer);
TASK(query);
TASK(style);
TASK(map);
TASK(integration);
TASK(bounds);

#endif
