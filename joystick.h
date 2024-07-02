#pragma once
/**
 * Get joystick/wheel input!
 *
 */

#include "tickableinterface.h"

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/joystick.h>

#include <thread>

/**
 * Current state of an axis.
 */
struct axis_state
{
    short x, y;
};

class JoystickHandler
{
public:
    bool isJoystickConnected = false;

    virtual void receivedButton(const uint8_t button, const short value) = 0;
    virtual void receivedAxis(const uint8_t axis, const short x, const short y) = 0;
    virtual void disconnected() { isJoystickConnected = false; }
    virtual void connected() { isJoystickConnected = true; }
};

class joystick
{
private:
public:
    bool running = true;

    const char *device;
    int js;
    struct js_event event;
    struct axis_state axes[3] = {0};
    size_t axis;
    /**
     * Reads a joystick event from the joystick device.
     *
     * Returns 0 on success. Otherwise -1 is returned.
     */
    int read_event(int fd, struct js_event *event)
    {
        ssize_t bytes;

        bytes = read(fd, event, sizeof(*event));

        if (bytes == sizeof(*event))
            return 0;

        /* Error, could not read full event. */
        return -1;
    }

    /**
     * Returns the number of axes on the controller or 0 if an error occurs.
     */
    size_t get_axis_count(int fd)
    {
        __u8 axes;

        if (ioctl(fd, JSIOCGAXES, &axes) == -1)
            return 0;

        return axes;
    }

    /**
     * Returns the number of buttons on the controller or 0 if an error occurs.
     */
    size_t get_button_count(int fd)
    {
        __u8 buttons;
        if (ioctl(fd, JSIOCGBUTTONS, &buttons) == -1)
            return 0;

        return buttons;
    }

    /**
     * Keeps track of the current axis state.
     *
     * NOTE: This function assumes that axes are numbered starting from 0, and that
     * the X axis is an even number, and the Y axis is an odd number. However, this
     * is usually a safe assumption.
     *
     * Returns the axis that the event indicated.
     */
    size_t get_axis_state(struct axis_state axes[3])
    {
        size_t axis = event.number / 2;

        if (axis < 3)
        {
            if (event.number % 2 == 0)
                axes[axis].x = event.value;
            else
                axes[axis].y = event.value;
        }

        return axis;
    }

    JoystickHandler *jshandler = nullptr;

    void
    readAndMessageThread()
    {
        jshandler->connected();
        cout << "Thread starting\n";

        /* This loop will exit if the controller is unplugged. */
        while (read_event(js, &event) == 0)
        {
            switch (event.type)
            {
            case JS_EVENT_BUTTON:
                // printf("Button %u %s\n", event.number, event.value ? "pressed" : "released");
                jshandler->receivedButton(event.number, event.value);
                break;
            case JS_EVENT_AXIS:
                axis = get_axis_state(axes);
                if (axis < 3)
                {
                    short x = axes[axis].x, y = axes[axis].y;
                    uint8_t ax = axis;
                    // printf("Axis %zu at (%6d, %6d)\n", ax, x, y);
                    if (jshandler)
                        jshandler->receivedAxis(ax, x, y);
                }
                break;
            default:
                /* Ignore init events. */
                break;
            }
        }
        jshandler->disconnected();
        std::cout << "Controller disconnected\n";
    }

    joystick(string device, JoystickHandler *jsh)
    {
        js = open(device.c_str(), O_RDONLY);

        jshandler = jsh;

        if (js == -1)
            perror("Could not open joystick");

        cout << "Ready to loop joystick\n";
        std::thread ttt(&joystick::readAndMessageThread, this);
        ttt.detach();
    }

    void closeJoystick()
    {
        cout << "Joystick exiting\n";
        running = false;
        close(js);
    }
};