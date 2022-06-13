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
- sudo dnf (apt) install yaml-cpp  (currently, Ubuntu installs 0.7 and Fedora 0.6, annoying)
- sudo ~/git/capsable/start_caps.sh  
  (or sudo ~/git/capsable/start_caps_apple-bt.sh for Apple keyboards to switch FN/LCtrl and ALT/META)
- maybe you have to adjust the event numbers in the start script, e.g. `INTERDEV=/dev/input/event6`  
  install and run 'evtest' if you cannot figure out the event# of your keyboard
