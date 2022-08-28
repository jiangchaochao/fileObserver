package com.jiangc.receiver;

import android.util.Log;

import java.net.PasswordAuthentication;

public class FileObserverJni {
    private static final String TAG = "FileObserverJni";

    //Event
    /** Event type: Data was read from a file */
    public static final int ACCESS = 0x00000001;
    /** Event type: Data was written to a file */
    public static final int MODIFY = 0x00000002;
    /** Event type: Metadata (permissions, owner, timestamp) was changed explicitly */
    public static final int ATTRIB = 0x00000004;
    /** Event type: Someone had a file or directory open for writing, and closed it */
    public static final int CLOSE_WRITE = 0x00000008;
    /** Event type: Someone had a file or directory open read-only, and closed it */
    public static final int CLOSE_NOWRITE = 0x00000010;
    /** Event type: A file or directory was opened */
    public static final int OPEN = 0x00000020;
    /** Event type: A file or subdirectory was moved from the monitored directory */
    public static final int MOVED_FROM = 0x00000040;
    /** Event type: A file or subdirectory was moved to the monitored directory */
    public static final int MOVED_TO = 0x00000080;
    /** Event type: A new file or subdirectory was created under the monitored directory */
    public static final int CREATE = 0x00000100;
    /** Event type: A file was deleted from the monitored directory */
    public static final int DELETE = 0x00000200;
    /** Event type: The monitored file or directory was deleted; monitoring effectively stops */
    public static final int DELETE_SELF = 0x00000400;
    /** Event type: The monitored file or directory was moved; monitoring continues */
    public static final int MOVE_SELF = 0x00000800;

    /** Event mask: All valid event types, combined */
    public static final int ALL_EVENTS = ACCESS | MODIFY | ATTRIB | CLOSE_WRITE
            | CLOSE_NOWRITE | OPEN | MOVED_FROM | MOVED_TO | DELETE | CREATE
            | DELETE_SELF | MOVE_SELF;


    private static Callback mCallback  = null;

    /**
     * Equivalent to FileObserver(path, FileObserver.ALL_EVENTS).
     */
    public FileObserverJni(String path)
    {
        this(path, ALL_EVENTS);
    }

    /**
     * Create a new file observer for a certain file or directory And start it.
     * @param path The file or directory to monitor
     * @param mask The event or events (added together) to watch for
     */
    public FileObserverJni(String path, int mask)
    {
        FileObserverInit(path, mask);
    }

    public interface Callback {
        /**
         * 总的事件出口,前面的都弃用
         * @param path
         * @param mask
         */
        void FileObserverEvent(String path, int mask);
    }


    /**
     * 总的事件入口，供native调用
     * @param path
     * @param mask
     */
    public static void FileObserverEvent(String path, int mask)
    {
        Log.e(TAG, "Event: path = " + path + "    " + mask);
    }


    public void setmCallback(Callback mCallback) {
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
     * @param mask
     * @return
     */
    public static native int FileObserverInit(String path, int mask);

    /**
     * 释放资源
     * @return
     */
    public static native int FileObserverDestroy();
}
