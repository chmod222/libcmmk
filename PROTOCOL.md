Communication style
===================
The communication protocol is realized via a simple USB interrupt exchange on
Interface 1.  All payloads are limited and padded to 64 bytes, excluding the
preceeding USB headers.

Requests are sent to the device using endpoint 04, responses are returned on
endpoint 83.


Packets
======
While most of these have fairly obvious meanings and have been verified by
changing various settings via the offical SDK or control software and observing
the changes, some packets are still largely unknown.


General structure
-----------------
The packets generally start with a single byte that indicate the type of
request (such as querying or setting properties), followed by another byte
to narrow the request down to a specific function (such as current effect,
effect parameters, current profile, ...).

Immediately following this header, the payload information starts, which is
completely specific to the request being sent.


01: Handshake / initialization
--------------------------------

### 01 02: (Unclear)

This is the first packet sent when starting the control software.  The payload
of the request is largely unknown (`df fc 00 30 00 00 3d 30 ...`).

Unlike most other packets, the response to `01 02` is not another `01 02`
packet, but a separate `05 00` described below.

The firmware does not seem to care if it's not being sent before taking control.


#### 05 00: Firmware Version

Payload is the firmware version in ASCII, starting at offset 04 and ending
at 0C for a total length of 8 bytes, padded with 00 to reach that length
before filling the rest of the packet with ff.

The payload of this packet is fairly obvious since it's in ASCII and also
displayed literally in the control software.


### 40 20: (Unclear)

This is the second packet sent in the software startup.  The payload is all `00`.

The response is completely unknown in meaning, all that is known right now is
that it appears to be constant between the same device models on different
physical devices.

The firmware also does not seem to care if this one is left out.


41: Change control mode
-----------------------
This one is fairly well understood. It's a pure request packet that contains no
interesting response (simply echos back the request).

### 41 00: Set firmware control
This is the default mode for the keyboard when not actively being controlled
by software.  In this mode there is no application defined controlling going
on and the device acts completely standalone.

### 41 01: Set effect control
This mode is a middle way between firmware and manual control.  While the
device is in this mode, the user is unable to change profiles or change the
device, but the application can change effects and effect parameters on demand.

The official control software spends the majority of its time in this mode.

### 41 02: Set manual control
This is a completely manual mode.  Once enabled, the device goes black and
simply waits for control commands to set LEDs.  The relevant packets are
described at the end of this documents.

This is also the mode used by the official SDK to set effects.  The official
control software only goes into this mode for the "System Status" pseudo-effect.

### 41 03: Set profile control
This mode is not *completely* understood.  Inside this mode, the keyboard does
not accept LED control or effect changes, and won't let the user switch to other
profiles.

It does allow changing profiles programmatically and querying all effects and
their respective parameters.

The official control software switches to this mode during startup to load all
settings off the device and also switches back to this mode when writing the
changes back to the firmware.


50: Profile management
----------------------
### 50 55: Save current profile
Very simply, saves any changes written to device memory for the currently
active profile to the firmware.

This is mostly used in profile control mode, but apparently you can do this
in effects mode as well.


51 and 52: Generic control
----------------------
The 51 and 52 packets are the meat of the protocol and for all intents and
purposes the thing that's interesting.  They are mirror copies of one another,
where `51` sets a property, `52` reads that same property.  The query packets
are only filled out as much as they need to be to fully describe what needs to
be queried, while the rest is filled with zeros.

For example, `51 2c <ml> <eid> <p1> <p2> <p3> ...` is used to set the effect
parameters for `<eid>` in `<ml>` mode (all described in more detail below).
The corresponding query packet is thus structured `52 2c <ml> <eid> [00...]`.

### 5x 00: Get or set active profile
Request and response payload: `00 00 <profile id>`.


### 5x 01: (Unknown)
Request and response payload: `<profile id> 00 [ff...]`.

No clear data available right now, so the purpose is unknown.  The official
control software sends `01` queries for every profile on init and sends
`01` setters for every profile on exit.


### 5x 10: (Unknown)
Only the query was observed (so no `51 10`).  It's purpose is also a complete
mystery.  The payload and response in the oberserved samples has been all zero.


### 5x 28: Get or set active effect
Request and response payload: `00 00 <eid>`.

The `eid` parameter describes the active effect.  It's represented in the
library by means of the `cmmk_effect_id` enum, defined as follows:

