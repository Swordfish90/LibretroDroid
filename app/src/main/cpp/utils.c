#include <stdio.h>
#include <malloc.h>

#include "utils.h"

struct read_file_result read_file_as_bytes(const char *name) {
    FILE *fl = fopen(name, "r");
    fseek(fl, 0, SEEK_END);
    long len = ftell(fl);
    char *ret = malloc(len);
    fseek(fl, 0, SEEK_SET);
    fread(ret, 1, len, fl);
    fclose(fl);

    struct read_file_result result = { ret, len };
    return result;
}
