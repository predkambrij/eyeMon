eyeMon
======

Eye fatigue monitor based on eye blinking for Linux and Android using OpenCV.

Currently are employed two methods for eye blink detection (template based and using farneback optical flow). See common_settings_comp.cpp

In the scope of this project, [eyeMonTester](https://github.com/predkambrij/eyeMonTester) was developed. You can test the methods against public available databases, or create your own (you can determine frame numbers of eye blinks with "make annot" tool).

##Running the code on your computer:
- ```git clone git@github.com:predkambrij/eyeMon.git```
- ```git clone git@github.com:predkambrij/eyeLike eyeMon/_1OpenCVopticalflow/src/main/jni/eyeLike```
- ```docker build --build-arg ARG_UID=$(id -u) --build-arg ARG_GID=$(id -g) -t predkambrij/eyemon eyeMon/dockerfileDesktop/``` # it takes some time to download and compile; currently docker image takes 882.6 MB

Note: if you want sound notifications, you need to pass --device /dev/tty0 and --cap-add SYS_TTY_CONFIG so that beep command (pc speaker) will work

Using SSH: (pass: developerpw)
- ```docker run -d -p 127.0.2.4:1122:22 -v $(pwd)/eyeMon:/eyeMon --device /dev/video0 --device /dev/tty0 --cap-add SYS_TTY_CONFIG predkambrij/eyemon```
- ```ssh -Y developer@127.0.2.4 -p 1122 "cd /eyeMon/; make d"```

Using X11 unix socket (it's less CPU intensive)
- ```docker run -it --rm -v $(pwd)/eyeMon:/eyeMon -v /tmp/.X11-unix:/tmp/.X11-unix -e DISPLAY=$DISPLAY --device /dev/video0 --device /dev/tty0 --cap-add SYS_TTY_CONFIG -u developer predkambrij/eyemon bash -c "cd /eyeMon/; make d"```

##Running the code on your Android phone:
- ```git clone git@github.com:predkambrij/eyeMon.git```
- ```git clone git@github.com:predkambrij/eyeLike eyeMon/_1OpenCVopticalflow/src/main/jni/eyeLike```
- in case of using X11 unix socket, replace uid and gid in Dockerfile so that it matches your user id and group id
- ```docker build -t predkambrij/eyemondev eyeMon/dockerfileDevelop/``` # it takes some time to download and compile; currently docker image takes 8.7 GB

Note: if you want to install the apk using adb, find which usb device belongs to your phone, in this case it's /dev/bus/usb/003/013  
Note2: you can use also X11 unix the same way as above  
Note3: this docker image contains also setup for running the code on your computer, so you can run "make d" to start the application

Using SSH: (pass: developerpw)
- ```docker run -d -p 127.0.2.5:1122:22 -v $(pwd)/eyeMon:/eyeMon --device /dev/video0 --device /dev/bus/usb/003/013 --device /dev/tty0 --cap-add SYS_TTY_CONFIG predkambrij/eyemondev```

Building the apk and running it on your phone
- ```ssh -Y developer@127.0.2.5 -p 1122```
    - ```cd /eyeMon/```
    - # build jni dependences
    - ```make ndkb```
    - # build the apk from cli
    - ```./gradlew assembleDebug```
    - # install the apk
    - ```~/android-sdk-linux/platform-tools/adb install -r ./_1OpenCVopticalflow/build/outputs/apk/_1OpenCVopticalflow-debug.apk```
        - in case of INSTALL_PARSE_FAILED_INCONSISTENT_CERTIFICATES error (rebuilt container)
        - ```~/android-sdk-linux/platform-tools/adb uninstall org.blatnik.eyemon```
    - #start the activity
    - ```~/android-sdk-linux/platform-tools/adb shell am start -n org.blatnik.eyemon/org.blatnik.eyemon.FdActivity```

Start Android Studio and open the project
- ```ssh -Y developer@127.0.2.5 -p 1122```
    - ```~/android-studio/bin/studio.sh``` # note you can install and start it inside the screen or tmux command
    - # it will be the first run. There shouldn't be anything additional to download (Android SDK)
    - # open an existing Android Studio project (select /eyeMon)
    - # all changes will be visible in the project on your host computer ($(pwd)/eyeMon)
        - # you can build the apk from menu: build -> build APK and install and run the same way as above
        - note, currently you need to run "cd /eyeMon/; make ndkb" that changes of c++ files will take effect

##Credits:
- This application is using [eyeLike](https://github.com/trishume/eyeLike/) for finding eye centers


