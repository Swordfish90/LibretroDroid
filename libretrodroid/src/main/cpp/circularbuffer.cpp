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
    writeIndex(0)
{
    dataCapacity = std::min(size, (size_t)(44L * 1000L));
}

size_t LibretroDroid::CircularBuffer::availableSize() {
    return (dataCapacity - writeIndex + readIndex - 1) % dataCapacity;
}

size_t LibretroDroid::CircularBuffer::usedSize() {
    return dataCapacity - availableSize();
}

size_t LibretroDroid::CircularBuffer::write(const unsigned char* inputData, size_t bytes) {
    if (bytes == 0) return 0;

    size_t bytesToWrite = std::min(bytes, availableSize());

    if (bytesToWrite <= dataCapacity - writeIndex) {
        memcpy(data + writeIndex, inputData, bytesToWrite);
    } else {
        size_t size1 = dataCapacity - writeIndex;
        memcpy(data + writeIndex, inputData, size1);
        size_t size2 = bytesToWrite - size1;
        memcpy(data, inputData + size1, size2);
    }

    writeIndex = (writeIndex + bytesToWrite) % dataCapacity;
    return bytesToWrite;
}

size_t LibretroDroid::CircularBuffer::read(unsigned char* outputData, size_t bytes) {
    if (bytes == 0) return 0;

    size_t bytesToRead = std::min(bytes, usedSize());

    if (bytesToRead <= dataCapacity - readIndex) {
        memcpy(outputData, data + readIndex, bytesToRead);
    } else {
        size_t size1 = dataCapacity - readIndex;
        memcpy(outputData, data + readIndex, size1);
        size_t size2 = bytesToRead - size1;
        memcpy(outputData + size1, data, size2);
    }

    readIndex = (readIndex + bytesToRead) % dataCapacity;
    return bytesToRead;
}

size_t LibretroDroid::CircularBuffer::drop(size_t bytes) {
    if (bytes == 0)
        return 0;

    size_t bytesToRead = std::min(bytes, usedSize());

    readIndex = (readIndex + bytesToRead) % dataCapacity;
    return bytes;
}
