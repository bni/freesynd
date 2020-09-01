#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    char *input, *name, *output;
    FILE *inf, *outf;
    size_t total;

    if (argc != 4) {
        fprintf(stderr, "Usage: embed <input> <name> <output>\n");
        return 1;
    }

    input = argv[1];
    name = argv[2];
    output = argv[3];

    inf = fopen(input, "rb");
    if (!inf) {
        fprintf(stderr, "Unable to open `%s' for input.\n", input);
        return 1;
    }

    outf = fopen(output, "wb");
    if (!outf) {
        fclose(inf);
        fprintf(stderr, "Unable to open `%s' for output.\n", output);
        return 1;
    }

    fprintf(outf, "#pragma once\n");
    fprintf(outf, "const unsigned char %s_data[] = {\n", name);

    total = 0;
    while (!feof(inf)) {
        unsigned char buf[16];
        size_t n = fread(buf, 1, 16, inf);
        if (n > 0) {
            size_t i;
            for (i = 0; i < n; i++) {
                fprintf(outf, "0x%02x, ", buf[i]);
            }
            total += n;
            fprintf(outf, "\n");
        }
    }

    fprintf(outf, "};\n");
    fprintf(outf, "const int %s_size = %d;\n", name, (int)total);

    fclose(inf);
    fclose(outf);

    return 0;
}

