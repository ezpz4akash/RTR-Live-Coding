# Devices
    > adb devices

# Build
    > gradlew.bat build

# Deploy
    > adb -d install -r app/build/outputs/apk/debug/app-debug.apk
        ~ -d means device
        ~ -r means replace if already present with same app id
    > adb install -r app/build/outputs/apk/debug/app-debug.apk
        ~ Install wia Wifi Debugging

# Log Print
    > adb logcat | findstr /i AKM:
    > adb logcat | grep /i AKM:

# Android
PNG Images
res/raw/
Names should be small case(only underscores)

private int textureStone[] = new int[1];

Texture.java
textureStone = Texture.loadGLTexture(R.raw.smiley);

Make sure to delete textures

private int singleTap = 0;
onSingleTapConfirmed : 
    singleTap++;
    if(singleTap > 3){
        singleTap = 0;
    }

Checkerboard
checkerboardWidth, Height : private int
checkerboard = new byte[checkImageHeight * checkImageWidth * 4];


private void makeCheckerBoard(){
    checkerboard
}
