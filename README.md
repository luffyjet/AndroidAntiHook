
library目录下有反调试，防so注入等代码，修改jni代码后，运行rebuild project即可重新生成so

inject目录下有so注入例子


### Inject说明

1. 真机先进行root

2. android6.0/7.0禁掉Selinux

        adb shell setenforce 0

3. local.properties配置好NDK目录

4. cd 进入 inject目录，执行 ndk-build 命令

5. 将so和可执行文件inject导入真机，执行 inject

        adb root
        adb remount
        adb push libs/armeabi-v7a/libqever.so /data/local/tmp
        adb push libs/armeabi-v7a/inject /data/local/tmp
        adb shell
        cd /data/local/tmp
        ./inject ... ... ...



# Thanks

https://github.com/fourbrother/android_anti_debug

https://bbs.pediy.com/thread-194080.htm

