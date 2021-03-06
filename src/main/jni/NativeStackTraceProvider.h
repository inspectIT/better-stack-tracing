/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class NativeStackTraceProvider */

#ifndef _Included_NativeStackTraceProvider
#define _Included_NativeStackTraceProvider
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     NativeStackTraceProvider
 * Method:    init
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_rocks_inspectit_bst_NativeStackTraceProvider_init
  (JNIEnv *, jclass);

/*
 * Class:     NativeStackTraceProvider
 * Method:    destroy
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_rocks_inspectit_bst_NativeStackTraceProvider_destroy
  (JNIEnv *, jclass);

/*
 * Class:     NativeStackTraceProvider
 * Method:    getNativeThreadId
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_rocks_inspectit_bst_NativeStackTraceProvider_getNativeThreadId
  (JNIEnv *, jclass);

/*
 * Class:     NativeStackTraceProvider
 * Method:    collectStackTraceAsync
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_rocks_inspectit_bst_NativeStackTraceProvider_collectStackTraceAsync
  (JNIEnv *, jclass, jlong);

/*
 * Class:     NativeStackTraceProvider
 * Method:    collectStackTraceForCurrentThread
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_rocks_inspectit_bst_NativeStackTraceProvider_collectStackTraceForCurrentThread
  (JNIEnv *, jclass);

/*
 * Class:     NativeStackTraceProvider
 * Method:    fetchAndReleaseStackTrace
 * Signature: (J)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_rocks_inspectit_bst_NativeStackTraceProvider_fetchAndReleaseStackTrace
  (JNIEnv *, jclass, jlong, jclass);

/*
 * Class:     NativeStackTraceProvider
 * Method:    releaseStackTrace
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_rocks_inspectit_bst_NativeStackTraceProvider_releaseStackTrace
  (JNIEnv *, jclass, jlong);

#ifdef __cplusplus
}
#endif
#endif
