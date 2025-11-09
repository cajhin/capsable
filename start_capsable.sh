 #!/bin/bash

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo "Error: This script must be run as root (use sudo)"
    exit 1
fi

  # Check if --apple flag is passed : find first apple keyboard and switch cmd/alt
  FIND_APPLE=false
  if [ "$1" = "--apple" ]; then
      FIND_APPLE=true
  fi

  # Find first keyboard
  find_keyboard() {
      local find_apple=$1

      awk -v find_apple="$find_apple" '
          /^I: Bus=/ {
              # Extract bus and vendor ID
              match($0, /Bus=([0-9a-f]+)/, bus)
              bus_id = bus[1]
              match($0, /Vendor=([0-9a-f]+)/, vendor)
              vendor_id = vendor[1]
              found=1
              next
          }
          found && /^N: Name=/ {
              # Extract device name (remove quotes)
              match($0, /"([^"]+)"/, name_match)
              device_name = name_match[1]
          }
          found && /^P: Phys=/ && $2 != "" {has_phys=1}
          found && /^H: Handlers=/ && has_phys {
              # Only consider USB (0003) or Bluetooth (0005) devices
              is_real_device = (bus_id == "0003" || bus_id == "0005")
              is_apple = (vendor_id == "05ac")
              is_keyboard = ($0 ~ /kbd/)

              # Match based on what we are looking for
              if (is_real_device && is_keyboard && ((find_apple == "true" && is_apple) || (find_apple == "false" && !is_apple))) {
                  match($0, /event[0-9]+/)
                  if (RSTART) {
                      print substr($0, RSTART, RLENGTH)
                      print device_name
                      exit
                  }
              }
          }
          /^$/ {found=0; has_phys=0; vendor_id=""; device_name=""; bus_id=""}
      ' /proc/bus/input/devices
  }

  # If --apple flag is set, check for persistent symlink first
  if [ "$FIND_APPLE" = true ] && [ -e "/dev/input/apple-keyboard" ]; then
      KEYBOARD_PATH="/dev/input/apple-keyboard"
      # Get the actual event device for display purposes
      EVENT_NUM=$(basename $(readlink -f /dev/input/apple-keyboard))
      KEYBOARD_NAME="Apple Keyboard (via persistent symlink)"
      echo "Found persistent apple-keyboard symlink"
  else
      # Fall back to searching
      RESULT=$(find_keyboard "$FIND_APPLE")

      if [ -z "$RESULT" ]; then
          if [ "$FIND_APPLE" = true ]; then
              echo "no apple keyboard found"
          else
              echo "no non-apple keyboard found"
          fi
          exit 1
      fi

      EVENT_NUM=$(echo "$RESULT" | head -1)
      KEYBOARD_NAME=$(echo "$RESULT" | tail -1)
      KEYBOARD_PATH="/dev/input/$EVENT_NUM"
  fi

  echo "Using: $KEYBOARD_PATH ($KEYBOARD_NAME)"

#if the enter key is down on startup -> hangs. So wait for release of Enter.
echo -n 'please release all keys'
for (( i=0; i<10; ++i )); do
    echo -n '.'
    sleep 0.05
done
echo

BUILDDIR=$(dirname $0)/build

# Main loop - restart if keyboard disconnects, but exit on Ctrl-C
while true; do
    echo "Starting capsable... ($KEYBOARD_PATH)"
    sudo nice -n -20 $BUILDDIR/intercept -g $KEYBOARD_PATH | nice -n -20 $BUILDDIR/capsable $1 | nice -n -20 $BUILDDIR/uinput -d $KEYBOARD_PATH
    PIPE_STATUSES=("${PIPESTATUS[@]}")  # Save all exit codes immediately
    EXIT_CODE=${PIPE_STATUSES[1]}  # Get exit code from capsable (2nd command in pipeline)
    echo "DEBUG: intercept=${PIPE_STATUSES[0]}, capsable=${PIPE_STATUSES[1]}, uinput=${PIPE_STATUSES[2]}"

    # Exit code 0 means clean exit (ESC+X), 130 means Ctrl-C (SIGINT)
    if [ $EXIT_CODE -eq 0 ]; then
        echo "Stopped cleanly (ESC+X)"
        exit 0
    elif [ $EXIT_CODE -eq 130 ]; then
        echo "Stopped by user (Ctrl-C)"
        exit 0
    elif [ $EXIT_CODE -eq 1 ]; then
        # Keyboard disconnected - wait and restart
        echo "Keyboard disconnected. Waiting for keyboard to wake up..."
        sleep 3
    else
        # Unexpected exit code - don't restart
        echo "Unexpected exit code: $EXIT_CODE. Exiting."
        exit $EXIT_CODE
    fi
done
