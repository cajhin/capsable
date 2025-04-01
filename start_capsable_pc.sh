if [ -e /usr/lib64/libyaml-cpp.so.0.6 ]
then
  YAMLVERSION=yaml06
elif [ -e /usr/lib64/libyaml-cpp.so.0.7 ]
then
  YAMLVERSION=yaml07
elif [ -e /usr/lib64/libyaml-cpp.so.0.8 ]
then
  YAMLVERSION=yaml08
else
  echo "unknown version of libyaml-cpp"
  exit
fi

if [ -e /dev/input/by-path/platform-i8042-serio-0-event-kbd ]
then
  INTERDEV=/dev/input/by-path/platform-i8042-serio-0-event-kbd  #VirtualBox, Dell 9350, Thinkpad
else
  echo "unknown event#, figure out the name with 'evtest' tool"
  exit
fi
#bluetooth devices are not listed in /by-path
#INTERDEV=/dev/input/event4  #apple magic keyboard?
#INTERDEV=/dev/input/event6  #apple air keyboard

#if the enter key is down on startup -> hangs (interception uses different device?)
echo -n 'please release all keys'
for (( i=0; i<10; ++i )); do
    echo -n '.'
    sleep 0.05
done

BUILDDIR=$(dirname $0)/build
sudo nice -n -20 $BUILDDIR/intercept -g $INTERDEV | nice -n -20 $BUILDDIR/capsable | nice -n -20 $BUILDDIR/uinput_$YAMLVERSION -d $INTERDEV
