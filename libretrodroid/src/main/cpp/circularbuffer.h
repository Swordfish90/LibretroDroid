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

#ifndef LIBRETRODROID_CIRCULAR_BUFFER_H
#define LIBRETRODROID_CIRCULAR_BUFFER_H

namespace LibretroDroid {

class CircularBuffer
{
public:
    CircularBuffer(size_t size);
    ~CircularBuffer();

    size_t size();
    size_t capacity() const { return dataCapacity; }

    size_t write(const char *data, size_t bytes);
    size_t read(char *data, size_t bytes);
    size_t drop(size_t bytes);

private:
    size_t readIndex;
    size_t writeIndex;
    size_t dataCapacity;

    char* data;
};

}

#endif LIBRETRODROID_CIRCULAR_BUFFER_H
