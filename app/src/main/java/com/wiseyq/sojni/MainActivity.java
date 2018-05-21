package com.wiseyq.sojni;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;

import com.wiseyq.safe.CCProctect;

import java.io.File;
import java.util.HashSet;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = "MainActivity";

    //加载so库
    static {
        System.loadLibrary("ccprotect");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
//        List<AndroidProcess> list = AndroidProcesses.getRunningProcesses();
//        for (AndroidProcess androidProcess :list){
//            Log.i(TAG, "androidProcess: "+androidProcess.name + " pid:" + androidProcess.pid);
//        }

        Log.i(TAG,  "my pid:" + android.os.Process.myPid());

        findViewById(R.id.findbtn).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                HashSet<String> hashSet = CCProctect.getSoList(android.os.Process.myPid(),getPackageName());
                Log.i(TAG, "onClick: ");
                for (String s:hashSet){
                    Log.i(TAG, "so path: "+s);
                }
            }
        });


        findViewById(R.id.deleteBtn).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.i(TAG, "onClick: delete");
                CCProctect.uninstall("/data/local/tmp/libqever.so");
            }
        });


        try {
            File file = new File("/data/local/tmp/libqever.so");
            if (file.exists()){
                Log.i(TAG, "libqever: "+file.length());
            }
        }catch (Exception e){
            e.printStackTrace();
        }
    }
}
