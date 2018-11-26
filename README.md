= Example on using OpenCV from Qt

Tested with both linux desktop and on Android.

Issues
* Something wrong with color detection filter on Android, colospace conversion must be wrong

= Object detetction with YOLO

The example uses YOLO for object detection, you need to download yolo weights from https://pjreddie.com/darknet/yolo/ and put in respective directory.
Configuration CFG and object name files are already included. Then choose the version to use in the project file.

= Out of memory when building apk

Add _JAVA_OPTIONS="-Xmx2g" to build environment if you hit an Out of memory exception when building apk with large weights file.
