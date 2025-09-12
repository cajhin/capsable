#you have to figure out which input your keyboard is. Look into /dev/input, or use evtest tool
#bluetooth devices are not listed in /by-path
#INTERDEV=/dev/input/event4  #apple magic keyboard?
#INTERDEV=/dev/input/event6  #apple air keyboard
#INTERDEV=/dev/input/by-path/platform-i8042-serio-0-event-kbd  #VirtualBox, Dell 9350, Thinkpad

if [ -e /dev/input/by-path/platform-i8042-serio-0-event-kbd ]
then
  INTERDEV=/dev/input/by-path/platform-i8042-serio-0-event-kbd  #VirtualBox, Dell 9350, Thinkpad
else
  echo "unknown event#, figure out the name with 'evtest' tool"
  exit
fi

#if the enter key is down on startup -> hangs (interception uses different device?)
echo -n 'please release all keys'
for (( i=0; i<10; ++i )); do
    echo -n '.'
    sleep 0.05
done

BUILDDIR=$(dirname $0)/build
sudo nice -n -20 $BUILDDIR/intercept -g $INTERDEV | nice -n -20 $BUILDDIR/capsable | nice -n -20 $BUILDDIR/uinput -d $INTERDEV
