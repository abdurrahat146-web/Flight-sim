# FlightSim Pro

Native Android flight-simulator foundation using Kotlin + C++20 + OpenGL ES 3.0, CMake and Gradle. No HTML/WebView is used.

## Build

Open in Android Studio with Android SDK 35, NDK 27.0.12077973 and CMake 3.22.1 installed.

GitHub Actions installs the Android SDK/NDK/CMake toolchain and uses Gradle 8.10.2 to produce `app-debug.apk` as an artifact.

## Architecture

- Kotlin: Android lifecycle, SurfaceView, touch input and HUD.
- C++20: simulation loop and native renderer.
- OpenGL ES 3: real-time 3D rendering.
- CMake: native build integration.
- Gradle: Android packaging.
- GitHub Actions: reproducible CI build.

The current build contains a procedural aircraft, runway, terrain, camera/projection, depth-tested 3D scene, touch flight controls, throttle and basic flight dynamics. It intentionally contains no copyrighted third-party aircraft/terrain assets.
