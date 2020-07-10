#ifndef _STACKTRACE_H_
#define _STACKTRACE_H_

#include <jvmti.h>
#include "agct_types.h"

class StackTrace {
public:
    jint frameCount;
    jmethodID* frames;
    StackTrace(jint frameCount, jvmtiFrameInfo* jvmtiFrames);
    StackTrace(AsyncCallTrace* trace);
    ~StackTrace();

    jobject toJavaObject(JNIEnv* env, jvmtiEnv* jvmti, jclass stackFrameClass);
};

#endif
