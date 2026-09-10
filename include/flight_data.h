#ifndef FLIGHT_DATA_H
#define FLIGHT_DATA_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t timestamp_ms;
    float lat_deg;
    float lon_deg;
    float alt_m;
} FlightData;

typedef struct FlightNode {
    FlightData data;
    struct FlightNode *next;
} FlightNode;

typedef struct {
    FlightNode *head;
    FlightNode *tail;
    size_t count;
} FlightList;

typedef struct HeightNode {
    float height;
    struct HeightNode *left;
    struct HeightNode *right;
} HeightNode;

typedef struct {
    size_t point_count;
    uint32_t first_timestamp_ms;
    uint32_t last_timestamp_ms;
    float duration_sec;
    float min_height_m;
    float max_height_m;
} RouteStatistics;

void list_init(FlightList *list);
int list_append(FlightList *list, FlightData data);
void list_print_first(const FlightList *list, size_t limit);
void list_free(FlightList *list);

int read_flight_file(const char *path, FlightList *list);

HeightNode *height_node_create(float height);
int height_tree_insert(HeightNode **root, float height);
float height_tree_min(const HeightNode *root);
float height_tree_max(const HeightNode *root);
void height_tree_free(HeightNode *root);

int build_statistics(const FlightList *list, RouteStatistics *statistics);
int write_statistics(const char *path, const RouteStatistics *statistics);

#endif
