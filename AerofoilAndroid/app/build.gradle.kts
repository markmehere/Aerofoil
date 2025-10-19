plugins {
    alias(libs.plugins.android.application)
    alias(libs.plugins.kotlin.android)
}

android {
    namespace = "org.thecodedeposit.aerofoil"
    compileSdk = 36

    defaultConfig {
        applicationId = "org.thecodedeposit.aerofoil"
        minSdk = 31
        targetSdk = 36
        versionCode = 33
        versionName = "1.2.3"

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
        setProperty("archivesBaseName", "aerofoil")
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_11
        targetCompatibility = JavaVersion.VERSION_11
    }
    kotlinOptions {
        jvmTarget = "11"
    }
    buildFeatures {
        prefab = true
    }
    externalNativeBuild {
        cmake {
            path = file("jni/CMakeLists.txt")
            version = "3.22.1"
        }
    }
}

dependencies {

    implementation(libs.androidx.core.ktx)
    implementation(libs.androidx.appcompat)
    implementation(libs.material)
    implementation(libs.androidx.games.activity)
    testImplementation(libs.junit)
    androidTestImplementation(libs.androidx.junit)
    androidTestImplementation(libs.androidx.espresso.core)
}