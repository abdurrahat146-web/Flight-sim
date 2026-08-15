plugins { id("com.android.application"); kotlin("android") }

android { namespace = "com.rahu.flightsim"; compileSdk = 35
    defaultConfig { applicationId = "com.rahu.flightsim"; minSdk = 24; targetSdk = 35; versionCode = 1; versionName = "1.0" }
    externalNativeBuild { cmake { path = file("src/main/cpp/CMakeLists.txt"); version = "3.22.1" } }
    ndkVersion = "27.0.12077973"
}
