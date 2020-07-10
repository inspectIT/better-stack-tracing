#ifndef _ASYNCTRACES_H
#define _ASYNCTRACES_H

#include <jvmti.h>
#include <signal.h>
#include <atomic>
#include <pthread.h>
#include "StackTrace.h"
#include "agct_types.h"

class AsyncTracer {
    static AsyncGetCallTraceType asyncGetCallTrace;

    static const int SIGNAL_CODE;
    static constexpr pthread_t INVALID_TID = 0;

    static std::atomic<pthread_t> armedThread;
    static volatile bool sampleFinished;

    static JavaVM* vm;
    static jint maxDepth;
    static AsyncCallTrace globalTrace;
    static AsyncCallFrame* globalFrames;

    static void signalHandler(int signal, siginfo_t* siginfo, void* context);
    
    static void classLoadCallback(jvmtiEnv* jvmti, JNIEnv* jni, jthread thread, jclass clazz);

    static void populateMethodIDs(jvmtiEnv* jvmti, jclass clazz);
    static void populateMethodIDsOfLoadedClasses(jvmtiEnv* jvmti);

    static jstring locateAsyncGetCallTrace(JNIEnv* env);
    static jstring initMethodIdPopulation(JNIEnv * env, jvmtiEnv * jvmti);
    static jstring registerSignalHandler(JNIEnv* env, jint maxDepth);

    //static void disableMethodIdPopulation(JNIEnv* env, jvmtiEnv* jvmti);
    //static void unregisterSignalHandler();

public:
    static jstring init(JNIEnv* env, jvmtiEnv* jvmti, jint maxDepth);
    static StackTrace* getStackTrace(pthread_t target);
    static jstring destroy(jvmtiEnv* jvmti, JNIEnv* jni);
};

#endif 