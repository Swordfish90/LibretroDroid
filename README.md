# LibretroDroid
LibretroDroid is a simple C++ LibRetro frontend library for Android. It's powering [Lemuroid](https://github.com/Swordfish90/Lemuroid).

### Supported features:
* 2D Cores
* GL Cores
* Audio
* Gamepad events
* Serialization and Deserialization of game states
* Serialization and Deserialization of SaveRAM
* Simple shader effects (CRT and LCD)
* Touchscreen
* Multiple disk support
* Core variables

### Tested working cores:
* Stella
* Gambatte
* mGBA
* Mupen64Plus
* Snes9x
* QuickNES
* fceumm
* nestopia
* PPSSPP
* fbneo
* picodrive
* Genesis Plus GX
* DeSmuME
* PCSXReARMed

### Setup
LibretroDroid can be added to a standard build.gradle file.
```
[app/build.gradle]
dependencies {
    ...
    implementation 'com.github.swordfish90:libretrodroid:<version>'
    ...
}
```
