package com.wiseyq.safe;

import android.util.Log;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.HashSet;

/**
 * Title :    
 * Author : luffyjet 
 * Date : 2018/5/15
 * Project : SOJNI
 * Site : http://www.luffyjet.com
 */
public class CCProctect {
    private static final String TAG = "UninstallSo";

    //加载so库
    static {
        System.loadLibrary("ccprotect");
    }

    /***
     *  true:already in using  false:not using
     * @param port
     */
    public static boolean isLoclePortUsing(int port) {
        boolean flag = true;
        try {
            flag = isPortUsing("127.0.0.1", port);
        } catch (Exception e) {
            e.printStackTrace();
        }
        return flag;
    }

    /***
     *  true:already in using  false:not using
     * @param host
     * @param port
     * @throws UnknownHostException
     */
    public static boolean isPortUsing(String host, int port) throws UnknownHostException {
        boolean flag = false;
        InetAddress theAddress = InetAddress.getByName(host);
        try {
            Socket socket = new Socket(theAddress, port);
            flag = true;
        } catch (IOException e) {
            e.printStackTrace();
        }
        return flag;
    }


    //该函数搜索进程PID被注入的第三方so。
    public static HashSet<String> getSoList(int pid, String pkg) {
        HashSet<String> temp = new HashSet<String>();
        File file = new File("/proc/" + pid + "/maps");
        if (!file.exists()) {
            Log.i(TAG, "getSoList:proc file not exists");
            return temp;
        }

        try {
            BufferedReader bufferedReader = new BufferedReader(new InputStreamReader(new FileInputStream(file)));
            String lineString;
            while ((lineString = bufferedReader.readLine()) != null) {
                String tempString = lineString.trim();
                Log.i(TAG, "getSoList  tempString:" + tempString);
                if (tempString.endsWith(".so")
                        && !tempString.contains("/data/data/" + pkg)
                        && !tempString.contains("system/lib")) {
                    temp.add(tempString);
                }
            }
            bufferedReader.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return temp;
    }

    //卸载加载的so库
    public static native void uninstall(String soPath);

}
