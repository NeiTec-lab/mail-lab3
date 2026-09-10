#include "flight_data.h"

#include <stdio.h>

int main(int argc, char *argv[]) {
    const char *input_path = "data/flight_data6.bin";
    const char *output_path = "output/statistics.txt";
    FlightList list;
    RouteStatistics statistics;

    if (argc > 2) {
        fprintf(stderr, "Использование: %s [путь_к_бинарному_файлу]\n", argv[0]);
        return 1;
    }
    if (argc == 2) {
        input_path = argv[1];
    } else {
        printf("Путь не задан: используется файл по умолчанию %s\n", input_path);
    }

    list_init(&list);
    printf("Открытие файла: %s\n", input_path);
    if (!read_flight_file(input_path, &list)) {
        return 1;
    }

    printf("Прочитано записей: %zu\n", list.count);
    printf("Первые 10 элементов односвязного списка:\n");
    list_print_first(&list, 10);

    if (!build_statistics(&list, &statistics)) {
        list_free(&list);
        return 1;
    }

    printf("\nСтатистика маршрута:\n");
    printf("Количество точек: %zu\n", statistics.point_count);
    printf("Продолжительность полёта: %.2f с (%.2f мин)\n",
           statistics.duration_sec, statistics.duration_sec / 60.0f);
    printf("Максимальная высота: %.2f м\n", statistics.max_height_m);
    printf("Минимальная высота: %.2f м\n", statistics.min_height_m);

    if (!write_statistics(output_path, &statistics)) {
        list_free(&list);
        return 1;
    }
    printf("Статистика сохранена: %s\n", output_path);

    list_free(&list);
    return 0;
}
