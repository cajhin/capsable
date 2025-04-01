# capsable
hardcoded kingcon layout on Linux  
(tested on Fedora and Ubuntu)

Prototype, using the Linux version of Interception  
https://gitlab.com/interception

Enables my personal minimum must-have keyboard config.  
For me, this is a prerequisite to use bare metal Linux installations.

Long term it would be nice if it could parse and implement capsicain ini files.
Probably lots of effort and cross-platform trouble.


### Installation
- clone capsable repo to ~/git/  (or copy the binaries in /build/ and start script. If you change the path you must update the start script).
- sudo dnf install yaml-cpp  (currently, Ubuntu installs 0.7 and Fedora 0.6 or 0.8, annoying. Intercept/uinput must be compiled against that 3pp)
- sudo ~/git/capsable/start_capsable_pc.sh (for typical Windows hardware)
  or start_capsable_apple-bt.sh for Apple keyboards to switch FN/LCtrl and ALT/META,
  or start_capsable_macbook.sh (they all have different /dev/input/<event-id>)
- maybe you have to adjust the event numbers in the start script, e.g. `INTERDEV=/dev/input/event6` 
  install and run 'evtest' if you cannot figure out the event# of your keyboard
- for convenience, set `alias sc='sudo /home/jjj/git/capsable/start_capsable_pc.sh'`