```c
enum cmmk_effect_id {
    CMMK_EFFECT_FULLY_LIT = 0x00,
    CMMK_EFFECT_BREATHE = 0x01,
    CMMK_EFFECT_CYCLE = 0x02,
    CMMK_EFFECT_SINGLE = 0x03,
    CMMK_EFFECT_WAVE = 0x04,
    CMMK_EFFECT_RIPPLE = 0x05,
    CMMK_EFFECT_CROSS = 0x06,
    CMMK_EFFECT_RAINDROPS = 0x07,
    CMMK_EFFECT_STARS = 0x08,
    CMMK_EFFECT_SNAKE = 0x09,
    CMMK_EFFECT_CUSTOMIZED = 0x0a,
    CMMK_EFFECT_MULTILAYER = 0xe0,
    CMMK_EFFECT_OFF = 0xfe 
};
```


### 5x 29: Get or set enabled effects
Request and response payload: `00 00 <eid>x18`.

The enabled effects are marked in the official control software by the red
boxes to the left or their names.  They may describe which effects are available
to be set via key shortcuts.

The payload is a list of up to 18 effect IDs, padded with `ff` if the actual
list of enabled effects is not long enough.  The ordering does not seem to matter.


### 5x 2c: Get or set effect parameters
Request and response payload: `<ml> 00 <eid> <p1> <p2> <p3> <r1> <g1> <b1> <r2> <g2> <b2> [ff...]`

The parameters `p1`, `p2`, `p3` as well as the colors (here called `c1`
and `c2`) have different meaning for all effects and some components are unused
in various effects.

What is constant is that `p1` is always a speed value for effects that are in some
way animated.  The values goes from approximately `10` (absurdly fast) to `50`
(imperceptably slow).  While the official control software always sets the same
values for the various positions of the speed slider (apart from effect ID 02
which for some reason uses different settings), the firmware happily accepts
most values and seems to interpolate between the "real" steps.

`p2` so far has only been observed in effects `04` and `05`.  In `04` it's
constrained to the following enumeration:

```c
enum cmmk_wave_direction {
	CMMK_LEFT_TO_RIGHT = 0x00,
	CMMK_RIGHT_TO_LEFT = 0x04,
	CMMK_BACK_TO_FRONT = 0x02,
	CMMK_FRONT_TO_BACK = 0x06
};
```

In effect `05` it's values are `00` in which case the device will use the given
color to represent the ripple waves, or `80` in which case it will be a
randomized color every keypress.

`p3` is used in effects `07` and `08` where it controls what can best be described
as an intensity or interval setting.  It follows the same approximate schema
as `p1` does for speed.

The colors `c1` and `c2` define the active and background colors respectively.

Lastly, the `ml` parameter at the very beginning of the payload indicates to the
firmware if you want to query or set the effect parameters in normal mode
(= `00`) or in multilayer mode (= `01`).


### 5x a0: Get or set multilayer mapping
Request and response payload: `01 00 <o1> <o2> 00 00 [eids]`.

The request consists of 3 separate packets with the following sequence of
`o1` and `o2` parameters:

1. `00 07`
2. `07 07`
3. `0e 01`

Starting at payload offset 6 is a list of effect IDs which map an effect to
the button at that offset.  The first packets always contain 56 effect IDs,
the last one contains the remainder which is up to the specific device model
and layout.


### 5x a8: Get or set custom RGB mapping
Request and response payload: `<o1> 00 [<rN> <gN> <bN>]x16`.

The request consists of 8 separate packets with `o1` being set to `2*i` for the
`i`th packet.

Every packet contains up to 16 RGB values.


c0: Manual control
------------------
These are used in the manual control mode.

### c0 00: Set the entire keyboard to a single color
Payload: `00 00 <r> <g> <b>`

### c0 01: Set a single key to a given color
Payload: `01 00 <key id> <r> <g> <b>`

### c0 02: Set the entire keyboard from a given color map
Payload: See `5x a8`.

### c0 f0: (Unknown)
Fairly recent addition, not much known except the payload from the official
SDK being `00 00 01` with an all `00` response.


Key Mapping and key IDs
=======================
The key mapping from the outside appears largely random, with keys being
all over the place.

The known key position and ID associations have been determined manually and
compiled into the `keymap_eu.h` and `keymap_us.h` files.

For the big multi-packet LED setting packets `c0 02` and `51 a8` the keys are
passed in ascending order from id 0 to N.  No ordering or topography can be
discerned from this linear list so the conversion between the convenient
matrix and inconvenient but protocol friendly linear representations is done
through lookup tables generated from the manually created layouts.
