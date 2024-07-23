#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <sys/mman.h>
#include <sys/time.h>
#include "lib/basics/math_utils.h"
#include "lib/eviction_set/sys_utils.h"

const int TEST_SIZE = 100;
const int ARRAY_SIZE = 1e6 + 5;

uint64_t* arr[ARRAY_SIZE * 2 + 2];

void generate_array() {
    for(int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = (uint64_t*) malloc(sizeof(uint64_t));
        arr[i + ARRAY_SIZE] = (uint64_t*)malloc(sizeof(uint64_t));
        *arr[i] = rand();
        *arr[i + ARRAY_SIZE] = rand();
    }
}

void free_array() {
    for(int i = 0; i < ARRAY_SIZE; i++) {
        free(arr[i]);
        free(arr[i + ARRAY_SIZE]);
    }
}

void swap(uint64_t secret, size_t len) {
    uint64_t mask = ~(secret - 1);
    uint64_t delta;
    for(int i = 0; i < len; i++) {
        delta = (*arr[i] ^ *arr[i + ARRAY_SIZE]) & mask;
        *arr[i] = *arr[i] ^ delta;
        *arr[i + ARRAY_SIZE] = *arr[i + ARRAY_SIZE] ^ delta;
    }
}

void blind_swap(uint64_t secret, size_t len) {
    uint64_t mask = ~(secret - 1);
    // Blind Array
    for(int i = 0; i < len; i++) {
        *arr[i] = *arr[i] ^ mask;
        *arr[i + ARRAY_SIZE] = *arr[i + ARRAY_SIZE] ^ mask;
    }

    // Swap
    swap(secret, len);

    // Unblind Array
    for(int i = 0; i < len; i++) {
        *arr[i] = *arr[i] ^ mask;
        *arr[i + ARRAY_SIZE] = *arr[i + ARRAY_SIZE] ^ mask;
    }
}

uint64_t pow_mod(uint64_t base, uint64_t exp, uint64_t mod) {
    uint64_t result = 1;
    base = base % mod;
    while(exp > 0) {
        if(exp % 2 == 1) {
            result = (result * base) % mod;
        }
        exp = exp >> 1;
        base = (base * base) % mod;
    }
    return result;
}

// calculate inverse of a mod n using extended euclidean algorithm
uint64_t exgcd(uint64_t a,uint64_t b,uint64_t *x,uint64_t *y)
{
	if(b==0)
	{
		*x = 1, *y = 0;
		return a;
	}
	uint64_t ret=exgcd(b,a%b,y,x);
	y -=a / b * (*x);
	return ret;
}

uint64_t inv(int a,int mod) {
	uint64_t x,y;
	uint64_t d=exgcd(a,mod, &x, &y);
	return d == 1?(x%mod+mod)%mod:-1;
}

uint64_t gcd(uint64_t a, uint64_t b) {
    if(b == 0) {
        return a;
    }
    return gcd(b, a % b);
}

uint64_t rsa_decrypt(uint64_t p, uint64_t q, uint64_t d, uint64_t c) {
    uint64_t n = p * q;
    uint64_t m1 = pow_mod(c % p, d % (p - 1), p);
    uint64_t m2 = pow_mod(c % q, d % (q - 1), q);
    uint64_t h = (m1 - m2) * inv(q, p) % p;
    uint64_t m = (m2 + h * q) % n;
    return m;
}

void rsa_decrypt_masked(uint64_t p, uint64_t q, uint64_t d, uint64_t c, \
    uint64_t mask, uint64_t inverse) {

    uint64_t phi = (p - 1) * (q - 1);
    c = pow_mod(c, mask, p * q);
    uint64_t m = rsa_decrypt(p, q, d, c);
    m = rsa_decrypt(p, q, inverse, m);
}

int main(int argc, char *argv[])
{
    pin_cpu(4);
    srand(time(NULL));
    struct timespec start, end;
    uint64_t runtime_blind[TEST_SIZE], runtime_unblind[TEST_SIZE], \
        runtime_rsa[TEST_SIZE], runtime_rsa_masked[TEST_SIZE];

    for(int i = 0; i < TEST_SIZE; i++) {
        generate_array();

        // Blind
        clock_gettime(CLOCK_MONOTONIC_RAW, &start);
        blind_swap(1, ARRAY_SIZE); 
        clock_gettime(CLOCK_MONOTONIC_RAW, &end);
        runtime_blind[i] = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;

        // Unblind
        clock_gettime(CLOCK_MONOTONIC_RAW, &start);
        swap(1, ARRAY_SIZE);
        clock_gettime(CLOCK_MONOTONIC_RAW, &end);
        runtime_unblind[i] = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;

        free_array();

        // RSA
        uint64_t p = 65537, q = 65539;
        uint64_t mask = 0, phi = (p - 1) * (q - 1);

        clock_gettime(CLOCK_MONOTONIC_RAW, &start);
        for(int j = 0; j < 100000; j++)
            rsa_decrypt(65537, 65539, 0x10001, 0x12345678);
        clock_gettime(CLOCK_MONOTONIC_RAW, &end);
        runtime_rsa[i] = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
        
        while(gcd(mask, phi) != 1) mask = rand();

        // RSA Masked
        clock_gettime(CLOCK_MONOTONIC_RAW, &start);
        for(int j = 0; j < 100000; j++)
            rsa_decrypt_masked(p, q, 0x10001, 0x12345678, mask, inv(mask, phi));
        clock_gettime(CLOCK_MONOTONIC_RAW, &end);
        runtime_rsa_masked[i] = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
    }
    
    uint64_t avg_runtime = mean_8B(runtime_blind, TEST_SIZE);
    printf("Average Blind runtime: %llu us\n", avg_runtime);
    avg_runtime = mean_8B(runtime_unblind, TEST_SIZE);
    printf("Average Unblind runtime: %llu us\n", avg_runtime);
    avg_runtime = mean_8B(runtime_rsa, TEST_SIZE);
    printf("Average RSA runtime: %llu us\n", avg_runtime);
    avg_runtime = mean_8B(runtime_rsa_masked, TEST_SIZE);
    printf("Average RSA Masked runtime: %llu us\n", avg_runtime);

    return EXIT_SUCCESS;
}
