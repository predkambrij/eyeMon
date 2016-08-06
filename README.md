eyeMon
======

Eye fatigue monitor based on eye blinking for Linux and Android using OpenCV.

Currently are employed two methods for eye blink detection (template based and using farneback optical flow). See common_settings_comp.cpp

##Running the code on your computer:
- git clone git@github.com:predkambrij/eyeMon.git
- git clone git@github.com:predkambrij/eyeLike.git eyeMon/jni/eyeLike/
- in case of using X11 unix socket, replace uid and gid in Dockerfile so that it matches your user id and group id
- docker build -t predkambrij/eyemon eyeMon/dockerfileDesktop/

Note: if you want sound notifications, you need to pass --device /dev/tty0 and --cap-add SYS_TTY_CONFIG so that beep command (pc speaker) will work

Using SSH: (pass: developerpw)
- docker run -d -p 127.0.2.4:1122:22 -v $(pwd)/eyeMon:/eyeMon --device /dev/video0 --device /dev/tty0 --cap-add SYS_TTY_CONFIG predkambrij/eyemon
- ssh -Y developer@127.0.2.4 -p 1122 "cd /eyeMon/; make d"

Using X11 unix socket (it's less CPU intensive)
- docker run -it --rm -v $(pwd)/eyeMon:/eyeMon -v /tmp/.X11-unix:/tmp/.X11-unix -e DISPLAY=$DISPLAY --device /dev/video0 --device /dev/tty0 --cap-add SYS_TTY_CONFIG -u developer predkambrij/eyemon bash -c "cd /eyeMon/; make d"


##TODO
- build android apk in cli

##Credits:
- This application is using https://github.com/trishume/eyeLike/ for finding eye centers


