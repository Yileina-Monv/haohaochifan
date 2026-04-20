# Environment Recovery Guide

This guide records the development environment currently used by `MealAdvisor`
on Windows, and the minimum steps to reproduce it on a new machine.

## Current Machine Snapshot

Project requirements from `CMakeLists.txt`:

- CMake `>= 3.21`
- C++17
- Qt `>= 6.5`
- Qt modules: `Quick`, `Qml`, `Sql`, `Network`

Observed toolchain on this machine:

- Git: `2.45.1.windows.1`
- CMake: `3.30.5`
- Ninja: `1.12.1`
- MinGW g++: `13.1.0`
- JDK: `Temurin 21.0.10+7`
- ADB: `37.0.0`
- Qt desktop kit: `C:\Qt\6.10.3\mingw_64`
- Qt Android kit: `C:\Qt\6.10.3\android_arm64_v8a`
- Qt tools root: `C:\Qt\Tools`
- Android SDK: `C:\Users\Administrator\AppData\Local\Android\Sdk`
- Android NDK: `C:\Users\Administrator\AppData\Local\Android\Sdk\ndk\27.2.12479018`
- Android build-tools: `36.0.0`
- Android platform currently installed: `android-36`

Important note:

- The cached Android build configuration was created with
  `ANDROID_PLATFORM=android-35`.
- The current SDK directory on this machine only shows `android-36`.
- On a new machine, either install `android-35` as well, or reconfigure the
  Android build to target `android-36`.

## What This Project Does Not Currently Need

No project-local secret env file was found:

- no `.env`
- no local database file checked into the repo
- no token or credential file detected in the project root

That means environment recovery is mostly about toolchains, not secrets.

## Recommended Install Order

1. Install Git for Windows.
2. Install Qt `6.10.3` with these components:
   - `MinGW 64-bit`
   - `Android ARM64`
   - `CMake`
   - `Ninja`
   - `MinGW 13.1.0 64-bit`
   - `Qt Creator` if you want GUI-based kit management
3. Install Eclipse Temurin JDK `21`.
4. Install Android SDK command-line tools / platform-tools.
5. Install Android NDK `27.2.12479018`.
6. Install Android platform package:
   - preferred for parity: `android-35`
   - acceptable if you reconfigure: `android-36`
7. Clone the repo and create fresh `build/` directories.

## Paths Used on This Machine

These are not mandatory, but matching them reduces friction:

```text
Qt root:            C:\Qt
Desktop Qt:         C:\Qt\6.10.3\mingw_64
Android Qt:         C:\Qt\6.10.3\android_arm64_v8a
CMake:              C:\Qt\Tools\CMake_64\bin\cmake.exe
Ninja:              C:\Qt\Tools\Ninja\ninja.exe
MinGW g++:          C:\Qt\Tools\mingw1310_64\bin\g++.exe
qmake:              C:\Qt\6.10.3\mingw_64\bin\qmake.exe
windeployqt:        C:\Qt\6.10.3\mingw_64\bin\windeployqt.exe
Android SDK:        C:\Users\Administrator\AppData\Local\Android\Sdk
Android NDK:        C:\Users\Administrator\AppData\Local\Android\Sdk\ndk\27.2.12479018
JDK:                C:\Program Files\Eclipse Adoptium\jdk-21.0.10.7-hotspot
```

## Environment Variables

This machine is not relying on persistent system env vars for Qt or Android.
The following values were empty at process, user, and machine scope:

- `JAVA_HOME`
- `ANDROID_SDK_ROOT`
- `ANDROID_HOME`
- `ANDROID_NDK_ROOT`
- `Qt6_DIR`
- `CMAKE_PREFIX_PATH`
- `QTDIR`

That said, setting a few explicit env vars on a new machine is still a good
idea because it makes CLI builds reproducible outside Qt Creator.

Recommended user-level env vars:

```powershell
[Environment]::SetEnvironmentVariable('JAVA_HOME', 'C:\Program Files\Eclipse Adoptium\jdk-21.0.10.7-hotspot', 'User')
[Environment]::SetEnvironmentVariable('ANDROID_SDK_ROOT', 'C:\Users\<YourUser>\AppData\Local\Android\Sdk', 'User')
[Environment]::SetEnvironmentVariable('ANDROID_HOME', 'C:\Users\<YourUser>\AppData\Local\Android\Sdk', 'User')
[Environment]::SetEnvironmentVariable('ANDROID_NDK_ROOT', 'C:\Users\<YourUser>\AppData\Local\Android\Sdk\ndk\27.2.12479018', 'User')
```

