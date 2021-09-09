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

#ifndef LIBRETRODROID_JAVAUTILS_H
#define LIBRETRODROID_JAVAUTILS_H

#include <jni.h>
#include "../environment.h"

namespace libretrodroid {

class JavaUtils {
public:
    static jint throwRetroException(JNIEnv *env, int errorCode);
    static Variable variableFromJava(JNIEnv *env, jobject obj);
    static void javaListForEach(JNIEnv* env, jobject jList, const std::function<void(jobject)> &lambda);
};

}

#endif //LIBRETRODROID_JAVAUTILS_H
