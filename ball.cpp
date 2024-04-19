#include "ball.h"
#include <chrono>
#include <thread>
#include <algorithm>
#include <mutex>

void ball::update_zone()
{
    if (position_x < maxx / 3)
        zone = 0; // A
    else if (position_x < 2 * maxx / 3)
        zone = 1; // B
    else
        zone = 2; // C
}

int ball::check_next_zone()
{
    int next_x = position_x + direction_x;

    if (next_x < maxx / 3)
        return 0; // A
    else if (next_x < 2 * maxx / 3)
        return 1; // B
    else
        return 2; // C
}

std::string ball::to_string()
{
    return "Pos x:\t" + std::to_string(position_x) + "\nPosy:\t" + std::to_string(position_y) +
           "\nLife:\t" + std::to_string(deflection_counter) +
           "\nMaxx:\t" + std::to_string(maxx) + "\nMaxy:\t" + std::to_string(maxy);
}

void ball::run()
{
    std::unique_lock<std::mutex> A_lock(*A_zone_mutex, std::defer_lock);
    while (alive)
    {
        // sleep for delay time - implementation of speed
        // at the beginning to speed up the process of dying
        std::this_thread::sleep_for(std::chrono::milliseconds((int)delay));

        // move
        position_x += direction_x;
        position_y += direction_y;

        // check for wall collision
        if (position_x + 1 > maxx)
        {
            direction_x *= -1;
            deflection_counter--;
        }
        else if (position_x - 1 < 0)
        {
            direction_x *= -1;
            deflection_counter--;
        }

        if (position_y + 1 > maxy)
        {
            direction_y *= -1;
            deflection_counter--;
        }
        if (position_y - 1 < 0)
        {
            direction_y *= -1;
            deflection_counter--;
        }

        // check for zone transitions
        int old_zone = zone;
        update_zone();
        int new_zone = zone;
        if (old_zone != new_zone)
        {
            switch (old_zone)
            {
            case 0:
                // A => B
                delay /= 1.3;
                break;
            case 1:
                switch (new_zone)
                {
                case 0:
                    // B => A
                    delay /= 0.6;
                    break;
                case 2:
                    // B => C
                    delay /= 0.9;
                    break;
                default:
                    break;
                }
                break;
            case 2:
                // C => B
                delay /= 1.2;
                break;
            default:
                break;
            }
        }

        if (check_next_zone() == 0 && zone == 1)
        {
            A_lock.lock();
        }

        if (check_next_zone() == 1 && zone == 0)
        {
            A_lock.unlock();
        }

        // check for death condition
        if (deflection_counter < 0)
            alive = false;

        
    }

    // if you're dead and stil own the A-zone mutex - unlock it
    if (A_lock.owns_lock())
    {
        A_lock.unlock();
    }
    
}

ball::ball(int x, int y, int dir_x, int dir_y, double velocity, int desired_color, int in_maxx, int in_maxy, std::mutex *in_A_zone_mutex)
{
    position_x = x;
    position_y = y;

    direction_x = std::clamp(dir_x, -1, 1); // clamp values
    direction_y = std::clamp(dir_y, -1, 1);

    if (direction_x * direction_y != 0)
    {
        velocity /= 1.4; // diagonal movement normalization
    }

    delay = 1000 / velocity; // ms per character

    color = desired_color % 5; // make sure the color is valid

    maxx = in_maxx;
    maxy = in_maxy;

    A_zone_mutex = in_A_zone_mutex;

    update_zone();
};
