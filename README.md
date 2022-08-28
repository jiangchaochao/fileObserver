
#### 说明
Android JNI 监控指定目录下的文件以及子目录及子目录下的文件，支持传参控制监控事件，主要使用linux inotify和epoll实现



#### 使用方式


```
主要API：
       /**
        * Create a new file observer for a certain file or directory And start it.
        * @param path The file or directory to monitor
        * @param mask The event or events (added together) to watch for
        */
        public FileObserverJni(String path, int mask)   //推荐使用

       /**
        * Equivalent to FileObserver(path, FileObserver.ALL_EVENTS).
        */
        public FileObserverJni(String path)
  
例子：
        String path = Environment.getExternalStorageDirectory().getAbsolutePath();

        FileObserverJni fileObserverJni = new FileObserverJni(path + "/ftpFile", FileObserverJni.ALL_EVENTS);
        fileObserverJni.setmCallback(new FileObserverJni.Callback() {
            @Override
            public void FileObserverEvent(String path, int mask) {
                  //这里根据mask做事件的判断
            }
        });

```



#### 修改记录：

##### 2019/5/9  
1.修改默认监听创建删除事件为可传入mask参数
2.修改使用接口，仿FileObserver
3.修改回调方法为统一方法回调



##### 适配发现
1.锤子坚果pro事件有问题，除了删除，其他都不好用



##### 2022/08/28

1. 修改项目为Androidx
2. 修改项目Android.mk 为CMakeLists.txt
3. 修正没有回调Java接口的bug
