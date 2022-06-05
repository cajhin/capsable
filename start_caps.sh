DISTRO=fed
if [ -e /usr/lib64/libyaml-cpp.so.0.7 ]
then
  DISTRO=ubu
fi

INTERDEV=/dev/input/by-path/platform-i8042-serio-0-event-kbd
BUILDDIR=/home/jjj/git/capsable/build
sleep 1
echo capsable go...

sudo nice -n -20 $BUILDDIR/intercept -g $INTERDEV | nice -n -20 $BUILDDIR/capsable | nice -n -20 $BUILDDIR/uinput_$DISTRO -d $INTERDEV
#sudo $BUILDDIR/intercept -g $INTERDEV | $BUILDDIR/capsable | $BUILDDIR/uinput_$DISTRO -d $INTERDEV

