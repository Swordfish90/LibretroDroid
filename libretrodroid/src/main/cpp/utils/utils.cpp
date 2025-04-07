/*
 *     Copyright (C) 2019  Filippo Scognamiglio
 *
 *     This program is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <fstream>
#include <unistd.h>

#include "utils.h"
#include "../log.h"

namespace libretrodroid {

Utils::ReadResult Utils::readFileAsBytes(const std::string &filePath) {
    std::ifstream fileStream(filePath, std::ios::binary);
    if (!fileStream) {
        LOGE("Failed to open file: %s", filePath.c_str());
        return {0, nullptr};
    }

    fileStream.seekg(0, std::ios::end);
    std::streampos size = fileStream.tellg();
    if (size == -1) {
        LOGE("Failed to get file size: %s", filePath.c_str());
        return {0, nullptr};
    }

    char* bytes = new char[size];
    fileStream.seekg(0, std::ios::beg);
    fileStream.read(bytes, size);

    if (fileStream.gcount() != size) {
        LOGE("Failed to read entire file: %s", filePath.c_str());
        delete[] bytes;
        return {0, nullptr};
    }

    fileStream.close();
    return ReadResult { static_cast<size_t>(size), bytes };
}

Utils::ReadResult Utils::readFileAsBytes(const int fileDescriptor) {
    FILE* file = fdopen(fileDescriptor, "r");

    if (!file) {
        LOGE("Failed to open file descriptor %d", fileDescriptor);
        return {0, nullptr};
    }

    size_t size = getFileSize(file);

    if (size == (size_t)-1 || size == 0) {
        LOGE("File descriptor %d is empty or invalid size", fileDescriptor);
        fclose(file);
        return {0, nullptr};
    }
    char* bytes = new char[size];

    size_t bytesRead = fread(bytes, sizeof(char), size, file);
    if (bytesRead != size) {
        if (ferror(file)) {
            LOGE("Error reading from file descriptor %d", fileDescriptor);
        } else {
            LOGE("Reached EOF before reading expected size from descriptor %d", fileDescriptor);
        }
        delete[] bytes;
        fclose(file);
        return {0, nullptr};
    }

    fclose(file);
    return ReadResult { size, bytes };
}

size_t Utils::getFileSize(FILE* file) {
    if (!file) {
        return (size_t)-1;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        return (size_t)-1;
    }

    long size = ftell(file);
    if (size == -1) {
        return (size_t)-1;
    }

    rewind(file);
    return (size_t)size;
}



const char* Utils::cloneToCString(const std::string &input) {
    if (input.empty()) return strdup("");

    size_t len = input.length() + 1;

    char* result = new (std::nothrow) char[len];
    if (!result) {
        LOGE("Memory allocation failed for cloneToCString");
        return nullptr;  //
    }

    std::strcpy(result, input.c_str());
    return result;
}

} //namespace libretrodroid