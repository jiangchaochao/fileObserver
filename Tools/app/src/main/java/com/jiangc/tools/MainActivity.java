package com.jiangc.tools;

import android.os.Environment;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;

import com.jiangc.receiver.FileObserverJni;


/**
 * 文件监听demo实现，目前只支持创建和删除事件，默认递归监听子目录，未实现新建目录下文件的监听
 */
public class MainActivity extends AppCompatActivity {

    private static final String TAG = "MainActivity";
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        String path = Environment.getExternalStorageDirectory().getAbsolutePath();

        FileObserverJni fileObserverJni = new FileObserverJni(path + "/ftpFile", FileObserverJni.ALL_EVENTS);
        fileObserverJni.setmCallback(new FileObserverJni.Callback() {
            @Override
            public void FileObserverEvent(String path, int mask) {

            }
        });
    }
}
