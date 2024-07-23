#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/time.h>
#include "lib/eviction_set/sys_utils.h"
#include "lib/basics/math_utils.h"

const int TEST_SIZE = 100;
const int ARRAY_SIZE = 1e3 + 5;
const int L2_MISS_THRESHOLD = 300;

uint64_t* arr[ARRAY_SIZE];
uint64_t access_time, miss_time, trash, latency;

void sink(int index, int length) {
    int leftChild = 2 * index + 1;
    int rightChild = 2 * index + 2;
    int present = index;

    if (leftChild < length) {
        access_time += 2;

        mem_access(arr[leftChild], trash);
        trash = busy_wait(100, trash);
        kpc_time_load(arr[leftChild], latency, trash);
        if (latency > L2_MISS_THRESHOLD) miss_time++;
        
        mem_access(arr[rightChild], trash);
        trash = busy_wait(100, trash);
        kpc_time_load(arr[rightChild], latency, trash);
        if (latency > L2_MISS_THRESHOLD) miss_time++;

        if (*arr[leftChild] > *arr[present]) present = leftChild;
    }

    if (rightChild < length) {
        access_time += 2;

        mem_access(arr[leftChild], trash);
        trash = busy_wait(100, trash);
        kpc_time_load(arr[leftChild], latency, trash);
        if (latency > L2_MISS_THRESHOLD) miss_time++;
        
        mem_access(arr[rightChild], trash);
        trash = busy_wait(100, trash);
        kpc_time_load(arr[rightChild], latency, trash);
        if (latency > L2_MISS_THRESHOLD) miss_time++;

        if (*arr[rightChild] > *arr[present]) present = rightChild;
    }

    if (present != index) {
        access_time += 2;

        mem_access(arr[index], trash);
        trash = busy_wait(100, trash);
        kpc_time_load(arr[index], latency, trash);
        if (latency > L2_MISS_THRESHOLD) miss_time++;
        
        mem_access(arr[present], trash);
        trash = busy_wait(100, trash);
        kpc_time_load(arr[present], latency, trash);
        if (latency > L2_MISS_THRESHOLD) miss_time++;

        uint64_t* temp = arr[index];
        arr[index] = arr[present];
        arr[present] = temp;
        sink(present, length);
    }
}

void buildHeap(int length) {
    for (int i = length / 2; i >= 0; i--) {
        sink(i, length);
    }
}

void heap_sort(int length) {
    buildHeap(length);
    for (int i = length - 1; i > 0; i-- ) {
        access_time += 2;

        mem_access(arr[0], trash);
        trash = busy_wait(100, trash);
        kpc_time_load(arr[0], latency, trash);
        if (latency > L2_MISS_THRESHOLD) miss_time++;
        
        mem_access(arr[i], trash);
        trash = busy_wait(100, trash);
        kpc_time_load(arr[i], latency, trash);
        if (latency > L2_MISS_THRESHOLD) miss_time++;

        uint64_t* temp = arr[0];
        arr[0] = arr[i];
        arr[i] = temp;
        length--;
        sink(0, length);
    }
}

void generate_ptr() {
    for(int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = (uint64_t*)malloc(sizeof(uint64_t));
        *arr[i] = rand();
    }
}

int main(int argc, char *argv[])
{
    pin_cpu(atoi(argv[1]));
    init_kpc();
    configure_kpc();
    srand(time(NULL));
    struct timespec start, end;
    float miss_rate[TEST_SIZE];
    uint64_t runtime[TEST_SIZE];

    trash = busy_wait(100, trash);
    for(int i = 0; i < TEST_SIZE; i++) {
        generate_ptr();
        access_time = 0;
        miss_time = 0;
        clock_gettime(CLOCK_MONOTONIC_RAW, &start);
        heap_sort(ARRAY_SIZE);
        clock_gettime(CLOCK_MONOTONIC_RAW, &end);
        uint64_t delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
        miss_rate[i] = (float)miss_time / access_time;
        runtime[i] = delta_us;
    }
    
    float avg_miss_rate = mean_f(miss_rate, TEST_SIZE);
    uint64_t avg_runtime = mean_8B(runtime, TEST_SIZE);
    printf("Average runtime: %llu us\n", avg_runtime);
    printf("Average miss rate: %f\n", avg_miss_rate);

    return EXIT_SUCCESS;
}
