/* fastest-primes: segmented Sieve of Eratosthenes in C.
 * Base primes up to sqrt(N) (<= 2237) from a tiny odd sieve, then the
 * range [3, N] is swept in 64 KiB segments of odd numbers only (one byte
 * per odd). Each segment fits in L2, so marking is cache-hot; the whole
 * 5,000,000 range needs ~40 segments. No heap, no stdio, fully static.
 * Compile: gcc -O3 -march=native -static -o primes primes.c
 * Run: echo 5000000 | ./primes  -> 348513
 */
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#define MAXN 5000000
#define SEG (1 << 15) /* odds per segment */

static unsigned char seg[SEG];
static int base[512];
static int nbase;

static void base_sieve(int lim) {
    static unsigned char c[2238];
    for (int i = 3; i * i <= lim; i += 2)
        if (!c[i])
            for (int j = i * i; j <= lim; j += 2 * i) c[j] = 1;
    nbase = 0;
    for (int i = 3; i <= lim; i += 2)
        if (!c[i]) base[nbase++] = i;
}

int main(void) {
    char in[32];
    long n = 0;
    ssize_t r = read(0, in, sizeof in);
    for (ssize_t k = 0; k < r; k++)
        if (in[k] >= '0' && in[k] <= '9') n = n * 10 + (in[k] - '0');
    if (n < 2) { if (write(1, "0\n", 2) != 2) return 1; return 0; }

    int lim = 1;
    while ((long)(lim + 1) * (lim + 1) <= n) lim++;
    base_sieve(lim);

    long cnt = 1; /* prime 2 */
    for (long low = 3; low <= n; low += 2 * SEG) {
        long high = low + 2 * SEG;
        if (high > n + 1) high = n + 1; /* exclusive, odd-aligned */
        long m = (high - low + 1) / 2;  /* odds in this segment */
        memset(seg, 0, m);
        for (int b = 0; b < nbase; b++) {
            long p = base[b];
            if (p * p >= high) break; /* larger p mark nothing in this segment */
            long start = p * p;
            if (start < low) {
                start = low + (p - low % p) % p;
                if ((start & 1) == 0) start += p;
            }
            /* index k of odd j: k = (j-low)/2; j += 2p  =>  k += p */
            for (long k = (start - low) / 2; k < m; k += p) seg[k] = 1;
        }
        for (long k = 0; k < m; k++) if (!seg[k]) cnt++;
    }

    char out[24];
    int len = 0;
    long t = cnt;
    char rev[20];
    int rl = 0;
    do { rev[rl++] = '0' + (t % 10); t /= 10; } while (t);
    while (rl) out[len++] = rev[--rl];
    out[len++] = '\n';
    if (write(1, out, len) != len) return 1;
    return 0;
}
