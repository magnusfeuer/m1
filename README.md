# LICENSING
Jan 24 2015


After getting approval from Magden's original owners, we have agreed
to open source Magden's M1 code and its associated art work.

This code is now open source, licensed under Mozilla Public License
v2. (MPLv2)

All art work is licensed under Creative Commons Attribution 4.0
International (CC BY 4.0)

/Magnus Feuer
magnus _att_ feuerworks dott com


# Build Notes

Build shoddily migrated to Ubuntu 14.04 32 bit.

This repo builds an image that targets the original Jetway computer
that powered the m1 box. It was created before angstrom, buildroot,
and yocto was widespread, and thus the image generation system is
quite shitty.

A yocto port is strongly encouraged in order to start supporting other
platforms.

This top level build checks out ```github.com/magnusfeuer/m1_core```
and ```github.com/magnusfeuer/m1_app```, with branches specced in ```Version.inc```.

Once checked out the core engine is built with all its drivers, tools, etc.

Finally the animation DDS files are created from ``app/src```, and the end
result is packed up in ```out```

From there, various shell scripts can be used to buid a bootable USB
stick that installs the M1 on the target system.

Please note that the system comes with full DRM system (Magden was a
closed source company), but that it can be disabled during compile
time. There is no need to generate keys and other things to get a
working system.

More documentation to come as I ressurect the code.

## Setting up the build box

Install an Ubuntu 12.04 *32 bit* version in a VM or on a build
box. 20GB of disk will be needed for OS, tools, and build.

Add the necessary packages using

    sudo apt-get install libpng-dev flex gperf g++ libx11-dev \
        libmysqlclient-dev libssl-dev libgif-dev git

MySQL is used to manage device key databases on the backend server. It
has nothing to do with the code run on the device.

Non-DRM versions of the code can be built in order to skip package
protection and other corporate stuff not needed anymore

X11 is only used in debug mode on the build host. The target code
running on the device uses the framebuffer.

This code only works with bison-2.3, which is distributed and built in
core/extern, something that should be fixed.

We have custom font rendering, and tools that generates font files
from truetype source fonts.

We have our own graphics animation format, DDS, which is non
destructive and allows direct mapping of an animation file into memory
and render it directly from there onto a framebuffer. MMX, SSE, and
SSE2 acceleration is supported.

The frame buffer driver has a lot of Unichrome cruft code to support
analog video out. There is a cleaned up version of the epic graphics
code (now called EPX) available that we should probably port back to
the M1 code.

## Building

    make APP=jway/roll_demo_x11

## Running

    sh run_debug.sh

