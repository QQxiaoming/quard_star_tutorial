# 编译linuxdeployqt

## 获取源码

```shell
git clone https://github.com/probonopd/linuxdeployqt.git

cd linuxdeployqt
```

## 屏蔽以下代码

```c++
// openSUSE Leap 15.0 uses glibc 2.26 and is used on OBS
    if (strverscmp (glcv, "2.27") >= 0) {  //注释版本检查
      qInfo() << "ERROR: The host system is too new.";
      qInfo() << "Please run on a system with a glibc version no newer than what comes with the oldest";
      qInfo() << "currently still-supported mainstream distribution (xenial), which is glibc 2.23.";
      qInfo() << "This is so that the resulting bundle will work on most still-supported Linux distributions.";
      qInfo() << "For more information, please see";
      qInfo() << "https://github.com/probonopd/linuxdeployqt/issues/340";
      return 1;
    }
```

```shell
camke .

make

cp ./tools/linuxdeployqt ../linuxdeployqt
```
