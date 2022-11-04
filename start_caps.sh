DISTRO=fed
if [ -e /usr/lib64/libyaml-cpp.so.0.7 ]
then
  DISTRO=ubu
fi

#default event path on VirtualBox and Dell 9350
INTERDEV=/dev/input/by-path/platform-i8042-serio-0-event-kbd
#INTERDEV=/dev/input/event4


BUILDDIR=/home/jjj/git/capsable/build
echo -n 'please release all keys'
sleep 0.2
echo -n '.'
sleep 0.2
echo -n '.'
sleep 0.2
echo -n '. '
sleep 0.2

sudo nice -n -20 $BUILDDIR/intercept -g $INTERDEV | nice -n -20 $BUILDDIR/capsable | nice -n -20 $BUILDDIR/uinput_$DISTRO -d $INTERDEV
#sudo $BUILDDIR/intercept -g $INTERDEV | $BUILDDIR/capsable | $BUILDDIR/uinput_$DISTRO -d $INTERDEV

