#include "NativeStackTraceProvider.h"
#include "agct_types.h"
#include <iostream>
#include <jvmti.h>
#include <dlfcn.h>
#include <ucontext.h>
#include <string>
#include <sstream>
#include <pthread.h>
#include "StackTrace.h"
#include <cstring>
#include <signal.h>
#include <atomic>
#include "AsyncTracer.h"

constexpr jint MAX_STACK_DEPTH = 4096;

jvmtiEnv* jvmti;
JavaVM* vm;

template<typename T>
jlong toJlong(T value) {
    static_assert(sizeof(T) <= sizeof(jlong), "Type is too long!");
    jlong result = 0;
    std::memcpy(&result, &value, sizeof(T));
    return result;
}

template<typename T>
T fromJlong(jlong value) {
    static_assert(sizeof(T) <= sizeof(jlong), "Type is too long!");
    T result = 0;
    std::memcpy(&result, &value, sizeof(T));
    return result;
}

JNIEXPORT jstring JNICALL Java_rocks_inspectit_bst_NativeStackTraceProvider_init(JNIEnv* env , jclass clazz) {
    if (env->GetJavaVM(&vm) != JNI_OK) {
        return env->NewStringUTF("Failed to get JavaVM instance");
    }
    if (vm->GetEnv(reinterpret_cast<void**>(&jvmti), JVMTI_VERSION_1_0) != JNI_OK) {
        return env->NewStringUTF("Failed to get access to jvmtiEnv");
    }
    return AsyncTracer::init(env, jvmti, MAX_STACK_DEPTH);
}

/*
 * Class:     NativeStackTraceProvider
 * Method:    destroy
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_rocks_inspectit_bst_NativeStackTraceProvider_destroy(JNIEnv* jni, jclass clazz) {
    return AsyncTracer::destroy(jvmti,jni);
}

/*
 * Class:     NativeStackTraceProvider
 * Method:    getNativeThreadId
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_rocks_inspectit_bst_NativeStackTraceProvider_getNativeThreadId(JNIEnv*, jclass) {
    return toJlong(pthread_self());
}

/*
 * Class:     NativeStackTraceProvider
 * Method:    collectStackTraceAsync
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_rocks_inspectit_bst_NativeStackTraceProvider_collectStackTraceAsync(JNIEnv* env, jclass clazz, jlong threadHandle) {
    pthread_t targetThread = fromJlong<pthread_t>(threadHandle);
    return toJlong(AsyncTracer::getStackTrace(targetThread));
}

/*
 * Class:     NativeStackTraceProvider
 * Method:    collectStackTraceForCurrentThread
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_rocks_inspectit_bst_NativeStackTraceProvider_collectStackTraceForCurrentThread(JNIEnv*, jclass) {
    jlong result;

    jvmtiFrameInfo* frames = new jvmtiFrameInfo[MAX_STACK_DEPTH];
    jint frameCount;
    jvmtiError err = jvmti->GetStackTrace(NULL, 0, MAX_STACK_DEPTH, frames, &frameCount);
    if (err == 0) {
        result = toJlong(new StackTrace(frameCount, frames));
    }
    else {
        result = toJlong(new StackTrace(-1, NULL));
    }
    delete[] frames;
    return result;
}

/*
 * Class:     NativeStackTraceProvider
 * Method:    fetchAndReleaseStackTrace
 * Signature: (J)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_rocks_inspectit_bst_NativeStackTraceProvider_fetchAndReleaseStackTrace(JNIEnv* env, jclass clazz, jlong handle, jclass stackFrameClass) {
    StackTrace* trace = fromJlong<StackTrace*>(handle);
    jobject result;
    if (trace->frameCount < 0) { //error case
        jclass cls = env->FindClass("java/lang/Integer");
        jmethodID methodID = env->GetMethodID(cls, "<init>", "(I)V");
        result = env->NewObject(cls, methodID, trace->frameCount);
    }
    else {
        result = trace->toJavaObject(env,jvmti,stackFrameClass);
    }
    delete trace;
    return result;
}


/*
 * Class:     NativeStackTraceProvider
 * Method:    releaseStackTrace
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_rocks_inspectit_bst_NativeStackTraceProvider_releaseStackTrace(JNIEnv* env, jclass clazz , jlong handle) {
     StackTrace* trace = fromJlong<StackTrace*>(handle);
     delete trace;
}