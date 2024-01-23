
// BENCHMARKING

static inline void delay(unsigned int count){ while (count--) {} }

int randombytes(unsigned char* random_array, unsigned long long nbytes)
{   // Generation of "nbytes" of random values
    int r, n = (int)nbytes, count = 0;

    if(lock == -1) {
        do {
            lock = open("/dev/urandom", O_RDONLY);
            if (lock == -1) {
                delay(0xFFFFF);
            }
        } while (lock == -1);
    }
    while(n > 0) {
        do {
            r = read(lock, random_array+count, n);
            if (r == -1) {
                delay(0xFFFF);
            }
        } while (r == -1);
        count += r;
        n -= r;
    }
    return 0;
}





void read_keys(char* keys_path, char* keys)
{   // Reads public and secret key from file saved in keys_path into the array pointed to by keys

    FILE *fp = fopen(keys_path, "rb");
    assert((fp != NULL) && "Error opening keys file");

    assert( fread(keys, NBITS_FIELD * 2, 1, fp) != 0 );
    fclose(fp);
}




