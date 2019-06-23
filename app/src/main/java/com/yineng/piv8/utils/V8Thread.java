package com.yineng.piv8.utils;

import com.yineng.piv8.V8;

public class V8Thread extends Thread {

    private final V8Runnable target;
    private V8               runtime;

    /**
     * Create as new Thread with its own V8Runtime.
     *
     * @param target The code to execute with the given runtime.
     */
    public V8Thread(final V8Runnable target) {
        this.target = target;
    }

    /*
     * (non-Javadoc)
     * @see java.lang.Thread#run()
     */
    @Override
    public void run() {
        runtime = V8.createV8Runtime();
        try {
            target.run(runtime);
        } finally {
            synchronized (this) {
                if (runtime.getLocker().hasLock()) {
                    runtime.close();
                    runtime = null;
                }
            }
        }
    }

}
