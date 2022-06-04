INTERDEV=/dev/input/by-path/platform-i8042-serio-0-event-kbd
BUILDDIR=/home/jjj/git/capsable/build
sleep 1
echo capsable go...
sudo $BUILDDIR/intercept -g $INTERDEV | $BUILDDIR/capsable | $BUILDDIR/uinput -d $INTERDEV
