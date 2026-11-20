/*
 * gen_shifted_wav.c - Generate a shiftedWavData*.h header from an 8-bit PCM file.
 *
 * Usage: gen_shifted_wav <input.pcm> <output.h> <variable_name>
 *
 * Each 8-bit sample is left-shifted by 16 bits and stored as a uint32_t,
 * matching the on-the-fly shift that used to happen in init_sound():
 *     shifted_wav_data[i] = wav_data[i] << 16;
 *
 * The DMA reads 32-bit words from this array into the PWM compare register,
 * so each element is one full sample value.
 *
 * This is a host tool (built with the system compiler), not firmware code.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define VALUES_PER_LINE 12

static void to_upper(char *s) {
    for (; *s; s++) {
        *s = (char)toupper((unsigned char)*s);
    }
}

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <input.pcm> <output.h> <variable_name>\n", argv[0]);
        return 1;
    }

    const char *pcmfile = argv[1];
    const char *outheader = argv[2];
    const char *varname = argv[3];

    FILE *f = fopen(pcmfile, "rb");
    if (!f) {
        fprintf(stderr, "error: cannot open %s\n", pcmfile);
        return 1;
    }
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);

    unsigned char *raw = malloc((size_t)n);
    if (!raw) {
        fprintf(stderr, "error: out of memory\n");
        fclose(f);
        return 1;
    }
    if (fread(raw, 1, (size_t)n, f) != (size_t)n) {
        fprintf(stderr, "error: short read on %s\n", pcmfile);
        free(raw);
        fclose(f);
        return 1;
    }
    fclose(f);

    // Build the include guard from the variable name.
    char guard[256];
    snprintf(guard, sizeof(guard), "_SHIFTED_WAV_DATA_%s_H_", varname);
    to_upper(guard);

    FILE *out = fopen(outheader, "w");
    if (!out) {
        fprintf(stderr, "error: cannot open %s for writing\n", outheader);
        free(raw);
        return 1;
    }

    fprintf(out, "#ifndef %s\n", guard);
    fprintf(out, "#define %s\n", guard);
    fprintf(out, "const uint32_t %s[] = {\n", varname);

    for (long i = 0; i < n; i++) {
        if (i % VALUES_PER_LINE == 0) {
            fputs("  ", out);
        } else {
            fputs(", ", out);
        }
        fprintf(out, "0x%08lx", (unsigned long)(raw[i] << 16));
        if (i + 1 < n && (i + 1) % VALUES_PER_LINE == 0) {
            // Full line that is not the last one: trailing comma.
            fputc(',', out);
        }
        if ((i + 1) % VALUES_PER_LINE == 0 || i + 1 == n) {
            fputc('\n', out);
        }
    }

    fprintf(out, "};\n");
    fprintf(out, "const unsigned int %s_len = %ld;\n", varname, n);
    fprintf(out, "#endif\n");

    fclose(out);
    free(raw);

    // Print the basename of the output file.
    const char *base = strrchr(outheader, '/');
    base = base ? base + 1 : outheader;
    printf("wrote %s (%ld samples)\n", base, n);

    return 0;
}
