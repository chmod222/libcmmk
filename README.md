An unofficial implementation of the Masterkey Lighting API that can be used
to programmatically control LEDs of Coolermaster Masterkey keyboards.

Based entirely on captured USB data without any reverse engineering performed
on official binaries.

Due to a lack of hardware available for testing, the only currently supported
models are:

 * MasterKeys Pro L, EU / German
 * MasterKeys Pro S, US
 * MasterKeys MK750, EU / German

Implemented features:
  - Profile customization
    * Switching between P1 - P4
    * Configuring effects
    * Configuring custom background lighting
    * Saving profile customizations to the firmware

  - Manual control
    * Set the entire keyboard to a single color or a color map
    * Set individual keys
    * Activate and configure effects active during manual control
    * Multilayer/mosaic mode support

Build requirements:
  - A C compiler from at least the current century (C99)
  - libusb-1.0


# Ubuntu

## Install dependencies

```
sudo apt-get install libusb-1.0-0-dev
```

## Compile and run:

```
make clean && make
```

Requires root:

```
sudo LD_LIBRARY_PATH=/path/to/out/subdirectory/out:$LD_LIBRARY_PATH ./out/cmmk-testsu
```
