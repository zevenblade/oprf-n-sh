#include "aux.h"

void write_elm_arr(char fileString[], void *arr, int size)
{
    FILE *file = fopen(fileString, "wb");
    if (file == NULL)
    {
        perror("Error opening file");
    }
    fwrite(arr, sizeof(f_elm_t), size, file);
    fclose(file);
}

void write_int_arr(char fileString[], void *arr, int size)
{
    FILE *file = fopen(fileString, "wb");
    if (file == NULL)
    {
        perror("Error opening file");
    }
    fwrite(arr, sizeof(int), size, file);
    fclose(file);
}

void read_elm_arr(char fileString[], void *arr, int size)
{
    FILE *file = fopen(fileString, "rb");
    if (file == NULL)
    {
        perror("Error opening file");
    }
    size_t read = fread(arr, sizeof(f_elm_t), size, file);
    fclose(file);
}

void read_int_arr(char fileString[], void *arr, int size)
{
    FILE *file = fopen(fileString, "rb");
    if (file == NULL)
    {
        perror("Error opening file");
    }
    size_t read = fread(arr, sizeof(int), size, file);
    fclose(file);
}