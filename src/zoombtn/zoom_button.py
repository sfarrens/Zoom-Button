import serial
import subprocess
import time

# CHANGE this to your actual Arduino port
ser = serial.Serial('/dev/cu.usbmodem1301', 9600, timeout=1)

# --- Check if Zoom process is running ---
def zoom_is_running():
    result = subprocess.run(["pgrep", "-x", "zoom.us"],
                            stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE)
    return result.returncode == 0

# --- Check if in active meeting ---
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

# --- Mute/unmute Zoom regardless of active window ---
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

# --- Hold spacebar down (push to talk) ---
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

# --- Release spacebar ---
def key_up():
    script = '''
    tell application "System Events"
        tell process "zoom.us"
            key up space
        end tell
    end tell
    '''
    subprocess.run(["osascript", "-e", script])

last_state = None
last_zoom_check = 0
ZOOM_CHECK_INTERVAL = 1.0  # seconds

while True:
    # --- Handle button press ---
    if ser.in_waiting:
        line = ser.readline().decode().strip()
        if line == "MUTE":
            mute_zoom()
        elif line == "HOLD_START":
            key_down()
        elif line == "HOLD_END":
            key_up()

    # --- Check Zoom state once per second ---
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