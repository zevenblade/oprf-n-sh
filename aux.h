#ifndef AUX_H
#define AUX_H

#include "arith.h"
#include <stdio.h>

void write_elm_arr(char fileString[], void *arr, int size);

void write_int_arr(char fileString[], void *arr, int size);

void read_elm_arr(char fileString[], void *arr, int size);

void read_int_arr(char fileString[], void *arr, int size);

#endif