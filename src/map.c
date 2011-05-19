#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

#include "map.h"
#include "layer.h"
#include "rule.h"
#include "style.h"
#include "util.h"

simplet_map_t*
simplet_map_new(){
  simplet_map_t *map;
  if(!(map = malloc(sizeof(*map))))
    return NULL;
  
  if(!(map->layers = simplet_list_new())){
    free(map);
    return NULL;
  }
  
  map->bounds = NULL;
  map->proj   = NULL;
  map->_ctx   = NULL;
  map->height = 0;
  map->width  = 0;
  map->valid  = MAP_OK;
  return map;
}

void
simplet_map_free(simplet_map_t *map){
  if(map->bounds)
    simplet_bounds_free(map->bounds);
  if(map->layers) {
    map->layers->free = simplet_layer_vfree;
    simplet_list_free(map->layers);
  }
  if(map->proj)
    OSRRelease(map->proj);
  free(map);
}

int
simplet_map_set_srs(simplet_map_t *map, char *proj){
  assert(map->valid == MAP_OK);

  if(!(map->proj = OSRNewSpatialReference(NULL)))
    return (map->valid = MAP_ERR);

  if(OSRSetFromUserInput(map->proj, proj) != OGRERR_NONE)
    return (map->valid = MAP_ERR);

  return MAP_OK;
}

int
simplet_map_set_size(simplet_map_t *map, int width, int height){
  assert(map->valid == MAP_OK);

  map->height = height;
  map->width  = width;
  return MAP_OK;
}

int
simplet_map_set_bounds(simplet_map_t *map, double maxx, double maxy, double minx, double miny){
  assert(map->valid == MAP_OK);
  if(!(map->bounds = simplet_bounds_new()))
    return (map->valid = MAP_ERR);
  simplet_bounds_extend(map->bounds, maxx, maxy);
  simplet_bounds_extend(map->bounds, minx, miny);
  return MAP_OK;
}

simplet_layer_t*
simplet_map_add_layer(simplet_map_t *map, char *datastring){
  assert(map->valid == MAP_OK);

  simplet_layer_t *layer;
  if(!(layer = simplet_layer_new(datastring))){
    map->valid = MAP_ERR;
    return NULL;
  }

  if(!simplet_list_push(map->layers, layer)){
    map->valid = MAP_ERR;
    simplet_layer_free(layer);
    return NULL;
  }
    
  return layer;
}

simplet_rule_t*
simplet_map_add_rule(simplet_map_t *map, char *sqlquery){
  assert(map->valid == MAP_OK);

  simplet_rule_t *rule;
  if(!(rule = simplet_rule_new(sqlquery))){
    map->valid = MAP_ERR;
    return NULL;
  }
  
  simplet_layer_t *layer = map->layers->tail->value;

  if(!layer){
    map->valid = MAP_ERR;
    return NULL;
  }

  if(!simplet_list_push(layer->rules, rule)){
    map->valid = MAP_ERR;
    simplet_rule_free(rule);
    return NULL;
  }

  return rule;
}

simplet_style_t *
simplet_map_add_style(simplet_map_t *map, char *key, char *arg){
  assert(map->valid == MAP_OK);

  if(!map->layers->tail){
    map->valid = MAP_ERR;
    return NULL;
  }
  simplet_layer_t *layer = map->layers->tail->value;
  
  if(!layer){
    map->valid = MAP_ERR;
    return NULL;
  }
  
  simplet_rule_t *rule = layer->rules->tail->value;
  
  if(!rule){
    map->valid = MAP_ERR;
    return NULL;
  }

  simplet_style_t *style;
  if(!(style = simplet_rule_add_style(rule, key, arg))){
    map->valid = MAP_ERR;
    return NULL;
  }

  return style;
}

int
simplet_map_isvalid(simplet_map_t *map){
  assert(map->valid == MAP_OK);

  if(map->valid == MAP_ERR)
    return MAP_ERR;

  if(!map->bounds)
    return MAP_ERR;

  if(!map->proj)
    return MAP_ERR;

  if(!map->height)
    return MAP_ERR;

  if(!map->width)
    return MAP_ERR;

  if(!map->layers->tail)
    return MAP_ERR;

  return MAP_OK;
}

int
simplet_map_render_to_png(simplet_map_t *map, char *path){
  if(simplet_map_isvalid(map) == MAP_ERR)
    return (map->valid = MAP_ERR);
    
  cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, map->width, map->height);
  if(cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS)
    return (map->valid = MAP_ERR);
  cairo_t *ctx = cairo_create(surface);
  map->_ctx = ctx;
  cairo_scale(map->_ctx, map->width / map->bounds->width, map->width / map->bounds->width);
  simplet_listiter_t *iter = simplet_get_list_iter(map->layers);
  simplet_layer_t *layer;
  while((layer = simplet_list_next(iter)))
    simplet_layer_process(layer, map);
  /* rewind datasource */
  cairo_surface_write_to_png(surface, path);
  cairo_destroy(map->_ctx);
  map->_ctx = NULL;
  return MAP_OK;
}