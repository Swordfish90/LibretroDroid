#ifndef UTILS_H
#define UTILS_H

struct read_file_result {
    char* data;
    long size;
};

struct read_file_result read_file_as_bytes(const char *name);

#endif
