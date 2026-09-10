#include "flight_data.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint32_t timestamp_ms;
    float lat_rad;
    float lon_rad;
    float alt_m;
} BinaryRecord;

static const float RAD_TO_DEG = 57.29577951308232f;

void list_init(FlightList *list) {
    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
}

int list_append(FlightList *list, FlightData data) {
    FlightNode *node = malloc(sizeof(*node));
    if (node == NULL) {
        return 0;
    }
    node->data = data;
    node->next = NULL;
    if (list->tail == NULL) {
        list->head = node;
    } else {
        list->tail->next = node;
    }
    list->tail = node;
    list->count++;
    return 1;
}

void list_print_first(const FlightList *list, size_t limit) {
    const FlightNode *current = list->head;
    size_t index = 0;
    while (current != NULL && index < limit) {
        printf("  [%zu] time=%u ms, lat=%.6f deg, lon=%.6f deg, alt=%.2f m\n",
               index, current->data.timestamp_ms, current->data.lat_deg,
               current->data.lon_deg, current->data.alt_m);
        current = current->next;
        index++;
    }
}

void list_free(FlightList *list) {
    FlightNode *current = list->head;
    while (current != NULL) {
        FlightNode *next = current->next;
        free(current);
        current = next;
    }
    list_init(list);
}

int read_flight_file(const char *path, FlightList *list) {
    FILE *file = fopen(path, "rb");
    BinaryRecord record;

    if (file == NULL) {
        fprintf(stderr, "Ошибка открытия файла '%s': %s\n", path, strerror(errno));
        return 0;
    }

    while (fread(&record, sizeof(record), 1, file) == 1) {
        FlightData data = {
            record.timestamp_ms,
            record.lat_rad * RAD_TO_DEG,
            record.lon_rad * RAD_TO_DEG,
            record.alt_m
        };
        if (!list_append(list, data)) {
            fprintf(stderr, "Ошибка: не удалось выделить память для элемента списка.\n");
            fclose(file);
            list_free(list);
            return 0;
        }
    }

    long file_position = ftell(file);
    if (ferror(file) || file_position < 0) {
        fprintf(stderr, "Ошибка чтения файла '%s'.\n", path);
        fclose(file);
        list_free(list);
        return 0;
    }
    if (file_position % (long)sizeof(record) != 0) {
        fprintf(stderr, "Ошибка: файл содержит неполную запись.\n");
        fclose(file);
        list_free(list);
        return 0;
    }
    fclose(file);
    return list->count > 0;
}

HeightNode *height_node_create(float height) {
    HeightNode *node = malloc(sizeof(*node));
    if (node != NULL) {
        node->height = height;
        node->left = NULL;
        node->right = NULL;
    }
    return node;
}

int height_tree_insert(HeightNode **root, float height) {
    if (*root == NULL) {
        *root = height_node_create(height);
        return *root != NULL;
    }
    if (height < (*root)->height) {
        return height_tree_insert(&(*root)->left, height);
    }
    if (height > (*root)->height) {
        return height_tree_insert(&(*root)->right, height);
    }
    return 1; /* Точная повторяющаяся высота уже представлена в дереве. */
}

float height_tree_min(const HeightNode *root) {
    while (root->left != NULL) {
        root = root->left;
    }
    return root->height;
}

float height_tree_max(const HeightNode *root) {
    while (root->right != NULL) {
        root = root->right;
    }
    return root->height;
}

void height_tree_free(HeightNode *root) {
    if (root == NULL) {
        return;
    }
    height_tree_free(root->left);
    height_tree_free(root->right);
    free(root);
}

int build_statistics(const FlightList *list, RouteStatistics *statistics) {
    const FlightNode *current = list->head;
    HeightNode *tree = NULL;

    if (current == NULL) {
        return 0;
    }
    statistics->point_count = list->count;
    statistics->first_timestamp_ms = current->data.timestamp_ms;

    while (current != NULL) {
        if (!height_tree_insert(&tree, current->data.alt_m)) {
            fprintf(stderr, "Ошибка: не удалось выделить память для дерева высот.\n");
            height_tree_free(tree);
            return 0;
        }
        statistics->last_timestamp_ms = current->data.timestamp_ms;
        current = current->next;
    }
    statistics->duration_sec =
        (statistics->last_timestamp_ms - statistics->first_timestamp_ms) / 1000.0f;
    statistics->min_height_m = height_tree_min(tree);
    statistics->max_height_m = height_tree_max(tree);
    height_tree_free(tree);
    return 1;
}

int write_statistics(const char *path, const RouteStatistics *statistics) {
    FILE *file = fopen(path, "w");
    if (file == NULL) {
        fprintf(stderr, "Ошибка создания файла '%s': %s\n", path, strerror(errno));
        return 0;
    }
    fprintf(file, "Количество точек маршрута: %zu\n", statistics->point_count);
    fprintf(file, "Продолжительность полёта: %.2f с (%.2f мин)\n",
            statistics->duration_sec, statistics->duration_sec / 60.0f);
    fprintf(file, "Максимальная высота: %.2f м\n", statistics->max_height_m);
    fprintf(file, "Минимальная высота: %.2f м\n", statistics->min_height_m);
    if (fclose(file) != 0) {
        fprintf(stderr, "Ошибка закрытия файла '%s'.\n", path);
        return 0;
    }
    return 1;
}
