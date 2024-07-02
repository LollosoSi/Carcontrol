#include <ctime>    // time()
#include <iostream> // cin, cout, endl
#include <string>   // string, getline()
#include <time.h>   // CLOCK_MONOTONIC_RAW, timespec, clock_gettime()
#include <chrono>
#include <thread>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "tickableinterface.h"
#include "remote.h"
#include "onboard.h"

#include <cstring>
#include <string>

#include "utils.h"

using namespace std;

long min_delta_millis_tick = 5;

bool running = true;
void signalhandler(int s)
{
    printf("Caught signal %d\n", s);
    running = false;
}

Tickable *objectinquestion = nullptr;
void tick(double delta)
{
    objectinquestion->tick(delta);
}

int main(int argc, char **argv)
{

    string device("");
    printf("You have entered %d arguments:\n", argc);
    char mode = '2';
    if (argc >= 2)
    {
        string str(argv[1]);
        if (str.length() == 1)
            mode = str[0];
    }

    while (mode != '1' && mode != '0')
    {
        cout << "Not clear whether this is car or remote. Enter 0 for car or 1 for remote.\nKeep in mind you can determine via CLI by launching ./main 0 or 1.\n";

        cout << "\nEnter 0 or 1:\t";
        cin >> mode;
    }

    int prof = 10;
    std::string ip("");

    Server *s = nullptr;
    Client *c = nullptr;
    Remote *rremote = nullptr;
    Car *ccar = nullptr;
    joystick *j = nullptr;

    switch (mode)
    {
    case '0':
        cout << "Setting up as CAR\n";
        ccar = new Car();
        objectinquestion = (Tickable *)ccar;
        s = new Server((Messageable *)ccar);
        ccar->setSocket(s);
        break;

    case '1':
        cout << "Setting up as REMOTE\n";
        if (argc >= 3)
            device = string(argv[2]);
        while (device == "")
        {
            cout << "Specify joystick position.\nKeep in mind you can determine via CLI by launching ./main 1 /dev/input/jsX.\n";

            cout << "\nEnter /dev/input/js.....:\t";
            getline(cin, device);

            if (!strcmp(device.c_str(), "default"))
                device = "/dev/input/js0";
        }

        if (argc >= 4)
            prof = atoi(string(argv[3]).c_str());
        while (prof != GAMEPAD && prof != WHEEL)
        {
            cout << "Specify joystick type.\n0 for Gamepad\t1 for wheel\nKeep in mind you can determine via CLI by launching ./main 1 /dev/input/jsX 0/1.\n";

            cout << "\nEnter profile:\t";
            cin >> prof;
        }
        if (argc >= 5)
            ip = string(argv[4]);
        while (ip == "")
        {
            cout << "Specify target address.\nKeep in mind you can determine via CLI by launching ./main 1 /dev/input/jsX 0/1 192.168.x.y.\n";

            cout << "\nEnter 192.168.?.?......:\t";
            getline(cin, ip);
        }

        rremote = new Remote((PROFILE_PAD)prof);
        j = new joystick(device, (JoystickHandler *)rremote);
        rremote->setJoystick(j);
        objectinquestion = (Tickable *)rremote;
        c = new Client((Messageable *)rremote, ip);
        rremote->setSocket(c);
        break;
    }

    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = signalhandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);

    long current = currentMillis();

    long last_millis_tick = current;
#ifdef DEBUG
    int ticks = 0;
    long deltasum = 0;
    long deltas = 0;
    long last_log = current;
    int delta_log = 0;
#endif

    int delta_tick;
    while (running)
    {

        /** LOOP RELATED **/
        current = currentMillis();

        if ((delta_tick = current - last_millis_tick) >= min_delta_millis_tick)
        {

            // deltasum += delta_tick;

            last_millis_tick = current;
#ifdef DEBUG
            deltas++;
            ticks++;
#endif

            tick(delta_tick / 1000.0);
        }

#ifdef DEBUG
        if (deltas > 0 && (delta_log = current - last_log) >= 1000)
        {
            last_log = current;
            std::cout << "DeltaAvg: " << (deltasum / deltas) << "\t";
            std::cout << "Ticks: " << ticks << std::endl;
            ticks = 0;
        }
#endif
        /** END LOOP **/

        std::this_thread::sleep_for(std::chrono::nanoseconds(10));
    }

    if (s)
    {
        s->closeSocket();
        ccar->onExit();
    }
    if (c)
    {
        c->closeSocket();
        j->closeJoystick();
    }

    return 0;
}
