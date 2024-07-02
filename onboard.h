#pragma once

/**
 * This class must handle the real time car controls
 *
 */

#include "networkpackets.h"
#include "tickableinterface.h"

#include "utils.h"

#include <pigpio.h>

#include <deque>

const uint8_t servopin = 13, ESCpin = 12;

using namespace std;

class Car : public Tickable, public Messageable
{

private:
    uint8_t ESC = 90, SERVO = 90;

public:
    Car()
    {
        ESC = (char)90;
        SERVO = (char)90;

        cout << "GPIO\n";
        gpioInitialise();
        resetServoESC();
    }
    Server *server = nullptr;
    void setSocket(Server *s)
    {
        server = s;
    }
    bool updated = 0;
    unsigned long last_message = 0;

    std::deque<unsigned int> qq;

    double timeaverage = 0;
    void doAverage(unsigned long value)
    {
        if (qq.size() >= 10)
            qq.pop_front();

        qq.push_back(value);

        unsigned int sum = 0;
        for (auto &i : qq)
            sum += i;
        timeaverage = sum / (double)qq.size();
    }

    void tick(double delta)
    {
        // cout << "CAR tick, delta: " << delta << "\n";
        if ((currentMillis() - last_message > 300 && timeaverage > 100) || ((int)(currentMillis() - last_message)) > ((int)mapvalues(90 - abs(90 - ESC), 0, 90, 300, 1000)))
        {
            if (!updated && (ESC != 90 || SERVO != 90))
            {
                updated = 1;
                ESC = 90;
                // SERVO = 90;
                cout << "Packet late, resetting" << endl;
            }
        }

        if (updated)
        {
            updated = 0;

            // write esc and servo
            // cout << "ESC " << (int)ESC << " SERVO " << (int)SERVO;

            int SERVOwidth = SERVO < 90 ? mapvalues(SERVO, 0, 90, 970, 1500) : mapvalues(SERVO, 90, 180, 1500, 1900);
            int ESCwidth = ESC < 90 ? mapvalues(ESC, 0, 90, 1000, 1500) : mapvalues(ESC, 90, 180, 1500, 2000);

            // cout << " Real width (constrained): ESC " << (int)ESCwidth << " SERVO " << (int)SERVOwidth << endl;

            gpioServo(servopin, SERVOwidth);
            gpioServo(ESCpin, ESCwidth);
        }
    }

    void onExit()
    {
        resetServoESC();
    }

    void resetServoESC()
    {
        gpioServo(servopin, 1500);
        gpioServo(ESCpin, 1500);
    }

    void message(MessageStruct m)
    {
        doAverage(currentMillis() - last_message);
        last_message = currentMillis();
        cout << "Time average: " << timeaverage << endl;

        int l = strlen(m.values);
        for (int i = 0; i < l; i++)
            cout << (int)m.values[i] << "\t";

        ESC = ((char)181) - m.values[0];
        SERVO = ((char)181) - m.values[1];
        updated = 1;

        cout << "\n";
        cout.flush();
    }
};
