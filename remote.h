#pragma once
/**
 * This class must handle the remote.
 * Get gamepad input and send to socket
 * Receive from socket and display to user
 *
 */

#include "networkpackets.h"
#include "tickableinterface.h"
#include "sockets.h"

#include "joystick.h"

#include "utils.h"

using namespace std;

const int servo_margin_limit = 20;

enum PROFILE_PAD
{

    GAMEPAD = 0,
    WHEEL = 1,
	CLASSIC = 2

};
class Remote : public Tickable, public Messageable, public JoystickHandler
{

private:
    bool isESCLock = false;
    uint8_t ESC = 90, SERVO = 90;

    PROFILE_PAD profile = GAMEPAD;

public:
    Remote(PROFILE_PAD setprof = GAMEPAD)
    {
        profile = setprof;
    }

    joystick *j;
    Client *client = nullptr;
    void setJoystick(joystick *c)
    {
        j = c;
    }

    void setSocket(Client *c)
    {
        client = c;
    }
    unsigned int ccc = 0;
    unsigned long last_message = 0;
    void tick(double delta)
    {
        // cout << "REMOTE tick, delta: " << delta << "\n";
        if (currentMillis() - last_message >= 30 && isJoystickConnected)
        {
            last_message = currentMillis();

            MessageStruct ms;
            char *res = nullptr;
            res = new char[3]{
                (char)(1 + ESC),
                (char)(1 + SERVO),
                '\0'};

            strcpy(ms.values, res);
            delete[] res;
            client->send(ms);
        }
    }

    void message(MessageStruct m)
    {
        int l = strlen(m.values);
        for (int i = 0; i < l; i++)
            cout << (int)m.values[i] << "\t";
        cout << "\n";
        cout.flush();
    }

    void receivedButton(const uint8_t button, const short value)
    {

        cout << "Button " << (int)button << " : " << (value ? "pressed" : "released") << "\n";
        switch (profile)
        {

        case WHEEL:
            switch (button)
            {

            default:
                break;

            // Slow forward
            case 7:
                isESCLock = (bool)value;
                ESC = value ? 85 : 90;
                break;

            // Slow backwards
            case 6:
                isESCLock = (bool)value;
                ESC = value ? 103 : 90;
                break;
            }
            break;

        case GAMEPAD:
            switch (button)
            {

            default:
                break;

            // Slow forward
            case 7:
                isESCLock = (bool)value;
                ESC = value ? 85 : 90;
                break;

            // Slow backwards
            case 6:
                isESCLock = (bool)value;
                ESC = value ? 103 : 90;
                break;
            }
            break;

			case CLASSIC:

					switch (button) {
					case 8:
						if (value)
						classic_servo_offset--;
						break;
					case 9:
						if (value)
						classic_servo_offset++;
						break;
					case 10:
						if (value)
						classic_servo_offset = 0;
						break;
						// Slow forward
					case 11:
						isESCLock = (bool) value;
						ESC = value ? 85 : 90;
						break;

						// Slow backwards
					case 12:
						isESCLock = (bool) value;
						ESC = value ? 103 : 90;
						break;
					}
				break;
			}

        cout << std::endl;
        cout.flush();
    }

    int brake = 0, accel = 0;
    int classic_servo_offset = 0, classic_restriction = 0;
    void receivedAxis(const uint8_t axis, const short x, const short y)
    {

        //printf("Axis %zu at (%6d, %6d)\n", axis, x, y);
        // mapvalues(x, -14000, 14000, 0, 180)
        //                         mapvalues(y, -14000, 14000, 0, 180)

        switch (profile)
        {

        case WHEEL:
            switch (axis)
            {
            case 0:

                SERVO = mapvalues(x, -32767, 32767, 0 + servo_margin_limit, 180 - servo_margin_limit);
                accel = 100 - mapvalues(y, -32767, 32767, 0, 100);
                break;
            case 1:

                brake = 100 - mapvalues(x, -32767, 32767, 0, 100);
                break;
            }
            // if(!isESCLock)
            ESC = 180 - mapvalues(accel - brake, -100, 100, 0, 180);
            break;

        case GAMEPAD:

            switch (axis)
            {
            case 0:
                if (isESCLock)
                    return;
                ESC = mapvalues(y, -32767, 32767, 0, 180) -1;
                if(ESC==91)ESC=90;
                break;
            case 1:
                SERVO = mapvalues(x, -32767, 32767, 0 + servo_margin_limit, 180 - servo_margin_limit) -1;
                if(SERVO==91)SERVO=90;
                return;
            }
            break;
			case CLASSIC:

				switch (axis) {
				case 0:
					if (isESCLock)
						return;
					ESC = mapvalues(y, -32767, 32767, 0+classic_restriction, 180-classic_restriction) - 1;
					if (ESC == 91 || ESC == 89)
						ESC = 90;
					break;
				case 1:
					classic_restriction = mapvalues(x, -32767, 32767, 60, 0);
					SERVO = mapvalues(y, -32767, 32767, 0 + servo_margin_limit,
							180 - servo_margin_limit) - 1 + classic_servo_offset;
					if (SERVO == 91 || SERVO == 89)
						SERVO = 90;
					return;
				}
				break;
        }

        cout << "ESC: " << (int)ESC << " SERVO: " << (int)SERVO;
        cout << std::endl;
        cout.flush();
    }
};
