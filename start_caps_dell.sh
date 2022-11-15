DISTRO=fed
if [ -e /usr/lib64/libyaml-cpp.so.0.7 ]
then
  DISTRO=ubu
fi

#bluetooth devices are not listed in /by-path
#figure out the name with 'evtest' tool

INTERDEV=/dev/input/by-path/platform-i8042-serio-0-event-kbd    #VirtualBox and Dell 9350
#INTERDEV=/dev/input/event4  #apple magic keyboard?
#INTERDEV=/dev/input/event6  #apple air keyboard


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

