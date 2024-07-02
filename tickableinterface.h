#pragma once

class Tickable
{
public:
    virtual void tick(double delta) = 0;
};