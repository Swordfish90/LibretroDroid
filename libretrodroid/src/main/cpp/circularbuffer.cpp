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

#include <algorithm>
#include <cstdlib>

#include "circularbuffer.h"

LibretroDroid::CircularBuffer::CircularBuffer(size_t size):
    readIndex(0),
    writeIndex(0),
    dataCapacity(size)
{
    data = new char[size];
}

LibretroDroid::CircularBuffer::~CircularBuffer() {
    delete[] data;
}

size_t LibretroDroid::CircularBuffer::size() {
    long currentWriteIndex = writeIndex;
    long currentReadIndex = readIndex;
    return std::min(
        std::abs(currentWriteIndex - currentReadIndex),
        std::abs(currentReadIndex - currentWriteIndex)
    );
}

size_t LibretroDroid::CircularBuffer::write(const char* inputData, size_t bytes) {
    if (bytes == 0)
        return 0;

    size_t bytesToWrite = std::min(bytes, dataCapacity - size());

    if (bytesToWrite <= dataCapacity - writeIndex) {
        memcpy(data + writeIndex, inputData, bytesToWrite);
        writeIndex = (writeIndex + bytesToWrite) % dataCapacity;
    } else {
        size_t size1 = dataCapacity - writeIndex;
        memcpy(data + writeIndex, inputData, size1);
        size_t size2 = bytesToWrite - size1;
        memcpy(data, inputData + size1, size2);
        writeIndex = size2;
    }

    return bytesToWrite;
}

size_t LibretroDroid::CircularBuffer::read(char *outputData, size_t bytes) {
    if (bytes == 0)
        return 0;

    size_t capacity = dataCapacity;
    size_t bytesToRead = std::min(bytes, size());

    if (bytesToRead <= capacity - readIndex) {
        memcpy(outputData, data + readIndex, bytesToRead);
        readIndex = (readIndex + bytesToRead) % capacity;
    } else {
        size_t size1 = capacity - readIndex;
        memcpy(outputData, data + readIndex, size1);
        size_t size2 = bytesToRead - size1;
        memcpy(outputData + size1, data, size2);
        readIndex = size2;
    }

    return bytesToRead;
}

size_t LibretroDroid::CircularBuffer::drop(size_t bytes) {
    if (bytes == 0)
        return 0;

    readIndex += bytes;
    return bytes;
}