Optional PATH additions if you want direct shell access:

```text
C:\Qt\Tools\CMake_64\bin
C:\Qt\Tools\Ninja
C:\Qt\Tools\mingw1310_64\bin
C:\Qt\6.10.3\mingw_64\bin
C:\Users\<YourUser>\AppData\Local\Android\Sdk\platform-tools
```

## Desktop Build Recovery

Create a fresh desktop build directory:

```powershell
New-Item -ItemType Directory -Force build\desktop-debug | Out-Null
& 'C:\Qt\6.10.3\mingw_64\bin\qt-cmake.bat' -S . -B build\desktop-debug -G Ninja
& 'C:\Qt\Tools\CMake_64\bin\cmake.exe' --build build\desktop-debug
```

Equivalent explicit CMake form:

```powershell
& 'C:\Qt\Tools\CMake_64\bin\cmake.exe' `
  -S . `
  -B build\desktop-debug `
  -G Ninja `
  -DCMAKE_PREFIX_PATH='C:\Qt\6.10.3\mingw_64' `
  -DCMAKE_C_COMPILER='C:\Qt\Tools\mingw1310_64\bin\gcc.exe' `
  -DCMAKE_CXX_COMPILER='C:\Qt\Tools\mingw1310_64\bin\g++.exe' `
  -DCMAKE_MAKE_PROGRAM='C:\Qt\Tools\Ninja\ninja.exe'
& 'C:\Qt\Tools\CMake_64\bin\cmake.exe' --build build\desktop-debug
```

## Android Build Recovery

Create a fresh Android build directory:

```powershell
New-Item -ItemType Directory -Force build\android-arm64-debug | Out-Null
& 'C:\Qt\Tools\CMake_64\bin\cmake.exe' `
  -S . `
  -B build\android-arm64-debug `
  -G Ninja `
  -DCMAKE_TOOLCHAIN_FILE='C:\Qt\6.10.3\android_arm64_v8a\lib\cmake\Qt6\qt.toolchain.cmake' `
  -DQT_HOST_PATH='C:\Qt\6.10.3\mingw_64' `
  -DANDROID_SDK_ROOT='C:\Users\<YourUser>\AppData\Local\Android\Sdk' `
  -DANDROID_NDK_ROOT='C:\Users\<YourUser>\AppData\Local\Android\Sdk\ndk\27.2.12479018' `
  -DANDROID_ABI='arm64-v8a' `
  -DANDROID_PLATFORM='android-35' `
  -DCMAKE_MAKE_PROGRAM='C:\Qt\Tools\Ninja\ninja.exe'
& 'C:\Qt\Tools\CMake_64\bin\cmake.exe' --build build\android-arm64-debug
```

If `android-35` is not installed, either install it with `sdkmanager`, or
change the configure line to:

```text
-DANDROID_PLATFORM=android-36
```

Example install command:

```powershell
& 'C:\Users\<YourUser>\AppData\Local\Android\Sdk\cmdline-tools\latest\bin\sdkmanager.bat' `
  'platform-tools' `
  'platforms;android-35' `
  'build-tools;36.0.0' `
  'ndk;27.2.12479018'
```

## Verification Checklist

Run these after setup on the new machine:

```powershell
git --version
& 'C:\Qt\Tools\CMake_64\bin\cmake.exe' --version
& 'C:\Qt\Tools\Ninja\ninja.exe' --version
& 'C:\Qt\Tools\mingw1310_64\bin\g++.exe' --version
& 'C:\Qt\6.10.3\mingw_64\bin\qmake.exe' --version
& 'C:\Program Files\Eclipse Adoptium\jdk-21.0.10.7-hotspot\bin\java.exe' -version
& 'C:\Users\<YourUser>\AppData\Local\Android\Sdk\platform-tools\adb.exe' --version
```

Expected key outcomes:

- `qmake` reports Qt `6.10.3`
- `cmake` is `3.30.5` or newer
- `g++` is `13.1.0` or a compatible MinGW toolchain
- Android SDK and NDK paths resolve correctly

## Recovery Notes

- Do not copy the old `build/` directory to the new machine. Regenerate it.
- The repo already ignores build outputs, local DB files, `.env`, and IDE noise.
- If Qt was installed to a different location, adjust the commands above instead
  of forcing the old path layout.
- If you prefer Qt Creator, recreate one desktop kit and one Android kit that
  point to the same versions listed above.
