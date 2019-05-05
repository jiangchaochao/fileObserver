package com.jiangc.receiver;

import android.util.Log;

public class FileObserverJni {
    private static final String TAG = "FileObserverJni";

    public interface Callback {

        /**
         * 创建文件事件
         * @param path
         */
        void CreateEvent(String path);

        /**
         * 删除文件事件
         * @param path
         */
        void DeleteEvent(String path);
    }


    /**
     * 创建文件事件，供native调用
     * @param path
     */
    public static void CreateEvent(String path)
    {
        if (mCallback != null)
        {
            mCallback.CreateEvent(path);
        }
    }

    /**
     * 删除文件事件，供native调用
     * @param path
     */
    public static void DeleteEvent(String path)
    {
        if (mCallback != null)
        {
            mCallback.DeleteEvent(path);
        }
    }

    private static Callback mCallback  = null;



    public static void setmCallback(Callback mCallback) {
        FileObserverJni.mCallback = mCallback;
    }

    static {
        try{
            System.loadLibrary("fileobserver");
        }catch (Exception e)
        {
            Log.e(TAG, "static initializer: 加载fileObserver库失败");
        }
    }
    /**
     * 初始化要监听的目录，并启动线程
     * @param path
     * @return
     */
    public static native int FileObserverInit(String path);

    /**
     * 释放资源
     * @return
     */
    public static native int FileObserverDestroy();
}
