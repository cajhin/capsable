#include <stdio.h>
#include <stdlib.h>
#include <linux/input.h>
#include <unistd.h>

#define DEBUG if(1)  
#define VERSION 2

int main(void) 
{

    DEBUG fprintf(stderr, "CAPSABLE %i STARTED\n", VERSION);

    setbuf(stdin, NULL), setbuf(stdout, NULL);

    const struct input_event 
        sync_event = {.type = EV_SYN, .code = SYN_REPORT,   .value = 0};

    struct input_event event, modEvent;

    int capsIsDown = 0;
    int escIsDown = 0;
    int altIsDown = 0;
    unsigned int modmask = 0;

    while (fread(&event, sizeof(event), 1, stdin) == 1) 
    {
        modmask = 0;

        if(event.type != EV_KEY)
        {
            fwrite(&event, sizeof(event), 1, stdout);
            continue;
        }

        if (event.code == KEY_F2) {  //todo ESC on baremetal F2 in VM
            escIsDown = event.value == 0 ? 0 : 1;
            DEBUG fprintf(stderr, "F2 escIsDown:%u\n", escIsDown);
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
            else if (event.code == KEY_N) {
                if(!event.value)
                    continue;
                modmask = 0b0010;
                event.code = KEY_LEFT;
            }
            else if (event.code == KEY_M) {
                if(!event.value)
                    continue;
                modmask = 0b0010;
                event.code = KEY_RIGHT;
            }
            else if (event.code == KEY_A) {
                if(!event.value)
                    continue;
                modmask = 0b0010;
                event.code = KEY_Z;
            }
            else if (event.code == KEY_S) {
                if(!event.value)
                    continue;
                modmask = 0b0010;
                event.code = KEY_X;
            }
            else if (event.code == KEY_D) {
                if(!event.value)
                    continue;
                modmask = 0b0010;
                event.code = KEY_V;
            }
            else if (event.code == KEY_F) {
                if(!event.value)
                    continue;
                modmask = 0b0010;
                event.code = KEY_V;
            }
        }

        //alt chars
        if(altIsDown)
        {
            if (event.code == KEY_F)
                event.code = KEY_SLASH;
            else if (event.code == KEY_Q) {
                modmask = 0b0001;
                event.code = KEY_1;
            }
        }

        //try out with numbers
        else if (event.code == KEY_1)  {
            if(capsIsDown)
                event.code = KEY_A;
        }
        else
            DEBUG fprintf(stderr, "other code: %u - %i\n", event.code, event.value);



        if (modmask)
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
        else
            fwrite(&event, sizeof(event), 1, stdout);
    }

}

