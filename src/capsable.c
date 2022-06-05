#include <stdio.h>
#include <stdlib.h>
#include <linux/input.h>
#include <unistd.h>

#define DEBUG if(0)  
#define VERSION 4


const struct input_event sync_event = {.type = EV_SYN, .code = SYN_REPORT,   .value = 0};
struct input_event event, modEvent;
int modmask = 0;

void setModdedKey(int _modmask, unsigned short key)
{
    modmask = _modmask;
    event.code = key;
}

void produceModdedKey()
{
        modEvent.type = EV_KEY;

        if (modmask & 0b0001) {
            DEBUG fprintf(stderr, "modmask lshf");
            modEvent.code = KEY_LEFTSHIFT;
        }
        if (modmask & 0b0010) {
            DEBUG fprintf(stderr, "modmask lctrl");
            modEvent.code = KEY_LEFTCTRL;
        }

        modEvent.value = 1;
        modEvent.time.tv_sec = event.time.tv_sec;
        modEvent.time.tv_usec = event.time.tv_usec - 1;
        if (event.time.tv_usec == 0)
            modEvent.time.tv_sec--;
        fwrite(&modEvent, sizeof(modEvent), 1, stdout);

        event.value = 1;
        fwrite(&event, sizeof(event), 1, stdout);

        event.value = 0;
        event.time.tv_usec++;
        fwrite(&event, sizeof(event), 1, stdout);

        fwrite(&sync_event, sizeof(sync_event), 1, stdout);
        usleep(25000);

        modEvent.value = 0;
        modEvent.time.tv_sec = event.time.tv_sec;
        modEvent.time.tv_usec = event.time.tv_usec + 1;
        if (modEvent.time.tv_usec == 0)
            modEvent.time.tv_sec+=1;
        fwrite(&modEvent, sizeof(modEvent), 1, stdout);
}



int main(void) 
{

    DEBUG fprintf(stderr, "CAPSABLE %i STARTED\n", VERSION);

    setbuf(stdin, NULL), setbuf(stdout, NULL);

    int capsIsDown = 0;
    int escIsDown = 0;
    int altIsDown = 0;

    while (fread(&event, sizeof(event), 1, stdin) == 1) 
    {
        modmask = 0;

        if(event.type != EV_KEY) {
            fwrite(&event, sizeof(event), 1, stdout);
            continue;
        }

        if (event.code == KEY_ESC ||
            event.code == KEY_F1) {
            escIsDown = event.value == 0 ? 0 : 1;
            DEBUG fprintf(stderr, "ESC/F1 escIsDown:%u\n", escIsDown);
            fwrite(&event, sizeof(event), 1, stdout);
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
            event.code = KEY_RIGHTALT;
        else if (event.code == KEY_SLASH)
            event.code = KEY_RIGHTSHIFT;
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
                event.code = KEY_LEFT;
            else if (event.code == KEY_L)
                event.code = KEY_RIGHT;
            else if (event.code == KEY_I)
                event.code = KEY_UP;
            else if (event.code == KEY_K)
                event.code = KEY_DOWN;
            else if (event.code == KEY_O)
                event.code = KEY_PAGEUP;
            else if (event.code == KEY_DOT)
                event.code = KEY_PAGEDOWN;
            else if (event.code == KEY_H)
                event.code = KEY_BACKSPACE;
            else if (event.code == KEY_SEMICOLON)
                event.code = KEY_DELETE;
            else if (event.code == KEY_Z)
                event.code = KEY_HOME;
            else if (event.code == KEY_U)
                event.code = KEY_END;
            else if (event.code == KEY_N) 
                setModdedKey(2,KEY_LEFT);
            else if (event.code == KEY_M) 
                setModdedKey(2,KEY_RIGHT);
            else if (event.code == KEY_A) 
                setModdedKey(2, KEY_Z);
            else if (event.code == KEY_S)
                setModdedKey(2, KEY_X);
            else if (event.code == KEY_D)
                setModdedKey(2, KEY_C);
            else if (event.code == KEY_F)
                setModdedKey(2, KEY_V);
        }

        //alt chars
        if(altIsDown)
        {
                 if (event.code == KEY_Q) 
                setModdedKey(1, KEY_1);
            else if (event.code == KEY_W) 
                setModdedKey(1, KEY_2);
            else if (event.code == KEY_E) 
                setModdedKey(1, KEY_3);
            else if (event.code == KEY_R) 
                setModdedKey(1, KEY_4);
            else if (event.code == KEY_T) 
                setModdedKey(1, KEY_5);
            else if (event.code == KEY_Z) //TODO read original event
                setModdedKey(1, KEY_6);
            else if (event.code == KEY_U) 
                setModdedKey(1, KEY_7);
            else if (event.code == KEY_I) 
                setModdedKey(1, KEY_8);
            else if (event.code == KEY_O) 
                setModdedKey(1, KEY_9);
            else if (event.code == KEY_P) 
                setModdedKey(1, KEY_0);
            else if (event.code == KEY_A) 
                setModdedKey(0, KEY_MINUS);
            else if (event.code == KEY_S) 
                setModdedKey(1, KEY_EQUAL);
            else if (event.code == KEY_D) 
                setModdedKey(1, KEY_8);
            else if (event.code == KEY_F) 
                setModdedKey(0, KEY_SLASH);
            else if (event.code == KEY_G) 
                setModdedKey(0, KEY_EQUAL);
            else if (event.code == KEY_H) 
                setModdedKey(0, KEY_BACKSPACE);
            // else if (event.code == KEY_J) 
            //     setModdedKey(1, KEY_);
            else if (event.code == KEY_K) 
                setModdedKey(1, KEY_LEFTBRACE);
            else if (event.code == KEY_L) 
                setModdedKey(1, KEY_RIGHTBRACE);
            else if (event.code == KEY_Y) //TODO read orig event not rewired
                setModdedKey(0, KEY_GRAVE);
            else if (event.code == KEY_X) 
                setModdedKey(1, KEY_GRAVE);
            else if (event.code == KEY_C) 
                setModdedKey(1, KEY_BACKSLASH);
            else if (event.code == KEY_V) 
                setModdedKey(1, KEY_MINUS);
            // else if (event.code == KEY_) 
            //     setModdedKey(1, KEY_);
            else if (event.code == KEY_N) 
                setModdedKey(1, KEY_COMMA);
            else if (event.code == KEY_M) 
                setModdedKey(1, KEY_DOT);
            else if (event.code == KEY_COMMA) 
                setModdedKey(0, KEY_LEFTBRACE);
            else if (event.code == KEY_DOT) 
                setModdedKey(0, KEY_RIGHTBRACE);
            // else if (event.code == KEY_) 
            //     setModdedKey(1, KEY_);
        }

        DEBUG fprintf(stderr, "code: %u - %i\n", event.code, event.value);

        //write
        if (modmask)
        {
            if(event.value)  //whole combo is written on key down
                produceModdedKey(modmask);
        }
        else
            fwrite(&event, sizeof(event), 1, stdout);
    }
}

