package rocks.inspectit.bst;

public class NativeStackTraceProvider {

    /**
     * Initializes the native library.
     *
     * @return null on success, otherwise an error message
     */
    public static native String init();

    /**
     * Destroys the native lbirary, freeing any allocated space.
     * Afterwards {@link #init()} may be called again.
     *
     * @return null on success, a error message on error
     */
    public static native String destroy();

    /**
     * @return the native thread-id of the current Thread, this ID may be used for {@link #collectStackTraceAsync(long)}
     */
    public static native long getNativeThreadId();

    /**
     * Collects a stack-trace of the given target thread without triggering a safe-point.
     *
     * @param nativeThreadId the id of the target thread, must be obtained via {@link #getNativeThreadId()} first.
     *
     * @return a handle for the resulting stack trace. Must be freed with either {@link #fetchAndReleaseStackTrace(long)} or {@link #releaseStackTrace(long)}.
     */
    public static native long collectStackTraceAsync(long nativeThreadId);

    /**
     * Collects a stackjtrace for the current thread.
     * In contrast to {@link Thread#getStackTrace()} this does not immediately convert the collect trace to a Java-Object, which can be costly.
     *
     * @return a handle for the resulting stack trace. Must be freed with either {@link #fetchAndReleaseStackTrace(long)} or {@link #releaseStackTrace(long)}.
     */
    public static native long collectStackTraceForCurrentThread();

    /**
     * Converts the provided stack-trace from a native to a Java-Object and releases its native counterpart.
     *
     * @param handle the handle to the stacktrace
     * @param stackFrameClass the stack frame class to use. Must have a constructor accepting (Class declaringClass, String name, String signature)
     * @return On success an array of stack-frames. On failure a {@link Integer} with an error code is returned.
     */
    public static native Object fetchAndReleaseStackTrace(long handle, Class<?> stackFrameClass);

    /**
     * Releases the native stack trace without converting it to a java object.
     * @param handle the handle to the stacktrace
     */
    public static native void releaseStackTrace(long handle);

}
