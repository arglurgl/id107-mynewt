# id107-mynewt
As smartwatch project using Apache mynewt on the ID107 fitness tracker hardware

## How to make this work
- install mynewt: newt tool, toolchain, debugger. See [Getting Started Guide](https://mynewt.apache.org/latest/get_started/index.html).
- pull repo (or download)
- pull mynewt dependencies with `newt upgrade -v`
- run newt tool with build, create-image, load commands as usual. (See below for examples.)

## Targets in this repo
- `id107_boot` Bootloader target.
- `id107_oledtest` App to display some info on OLED display. Also reacts to button presses with vibration.
- `id107_vibrate` App that actuates the vibration motor with no display.

## Building, creating app image, and uploading
Switch the name of the app with the target app you want. Bootloader only needs to be loaded/flashed once. See the mynewt documentation for more info.
```
newt build id107_boot
newt build id107_oledtest

newt create-image id107_oledtest 1.0.0

newt load id107_boot

newt load id107_oledtest
```
