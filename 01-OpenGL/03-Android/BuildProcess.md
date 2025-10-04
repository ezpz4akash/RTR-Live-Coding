# Devices
    > adb devices

# Build
    > gradlew.bat build

# Deploy
    > adb -d install -r app/build/outputs/apk/debug/app-debug.apk
        ~ -d means device
        ~ -r means replace if already present with same app id
    > adb -d install -r app/build/outputs/apk/debug/app-debug.apk
        ~ Install wia Wifi Debugging