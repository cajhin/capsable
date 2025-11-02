# capsable
hardcoded 'kingcon' layout on Linux 
(tested on Fedora and Ubuntu)

Prototype, using the Linux version of Interception 
https://gitlab.com/interception
https://wiki.archlinux.org/title/Interception-tools

Enables my personal minimum must-have keyboard config. 
For me, this is a prerequisite to use bare metal Linux installations.

Long term it would be nice if it could parse and implement capsicain ini files.
Probably lots of effort and cross-platform trouble.


### Installation
- clone capsable repo to ~/git/  (or copy the binaries in /build/ and start script. If you change the path you must update the start script).
- sudo ~/git/capsable/start_capsable.sh       # OR
  sudo ~/git/capsable/start_capsable_apple.sh # flips cmd/alt and fn/ctrl keys (i don't like the apple layout on Linux)
- maybe you have to adjust the event numbers in the start script, e.g. 
  `INTERDEV=/dev/input/event14`  # for Apple magic keyboard 2
  install and run 'evtest' if you cannot figure out the event# of your keyboard
- for convenience, set `alias sc='sudo ~/git/capsable/start_capsable.sh'`
- keep it running in a terminal tab

Issue: Apple BT keyboard changes deviceID on every login
Solution: create a fixed udev symlink:
  ⎿  Wrote 3 lines to ../../tmp/99-apple-keyboard.rules
     # Create persistent symlink for Apple Bluetooth Keyboard
     SUBSYSTEM=="input", ATTRS{name}=="jjj2 Keyboard ANSI", ATTRS{uniq}=="e4:8b:7f:2a:f8:f0", KERNEL=="event*", SYMLINK+="input/apple-keyboard"

● To install this rule:
  sudo cp /tmp/99-apple-keyboard.rules /etc/udev/rules.d/
  sudo udevadm control --reload-rules
  sudo udevadm trigger

