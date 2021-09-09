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

#include "javautils.h"
#include "jnistring.h"

namespace libretrodroid {

jint JavaUtils::throwRetroException(JNIEnv* env, int errorCode) {
    const char *className = "com/swordfish/libretrodroid/RetroException";
    jclass clazz = env->FindClass(className);
    jmethodID constructor = env->GetMethodID(clazz, "<init>", "(I)V");
    auto exception = (jthrowable) env->NewObject(clazz, constructor, errorCode);
    return env->Throw(exception);
}

Variable JavaUtils::variableFromJava(JNIEnv *env, jobject obj) {
    jclass variableClass = env->FindClass("com/swordfish/libretrodroid/Variable");

    jfieldID jKeyField = env->GetFieldID(variableClass, "key", "Ljava/lang/String;");
    jfieldID jValueField = env->GetFieldID(variableClass, "value", "Ljava/lang/String;");

    auto key = JniString(env, (jstring) env->GetObjectField(obj, jKeyField));
    auto value = JniString(env, (jstring) env->GetObjectField(obj, jValueField));

    return Variable { key.stdString(), value.stdString() };
}

void JavaUtils::javaListForEach(
    JNIEnv *env,
    jobject jList,
    const std::function<void(jobject)> &lambda
) {
    auto jListClass = env->GetObjectClass(jList);

    jobject jIterator = env->CallObjectMethod(
        jList,
        env->GetMethodID(jListClass, "iterator", "()Ljava/util/Iterator;")
    );
    jclass jIteratorClass = env->GetObjectClass(jIterator);

    jmethodID nextMethodID = env->GetMethodID(jIteratorClass, "next", "()Ljava/lang/Object;");
    jmethodID hasNextMethodID = env->GetMethodID(jIteratorClass, "hasNext", "()Z");

    while (env->CallBooleanMethod(jIterator, hasNextMethodID)) {
        lambda(env->CallObjectMethod(jIterator, nextMethodID));
    }
}

} //namespace libretrodroid
