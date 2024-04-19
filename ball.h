#include <string>
#include <mutex>
#pragma once
// class representing a bouncing ball
class ball
{
private:
    volatile int position_x, position_y;
    volatile int direction_x, direction_y;
    volatile int deflection_counter = 6; // how many times can the ball hit the edge before dying
    volatile int color;                  // accepts values from <0, 5>
    double delay;                        // how long traversing one pixel takes, in ms per character
    volatile int zone;
    int maxx, maxy; // max screen width and height (used by the movement logic)
    std::mutex* A_zone_mutex;

    void update_zone();
    int check_next_zone();

public:
    volatile bool alive = true; // change to false to kill the thread of this ball

    // getters
    int get_X() volatile { return position_x; };
    int get_y() volatile { return position_y; };
    int get_life() volatile { return deflection_counter; };
    int get_zone() volatile { return zone; };
    int get_color() volatile { return color; };

    void run(); // movement logic

    std::string to_string();

    /**
     * A constructor that specifies parameters of the newly created ball.
     * distance units: chracters
     * velocity units: characters / second
     * direction_i - values from {-1, 0, 1}
     * color - value from <0, 5>
     * maxx, maxy - screen dimensions
     */
    ball(int x, int y, int dir_x, int dir_y, double velocity, int desired_color, int maxx, int maxy, std::mutex* A_zone_mutex);
};