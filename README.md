# id107-mynewt
As smartwatch project using Apache mynewt on the ID107 fitness tracker hardware

## How to make this work
- install mynewt: newt tool, toolchain, debugger. See [Getting Started Guide](https://mynewt.apache.org/latest/get_started/index.html).
- pull repo (or download)
- pull mynewt dependencies with `newt upgrade -v`
- run newt tool with build, create-image, load commands as usual. (See below for examples.)

## Targets in this repo
Bootloader

- `id107_boot` Bootloader target.

For apps, mynewt either has a 'unified' or 'split' architecture. Unified is a single app in one of the two image slots. Split is a loader and app combination which allows for using both slots at the same time and thus having more functionality while still having OTA updates over BLE with newtmgr. See mynewt docs for details.

- `id107_oledtest` 'Unified' test app to display some info on OLED display. Also reacts to button presses with vibration.
- `id107_vibrate` 'Unified' test app that actuates the vibration motor with no display.
- `id107_bletest` 'Unified' test app that only advertises over BLE. Basically uses the ble_loader in unified setup.
- `id107_smartwatch` Loader/app (split) target that acts as a watch and is accessible for updates via newtmgr.

## Building, creating app image, and uploading
Switch the name of the app with the target app you want. Bootloader only needs to be loaded/flashed once. See the mynewt documentation for more info.
```
newt build id107_boot
newt build id107_oledtest

newt create-image id107_oledtest 1.0.0

newt load id107_boot

newt load id107_oledtest
```
For split app/loader targets you need to test/activate the app. See test_slot1.sh for an example.
There are also tasks defined for VS Code in .vscode/task.json. Make sure to choose the correct task for unified/split build process.

### Fix Error when using JLinkGDBServer
If uploading fails, or more correctly the second upload for split images does not happen, because there is an error `Qt: Session management error: None of the authentication protocols specified are supported` in the output of the J-Link GDB Server V6.88a, try running `unset SESSION_MANAGER` in the terminal before starting the command.

### Enable shell autocomplete for mynewt
Run `complete -C "newt complete" newt` in terminal. For permanent addition add command to ~/.profile