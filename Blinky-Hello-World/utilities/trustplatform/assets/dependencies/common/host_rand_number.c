#include "cryptoauthlib.h"
#include <stdlib.h>
#if defined(_WIN32)
#include <time.h>
#endif

/** \brief - Host Random number Generator.
 */
int RNG(uint8_t *dest, unsigned size)
{
#if defined(_WIN32)
    static bool rng_seeded = 0;

    if (!rng_seeded)
    {
        //Seed random number generator
        rng_seeded = 1;
        srand((unsigned int)time(0));
    }

    //Get random number
    while (size--)
    {
        *dest++ = (uint8_t)rand();
    }
#endif
    return 1;
}
