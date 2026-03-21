#include "d64.h"
#include <string.h>

size_t ts2block(uint8_t track, uint8_t sector)
{
    if (track < 18) return ((size_t) track -  1) * 21 + sector;
    if (track < 25) return ((size_t) track - 18) * 19 + 17 * 21 + sector;
    if (track < 31) return ((size_t) track - 25) * 18 + 17 * 21 + 19 * 7 + sector;
    return ((size_t) track - 21) * 17 + 17 * 21 + 19 * 7 + 18 * 6 + sector;
}

FILE *opend64(const char *fname, bam_t *bam_p)
{
    FILE *f;

    memset(bam_p, 0, sizeof (bam_t));

    /* Open the file */
    f = fopen(fname, "r");
    if (!f) {
        /* Unable to open file */
        fprintf(stderr, "Unable to open %s for reading\n", fname);
        return NULL;
    }

    /* Check size */
    fseek(f, 0, SEEK_END);
    if (ftell(f) != 174848) {
        /* File size mismatch */
        fprintf(stderr, "D64 image %s size mismatch\n", fname);
        goto out;
    }

    /* Check integrity */
    fseek(f, 91392, SEEK_SET); /* Start of directory header block */
    fread(bam_p, sizeof (bam_t), 1, f);
    if (bam_p->format != 'A') {
        /* Something is strange here */
        fprintf(stderr, "D64 image %s is not a C1541 disk image\n", fname);
        goto out;
    }
    if (bam_p->t != 18 || bam_p->s != 1) {
        /* That is strange, but ignored by a real drive */
        fprintf(stderr, "D64 image %s has non-standard directory start "
                "(%d,%d); assuming 18:1\n", fname, bam_p->t, bam_p->s);
    }

    /* Looks fine to us */
    return f;

out:
    fclose(f);
    return NULL;
}
