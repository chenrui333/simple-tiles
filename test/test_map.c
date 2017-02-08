#include "test.h"
#include "map.h"

static void close_enough(float number, float test) {
  assert((number - test) < 0.001);
}

static void test_resetting() {
  simplet_map_t *map;
  assert((map = simplet_map_new()));
  simplet_map_set_bounds(map, 10, 10, 0, 0);
  assert(map->bounds->se.x == 10);
  assert(map->bounds->nw.y == 10);
  assert(map->bounds->nw.x == 0);
  assert(map->bounds->se.y == 0);
  simplet_map_set_bounds(map, 0, 0, -10, -10);
  assert(map->bounds->se.x == 0);
  assert(map->bounds->nw.y == 0);
  assert(map->bounds->nw.x == -10);
  assert(map->bounds->se.y == -10);
  simplet_map_free(map);
}

static void test_map() {
  simplet_map_t *map;
  assert((map = simplet_map_new()));
  assert(map->bounds->nw.x);
  simplet_map_set_srs(map,
                      "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 "
                      "+x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null "
                      "+no_defs");
  assert(map->proj);
  simplet_map_set_size(map, 256, 256);
  assert(simplet_map_get_width(map) == 256);
  assert(simplet_map_get_height(map) == 256);
  simplet_map_set_bounds(map, -179.231086, 17.831509, -100.859681, 71.441059);
  assert(map->bounds);
  simplet_map_free(map);
}

static void test_proj() {
  const char *srs =
      "+proj=longlat +ellps=GRS80 +towgs84=0,0,0,0,0,0,0 +no_defs ";
  simplet_map_t *map;
  assert((map = simplet_map_new()));
  simplet_map_set_srs(map, srs);
  char *test = NULL;
  simplet_map_get_srs(map, &test);
  assert(!strcmp(srs, test));
  free(test);
  simplet_map_free(map);
}

static void test_slippy() {
  simplet_map_t *map;
  assert((map = simplet_map_new()));
  assert(simplet_map_set_slippy(map, 0, 0, 1));
  assert(map->bounds);
  close_enough(map->bounds->nw.x, -20037508.34);
  close_enough(map->bounds->nw.y, 20037508.34);
  close_enough(map->bounds->se.y, 0.0);
  close_enough(map->bounds->se.x, 0.0);
  simplet_map_free(map);
}

static void test_user_data() {
  simplet_map_t *map;
  assert((map = simplet_map_new()));
  int i = 5;
  simplet_map_set_user_data(map, &i);
  assert(*(int *)simplet_map_get_user_data(map) == i);
  simplet_map_free(map);
}

TASK(map) {
  test(resetting);
  test(map);
  test(proj);
  test(slippy);
  test(user_data);
}
