#include <stdio.h>
#include <stdlib.h>
#include <linux/input.h>
#include <unistd.h>

#define DEBUG if(1)  
#define VERSION 6

//fuzzy. VSCode needs no sleep, gnome apps a lot
#define SLEEP_SHORT_US 2000
#define SLEEP_MEDIUM_US 2000

const struct input_event syn_report = {.type = EV_SYN, .code = SYN_REPORT, .value = 0};
struct input_event event, modEvent;
int key_handled = 0;

// void compose(int modmask1, unsigned short key1, int modmask2, unsigned short key2)
// {
//     writeKey
//     writeModdedKey(modmask1, key1);
//     writeModdedKey(modmask2, key2);
// }

void sleepShort()
{
    usleep(SLEEP_SHORT_US);
}
void sleepMedium()
{
    usleep(SLEEP_MEDIUM_US);
}

void writeSyn()
{
    //syn_report.time
    fwrite(&syn_report, sizeof(syn_report), 1, stdout);
}

void writeKey(unsigned short code)
{ 
    DEBUG fprintf(stderr, "KEY out: %u - %i\n", code, event.value);
    key_handled = 1;
    event.code = code;
    fwrite(&event, sizeof(event), 1, stdout);
    writeSyn();
//    sleepMedium();
}

void writeModdedKey(int modmask, unsigned short code)
{
    key_handled = 1;
    if(!event.value) //entire sequence is written on key down
        return;

    event.code = code;

    modEvent.type = EV_KEY;
    if (modmask & 0b0001) {
        DEBUG fprintf(stderr, "modmask lshf");
        modEvent.code = KEY_LEFTSHIFT;
    }
    else if (modmask & 0b0010) {
        DEBUG fprintf(stderr, "modmask lctrl");
        modEvent.code = KEY_LEFTCTRL;
    }
    else
        modEvent.code = 0;

    if(modEvent.code) {
        modEvent.value = 1;
        modEvent.time.tv_sec = event.time.tv_sec;
        modEvent.time.tv_usec = event.time.tv_usec - 1;
        if (event.time.tv_usec == 0)
            modEvent.time.tv_sec--;
        fwrite(&modEvent, sizeof(modEvent), 1, stdout);
        writeSyn();
        sleepShort();
    }

    event.value = 1;
    writeKey(event.code);
    writeSyn();
    sleepShort();

    event.value = 0;
    event.time.tv_usec++;
    writeKey(event.code);
    writeSyn();
    sleepShort();

    if(modEvent.code) {
        modEvent.value = 0;
        modEvent.time.tv_sec = event.time.tv_sec;
        modEvent.time.tv_usec = event.time.tv_usec + 1;
        if (modEvent.time.tv_usec == 0)
            modEvent.time.tv_sec+=1;
        fwrite(&modEvent, sizeof(modEvent), 1, stdout);
        writeSyn();
        sleepShort();
    }
}



int main(void) 
{

    DEBUG fprintf(stderr, "CAPSABLE %i STARTED\n", VERSION);

    setbuf(stdin, NULL), setbuf(stdout, NULL);

    int escIsDown = 0;
    int capsIsDown = 0;
    int altIsDown = 0;

    while (fread(&event, sizeof(event), 1, stdin) == 1) 
    {
        key_handled = 0;

        if(event.type == EV_MSC && event.code == MSC_SCAN)
            continue;

        if(event.type == EV_SYN) {
            DEBUG fprintf(stderr," SYN: %i\n",event.code);
            fwrite(&event, sizeof(event), 1, stdout);
            continue;
        }

        if(event.type != EV_KEY) {
            DEBUG fprintf(stderr,"UNEXPECTED EV TYPE: %i\n",event.type);
            fwrite(&event, sizeof(event), 1, stdout);
            continue;
        }

        //ESC, MODIFIER
        if (event.code == KEY_ESC ||
            event.code == KEY_F1) {
            escIsDown = event.value == 0 ? 0 : 1;
            DEBUG fprintf(stderr, "ESC/F1 escIsDown:%u\n", escIsDown);
            writeKey(event.code);
            continue;
        }
        else if (escIsDown && event.code == KEY_X)  {
            fprintf(stderr, "capsable exit\n");
            break;
        }
        else if (event.code == KEY_CAPSLOCK) {
            capsIsDown = event.value == 0 ? 0 : 1;
            continue;
        }
        else if (event.code == KEY_RIGHTALT
                || event.code == KEY_LEFTALT) {
            altIsDown = event.value == 0 ? 0 : 1;
            continue;
        }

        //REWIRE
        else if (event.code == KEY_RIGHTCTRL)
            event.code = KEY_LEFTALT;
        else if (event.code == KEY_SLASH)
            event.code = KEY_RIGHTSHIFT;
        else if (event.code == KEY_BACKSLASH)
            event.code = KEY_ENTER;
        else if (event.code == KEY_102ND) //ISO boards left <>
            event.code = KEY_LEFTSHIFT;
        else if (event.code == KEY_Y)
            event.code = KEY_Z;
        else if (event.code == KEY_Z)
            event.code = KEY_Y;

        //CAPS cursor, ASDF
        if(capsIsDown)
        {
            if (event.code == KEY_J)
                writeKey(KEY_LEFT);
            else if (event.code == KEY_L)
                writeKey(KEY_RIGHT);
            else if (event.code == KEY_I)
                writeKey(KEY_UP);
            else if (event.code == KEY_K)
                writeKey(KEY_DOWN);
            else if (event.code == KEY_O)
                writeKey(KEY_PAGEUP);
            else if (event.code == KEY_DOT)
                writeKey(KEY_PAGEDOWN);
            else if (event.code == KEY_H)
                writeKey(KEY_BACKSPACE);
            else if (event.code == KEY_SEMICOLON)
                writeKey(KEY_DELETE);
            else if (event.code == KEY_Z)
                writeKey(KEY_HOME);
            else if (event.code == KEY_U)
                writeKey(KEY_END);
            else if (event.code == KEY_N) 
                writeModdedKey(2,KEY_LEFT);
            else if (event.code == KEY_M) 
                writeModdedKey(2,KEY_RIGHT);
            else if (event.code == KEY_COMMA) 
                writeModdedKey(1,KEY_RIGHT);
            
            else if (event.code == KEY_A) 
                writeModdedKey(2, KEY_Z);
            else if (event.code == KEY_S)
                writeModdedKey(2, KEY_X);
            else if (event.code == KEY_D)
                writeModdedKey(2, KEY_C);
            else if (event.code == KEY_F)
                writeModdedKey(2, KEY_V);
        }
        //ALT CHARS !@#$%^&()
        else if(altIsDown)
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
            else if (event.code == KEY_Z) //TODO read original event
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
            else if (event.code == KEY_Y) //TODO read orig event not rewired
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
    //test
            else if (event.code == KEY_SEMICOLON) 
                writeModdedKey(0, KEY_TOUCHPAD_OFF);
        }

        //write
        if (!key_handled)
            writeKey(event.code);
    }
}

