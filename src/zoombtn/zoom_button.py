import serial
import subprocess
import time

ser = serial.Serial('/dev/cu.usbmodem1301', 9600, timeout=1)

def zoom_is_running():
    result = subprocess.run(["pgrep", "-x", "zoom.us"],
                            stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE)
    return result.returncode == 0

def zoom_in_meeting():
    script = '''
    tell application "System Events"
        tell process "zoom.us"
            set windowList to name of every window
        end tell
    end tell
    '''
    result = subprocess.run(["osascript", "-e", script],
                            stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE)
    output = result.stdout.decode()
    return "Zoom Meeting" in output

def mute_zoom():
    script = '''
    tell application "zoom.us"
        activate
    end tell
    tell application "System Events"
        tell process "zoom.us"
            keystroke "a" using {command down, shift down}
        end tell
    end tell
    '''
    subprocess.run(["osascript", "-e", script])
    ser.write(b"BLINK\n")

def new_meeting():
    script = '''
    tell application "zoom.us"
        activate
    end tell
    tell application "System Events"
        tell process "zoom.us"
            keystroke "v" using {control down, command down}
        end tell
    end tell
    '''
    subprocess.run(["osascript", "-e", script])

def key_down():
    script = '''
    tell application "zoom.us"
        activate
    end tell
    tell application "System Events"
        tell process "zoom.us"
            key down space
        end tell
    end tell
    '''
    subprocess.run(["osascript", "-e", script])

def key_up():
    script = '''
    tell application "System Events"
        tell process "zoom.us"
            key up space
        end tell
    end tell
    '''
    subprocess.run(["osascript", "-e", script])

def toggle_hand():
    script = '''
    tell application "zoom.us"
        activate
    end tell
    tell application "System Events"
        tell process "zoom.us"
            keystroke "y" using {option down}
        end tell
    end tell
    '''
    subprocess.run(["osascript", "-e", script])

def thumbs_up():
    script = '''
    tell application "zoom.us"
        activate
    end tell
    tell application "System Events"
        tell process "zoom.us"
            key code 23 using {command down, option down}
        end tell
    end tell
    '''
    subprocess.run(["osascript", "-e", script])

def end_meeting():
    script = '''
    tell application "zoom.us"
        activate
    end tell
    tell application "System Events"
        tell process "zoom.us"
            keystroke "w" using {command down}
        end tell
    end tell
    '''
    subprocess.run(["osascript", "-e", script])

def quit_zoom():
    subprocess.run(
        ["osascript", "-e", 'tell application "zoom.us" to quit'],
        stderr=subprocess.DEVNULL
    )

last_state = None
last_zoom_check = 0
ZOOM_CHECK_INTERVAL = 1.0

while True:
    if ser.in_waiting:
        line = ser.readline().decode().strip()

        # Mute button messages
        if line == "MUTE":
            if not zoom_is_running():
                ser.write(b"OPENING\n")
                subprocess.Popen(["open", "-a", "zoom.us"])
            elif not zoom_in_meeting():
                new_meeting()
                ser.write(b"NEW_MEETING\n")
            else:
                mute_zoom()
        elif line == "HOLD_START":
            if zoom_in_meeting():
                key_down()
        elif line == "HOLD_END":
            if zoom_in_meeting():
                key_up()

        # Hand button messages
        elif line == "HAND":
            if zoom_in_meeting():
                toggle_hand()
                ser.write(b"HAND_OK\n")
            else:
                ser.write(b"NOT_IN_MEETING\n")
        elif line == "THUMBS_UP":
            if zoom_in_meeting():
                thumbs_up()
                ser.write(b"THUMBS_UP_OK\n")
            else:
                ser.write(b"NOT_IN_MEETING\n")
        elif line == "QUIT_ZOOM":
            if zoom_in_meeting():
                end_meeting()
                ser.write(b"ENDING_CALL\n")
            elif zoom_is_running():
                quit_zoom()
                ser.write(b"QUITTING_ZOOM\n")
            else:
                ser.write(b"NOT_IN_MEETING\n")

    now = time.time()
    if now - last_zoom_check >= ZOOM_CHECK_INTERVAL:
        last_zoom_check = now

        if not zoom_is_running():
            state = "OFF"
        elif zoom_in_meeting():
            state = "CALL_ON"
        else:
            state = "CALL_OFF"

        if state != last_state:
            ser.write((state + "\n").encode())
            last_state = state

    time.sleep(0.05)