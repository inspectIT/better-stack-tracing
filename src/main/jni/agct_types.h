#ifndef _AGCTTYPED_H
#define _AGCTTYPED_H

#include <jni.h>

struct AsyncCallFrame {
    volatile jint location;
    volatile jmethodID method;
};

struct AsyncCallTrace {
    volatile JNIEnv* env;
    volatile jint frameCount;
    volatile AsyncCallFrame* frames;
};

typedef void (*AsyncGetCallTraceType)(AsyncCallTrace*, jint, void*);

#endif 