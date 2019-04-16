An unofficial implementation of the Masterkey Lighting API that can be used
to programmatically control LEDs of Coolermaster Masterkey keyboards.

Based entirely on captured USB data without any reverse engineering performed
on official binaries.

For an implementation of this library usable for people who do not want to drop down to C programming
to configure their devices under Linux, see the separate [cmmk_ctrl](https://github.com/chmod222/cmmk_ctrl)
project.

# Device Support
The currently supported and known to work devices include:

 * MasterKeys Pro L, (US & EU / German)
 * MasterKeys Pro S, (US only)
 * MasterKeys MK750, (US / EU / German only)

In theory (and as a goal), this library should support any RGB (and in the future possibly monocolor)
MasterKeys keyboards. Gaps in device support are currently a result of a lack of test devices. See
the "Contributing" section if you would like to help in this regard.

# Implemented features:
  - Profile customization (comparable to the official control software)
    * Switching between P1 - P4
    * Configuring effects
    * Configuring custom background lighting
    * Saving profile customizations to the firmware
    * Multilayer/mosaic mode support

  - Manual control (comparable to the official SDK)
    * Set the entire keyboard to a single color or a color map
    * Set individual keys
    * Activate and configure effects active during manual control

# Contributing
Pull requests for new model support, bug fixes or anything else are always appreciated. If your
keyboard model and layout (ISO/ANSI) combination is not among the supported devices and you want
to help out, feel free to open an issue to let me know.

While the protocol is mapped out enough at this point to fully replicate the official control
center software, there are still some unanswered questions. I'll happily take more protocol samples
from other models to try and decipher the remaining packets.

So if you know you way around Wireshark and usbmon or similar capture tools, feel free to record 
some samples and open a new issue for them.

# Installation
If you're using Arch Linux, you can [find `libcmmk` in the
AUR](https://aur.archlinux.org/packages/libcmmk-git/). Depending on your AUR helper (e.g.
[yay](https://github.com/Jguer/yay)), you can install it with:

```
yay -S libcmmk-git
```

# Build requirements:
  - A C compiler from at least the current century (C99)
  - libusb-1.0
  - cmake 3.0

Build the library and main samples:
```
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=dist ..
make && make install
```

Run the demo:
```
# Requires root if udev rule is not installed

LD_LIBRARY_PATH=dist/lib64 dist/bin/cmmk-test
  -or-
LD_LIBRARY_PATH=dist/lib dist/bin/cmmk-test
```
