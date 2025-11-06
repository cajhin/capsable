#include <stdio.h>
#include <stdlib.h>
#include <linux/input.h>
#include <unistd.h>
#include <string.h>

#define DEBUG if (0)
#define VERSION "v19 ctrl and shift cancels alt2mod9"

// pause between key sends. Fuzzy. VSCode needs no sleep, gnome apps a lot? VirtualBox a lot lot
#define DEFAULT_SLEEP_BETWEEN_KEYS_US 6000

// needs to be in sync with compose key configured in Gnome. Used for öäü
const unsigned short COMPOSE_KEY = KEY_RIGHTMETA;

// EV_SYN is sent between key 'batches' (to help the OS process multi-byte events)
const struct input_event syn_report = {.type = EV_SYN, .code = SYN_REPORT, .value = 0};

struct input_event event, ev_out, previous_event, previous_event_tmp;

int key_handled = 0;
int SLEEP_BETWEEN_KEYS_US = DEFAULT_SLEEP_BETWEEN_KEYS_US;

void sleepBetweenKeys()
{
    usleep(SLEEP_BETWEEN_KEYS_US);
}

int isKeycodeModifier(unsigned short code)
{
    switch (code)
    {
    case KEY_LEFTSHIFT:
    case KEY_LEFTCTRL:
    case KEY_LEFTMETA:
    case KEY_LEFTALT:
    case KEY_RIGHTSHIFT:
    case KEY_RIGHTCTRL:
    case KEY_RIGHTMETA:
    case KEY_RIGHTALT:
    case KEY_FN:
        return 1;
    }
    return 0;
}

void writeSyn()
{
    // syn_report.time
    fwrite(&syn_report, sizeof(syn_report), 1, stdout);
}

void writeKeyOverride(unsigned short code, signed int value)
{
    DEBUG fprintf(stderr, "KEY out: %u - %i\n", code, value);
    key_handled = 1;
    ev_out.code = code;
    ev_out.value = value;
    ev_out.time = event.time;
    ev_out.type = event.type;
    fwrite(&ev_out, sizeof(ev_out), 1, stdout);
    writeSyn();
}

void writeKey(unsigned short code)
{
    writeKeyOverride(code, event.value);
}

void writeKeyMakeBreak(unsigned short code)
{
    writeKeyOverride(code, 1);
    sleepBetweenKeys();
    writeKeyOverride(code, 0);
}

void writeModdedKeyOverride(int modmask, unsigned short code, signed int value)
{
    key_handled = 1;
    if (!value) // entire sequence is written on key down
        return;

    unsigned short modcode;

    if (modmask & 0b0001)
    {
        DEBUG fprintf(stderr, "modmask lshf");
        modcode = KEY_LEFTSHIFT;
    }
    else if (modmask & 0b010000)
    {
        DEBUG fprintf(stderr, "modmask rshf");
        modcode = KEY_RIGHTSHIFT;
    }
    else if (modmask & 0b0010)
    {
        DEBUG fprintf(stderr, "modmask lctrl");
        modcode = KEY_LEFTCTRL;
    }
    else
        modcode = 0;

    if (modcode)
    {
        writeKeyOverride(modcode, 1);
        sleepBetweenKeys();
    }

    writeKeyMakeBreak(code);

    if (modcode)
    {
        writeKeyOverride(modcode, 0);
        sleepBetweenKeys();
    }
}

void writeModdedKey(int modmask, unsigned short code)
{
    writeModdedKeyOverride(modmask, code, event.value);
}

void compose(int modmask1, unsigned short key1, int modmask2, unsigned short key2)
{
    if (!event.value) // compose everything on key down
        return;

    writeKeyOverride(COMPOSE_KEY, 1);
    writeKeyOverride(COMPOSE_KEY, 0);
    writeModdedKeyOverride(modmask1, key1, 1);
    writeModdedKeyOverride(modmask2, key2, 1);
}
// óäääÄÄÄJörn

