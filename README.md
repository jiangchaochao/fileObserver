
#### 说明
Android JNI 监控指定目录下的文件以及子目录及子目录下的文件，支持传参控制监控事件，主要使用linux inotify和epoll实现

#### 注意事项
demo中未申请存储权限，使用时需要手动打开或者申请存储权限

#### 使用方式


```
主要API：
       /**
        * Create a new file observer for a certain file or directory And start it.
        * @param path The file or directory to monitor
        * @param mask The event or events (added together) to watch for
        */
        public FileObserverJni(String path, int mask, ILifecycle callback) //推荐使用

       /**
        * Equivalent to FileObserver(path, FileObserver.ALL_EVENTS).
        */
        public FileObserverJni(String path)
        
  
例子：
        String path = Environment.getExternalStorageDirectory().getAbsolutePath();
        Log.e(TAG, "onCreate: " + path);

        FileObserverJni fileObserverJni = new FileObserverJni(path, FileObserverJni.ALL_EVENTS, new FileObserverJni.ILifecycle() {
            @Override
            public void onInit(int errno) {
                if (0 == errno) {
                    Log.e(TAG, "onInit: 初始化成功");
                } else {
                    Log.e(TAG, "onInit: 初始化失败: " + FileObserverJni.error2String(errno));
                }
            }

            @Override
            public void onExit(int errno) {
                if (0 == errno) {
                    Log.e(TAG, "onExit: 正常退出");
                } else {
                    Log.e(TAG, "onExit: 异常退出: " + errno);
                }
            }
        });
        fileObserverJni.setmCallback(new FileObserverJni.Callback() {
            @Override
            public void FileObserverEvent(String path, int mask) {
                Log.e(TAG, "FileObserverEvent: xxxxxxxxxxxxxx :  " + path);
                // 在这里监听事件
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

##### 2022/09/18

1. 增加初始化及退出回调
2. 增加错误码转换接口
3. 禁用Android Q 分区存储