int capsLockState = 0; // weak. Assumes shift lock is off on startup
void setCapsLockState(int newCapsLockState)
{
    if (newCapsLockState != capsLockState)
    {
        capsLockState = newCapsLockState;
        writeKeyMakeBreak(KEY_CAPSLOCK);
    }
}

void handleEscapeDownCombo(unsigned short ecode) 
{
    switch (ecode)
    {
        case KEY_DOT:
            SLEEP_BETWEEN_KEYS_US += 1000;
            fprintf(stderr, "Sleep:%d\n", SLEEP_BETWEEN_KEYS_US);
        break;
        case KEY_COMMA:
            if(SLEEP_BETWEEN_KEYS_US >= 1000)
            {
                SLEEP_BETWEEN_KEYS_US -= 1000;
                fprintf(stderr, "Sleep:%d\n", SLEEP_BETWEEN_KEYS_US);
            }
        break;
    }
}


int main(int argc, char **argv)
{
    fprintf(stderr, "\nCAPSABLE %s STARTED\n", VERSION);
    setbuf(stdin, NULL), setbuf(stdout, NULL);

    int escIsDown = 0;
    int capsIsDown = 0;
    int tabIsDown = 0;
    int altIsDown = 0;
    int lshiftIsDown = 0;
    int rshiftIsDown = 0;
    int lctrlIsDown = 0;

    int keyboardIsApple = 0;
    if (argc > 1 && strcmp("--apple", argv[1]) == 0)
    {
        keyboardIsApple = 1;
        DEBUG fprintf(stderr, "--apple mode\n");
    }

    while (fread(&event, sizeof(event), 1, stdin) == 1)
    {
        key_handled = 0;

        if (event.type == EV_MSC && event.code == MSC_SCAN)
            continue;

        if (event.type == EV_SYN)
        {
            DEBUG fprintf(stderr, " SYN: %i\n", event.code);
            fwrite(&event, sizeof(event), 1, stdout);
            continue;
        }

        if (event.type != EV_KEY)
        {
            DEBUG fprintf(stderr, "UNEXPECTED EV TYPE: %i\n", event.type);
            fwrite(&event, sizeof(event), 1, stdout);
            continue;
        }

        previous_event.code = previous_event_tmp.code;
        previous_event_tmp.code = event.code;
        DEBUG fprintf(stderr, " IN: %i\n", event.code);

        if (keyboardIsApple)
        {
            if (event.code == KEY_LEFTALT)
                event.code = KEY_LEFTMETA;
            else if (event.code == KEY_LEFTMETA)
                event.code = KEY_LEFTALT;
            else if (event.code == KEY_RIGHTALT)
                event.code = KEY_RIGHTMETA;
            else if (event.code == KEY_RIGHTMETA)
                event.code = KEY_RIGHTALT;
//            else if (event.code == KEY_FN)
//                event.code = KEY_LEFTCTRL;
//            else if (event.code == KEY_LEFTCTRL)
//                event.code = KEY_FN;
        }

        // ESC, VIRTUAL MODIFIERS
        if (event.code == KEY_ESC)
        {
            escIsDown = event.value == 0 ? 0 : 1;
            DEBUG fprintf(stderr, "ESC escIsDown:%u\n", escIsDown);
            writeKey(event.code);
            continue;
        }
        else if (escIsDown)
        {
            if(event.code == KEY_X)
            {
                fprintf(stderr, "capsable exit\n");
                break;
            }
            if(event.value != 0)
                handleEscapeDownCombo(event.code);
            continue;
        }
        else if (event.code == KEY_TAB)
        {
            DEBUG fprintf(stderr, "prev:%d",previous_event.code);
            tabIsDown = event.value == 0 ? 0 : 1;
            if(!tabIsDown && previous_event.code == KEY_TAB)
                writeKeyMakeBreak(KEY_TAB);
            continue;
        }
        else if (event.code == KEY_CAPSLOCK)
        {
            capsIsDown = event.value == 0 ? 0 : 1;
            continue;
        }
        else if (event.code == KEY_RIGHTALT || event.code == KEY_LEFTALT)
        {
            altIsDown = event.value == 0 ? 0 : 1;
            continue;
        }

        // REWIRE
        else if (event.code == KEY_RIGHTCTRL)
            event.code = KEY_LEFTALT;
        else if (event.code == KEY_SLASH)
            event.code = KEY_RIGHTSHIFT;
        else if (event.code == KEY_BACKSLASH)
            event.code = KEY_ENTER;
        else if (event.code == KEY_102ND) // ISO boards left <>
            event.code = KEY_LEFTSHIFT;
        else if (event.code == KEY_Y)
            event.code = KEY_Z;
        else if (event.code == KEY_Z)
            event.code = KEY_Y;

        // set CapsLock state with LSHF+RSHF
        if (event.code == KEY_LEFTSHIFT)
        {
            lshiftIsDown = event.value;
            if (rshiftIsDown && lshiftIsDown)
                setCapsLockState(0);
        }
        else if (event.code == KEY_RIGHTSHIFT)
        {
            rshiftIsDown = event.value;
            if (rshiftIsDown && lshiftIsDown)
                setCapsLockState(1);
        }
        //remember if ctrl is down; it cancels the alt reassignment
        else if (event.code == KEY_LEFTCTRL)
        {
            lctrlIsDown = event.value == 0 ? 0 : 1;
        }

        // CAPS cursor, ASDF
        if (capsIsDown && event.value)
        {
            //cursor control
            if (event.code == KEY_J)
                writeKeyMakeBreak(KEY_LEFT);
            else if (event.code == KEY_L)
                writeKeyMakeBreak(KEY_RIGHT);
            else if (event.code == KEY_I)
                writeKeyMakeBreak(KEY_UP);
            else if (event.code == KEY_K)
                writeKeyMakeBreak(KEY_DOWN);
            else if (event.code == KEY_O)
                writeKeyMakeBreak(KEY_PAGEUP);
            else if (event.code == KEY_DOT)
                writeKeyMakeBreak(KEY_PAGEDOWN);
            else if (event.code == KEY_H)
                writeKeyMakeBreak(KEY_BACKSPACE);
            else if (event.code == KEY_SEMICOLON)
                writeKeyMakeBreak(KEY_DELETE);
            else if (event.code == KEY_Z)
                writeKeyMakeBreak(KEY_HOME);
            else if (event.code == KEY_U)
                writeKeyMakeBreak(KEY_END);
            else if (event.code == KEY_N)
                writeModdedKey(2, KEY_LEFT);
            else if (event.code == KEY_M)
                writeModdedKey(2, KEY_RIGHT);
            else if (event.code == KEY_COMMA)
                writeModdedKey(1, KEY_RIGHT);
                
            // classic ctrl+x functions
            else if (event.code == KEY_A)
                writeModdedKey(2, KEY_Z);
            else if (event.code == KEY_S)
                writeModdedKey(2, KEY_X);
            else if (event.code == KEY_D)
                writeModdedKey(2, KEY_C);
            else if (event.code == KEY_F)
                writeModdedKey(2, KEY_V);
            else if (event.code == KEY_G)
                writeModdedKey(2, KEY_Y);
            else if (!isKeycodeModifier(event.code))
                continue; //drop undefined caps+X combos
        }
        // ALT CHARS !@#$%^&()
        else if (altIsDown && !lshiftIsDown && !rshiftIsDown && !lctrlIsDown && event.value)
        {
            if (event.code == KEY_Q)
                writeModdedKey(1, KEY_1);
            else if (event.code == KEY_W)
                writeModdedKey(1, KEY_2);
            else if (event.code == KEY_E)
                writeModdedKey(1, KEY_3);
            else if (event.code == KEY_R)
                writeModdedKey(1, KEY_4);
            else if (event.code == KEY_T)
                writeModdedKey(1, KEY_5);
            else if (event.code == KEY_Z) // TODO read original event
                writeModdedKey(1, KEY_6);
            else if (event.code == KEY_U)
                writeModdedKey(1, KEY_7);
            else if (event.code == KEY_I)
                writeModdedKey(1, KEY_9);
            else if (event.code == KEY_O)
                writeModdedKey(1, KEY_0);
            else if (event.code == KEY_P)
                writeModdedKey(1, KEY_SLASH);
            else if (event.code == KEY_A)
                writeModdedKey(0, KEY_MINUS);
            else if (event.code == KEY_S)
                writeModdedKey(1, KEY_EQUAL);
            else if (event.code == KEY_D)
                writeModdedKey(1, KEY_8);
            else if (event.code == KEY_F)
                writeModdedKey(0, KEY_SLASH);
            else if (event.code == KEY_G)
                writeModdedKey(0, KEY_EQUAL);
            else if (event.code == KEY_H)
                writeModdedKey(0, KEY_BACKSPACE);
            else if (event.code == KEY_J)
                writeModdedKey(0, KEY_BACKSLASH);
            else if (event.code == KEY_K)
                writeModdedKey(1, KEY_LEFTBRACE);
            else if (event.code == KEY_L)
                writeModdedKey(1, KEY_RIGHTBRACE);
            else if (event.code == KEY_Y) // TODO read orig event not rewired
                writeModdedKey(0, KEY_GRAVE);
            else if (event.code == KEY_X)
                writeModdedKey(1, KEY_GRAVE);
            else if (event.code == KEY_C)
                writeModdedKey(1, KEY_BACKSLASH);
            else if (event.code == KEY_V)
                writeModdedKey(1, KEY_MINUS);
            else if (event.code == KEY_B)
                continue;
            else if (event.code == KEY_N)
                writeModdedKey(1, KEY_COMMA);
            else if (event.code == KEY_M)
                writeModdedKey(1, KEY_DOT);
            else if (event.code == KEY_COMMA)
                writeModdedKey(0, KEY_LEFTBRACE);
            else if (event.code == KEY_DOT)
                writeModdedKey(0, KEY_RIGHTBRACE);
            // öäü ÖÄÜ
            else if (event.code == KEY_SEMICOLON)
                compose(0b010000, KEY_APOSTROPHE, 0, KEY_O);
            else if (event.code == KEY_APOSTROPHE)
                compose(0b010000, KEY_APOSTROPHE, 0, KEY_A);
            else if (event.code == KEY_LEFTBRACE)
                compose(0b010000, KEY_APOSTROPHE, 0, KEY_U);
        }
        //tab -> numpad  
        if (tabIsDown && event.value)
        {
            if (event.code == KEY_U)
                writeKeyMakeBreak(KEY_7);
            else if (event.code == KEY_I)
                writeKeyMakeBreak(KEY_8);
            else if (event.code == KEY_O)
                writeKeyMakeBreak(KEY_9);
            else if (event.code == KEY_J)
                writeKeyMakeBreak(KEY_4);
            else if (event.code == KEY_K)
                writeKeyMakeBreak(KEY_5);
            else if (event.code == KEY_L)
                writeKeyMakeBreak(KEY_6);
            else if (event.code == KEY_SEMICOLON)
                writeKeyMakeBreak(KEY_0);
            else if (event.code == KEY_APOSTROPHE)
                writeKeyMakeBreak(KEY_EQUAL);
            else if (event.code == KEY_M)
                writeKeyMakeBreak(KEY_1);
            else if (event.code == KEY_COMMA)
                writeKeyMakeBreak(KEY_2);
            else if (event.code == KEY_DOT)
                writeKeyMakeBreak(KEY_3);
            else if (event.code == KEY_H)
                writeKeyMakeBreak(KEY_BACKSPACE);
            else if (event.code == KEY_P)
                writeKeyMakeBreak(KEY_DOT);
            else if (event.code == KEY_LEFTBRACE)
                writeModdedKey(1, KEY_8);
            else if (event.code == KEY_RIGHTBRACE)
                writeKeyMakeBreak(KEY_SLASH);
            else if (event.code == KEY_Z)
                writeKeyMakeBreak(KEY_MINUS);
            else if (event.code == KEY_N)
                writeKeyMakeBreak(KEY_KPPLUS);
        }

        // write
        // note: explicitly break, even if maybe there was no make, to avoid stuck keys
        if (!key_handled)
            writeKey(event.code);
    }
}
