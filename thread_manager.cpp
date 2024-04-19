#include <ncurses.h>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <vector>
#include <iostream>
#include <math.h>
#include "ball.h"
#define PI 3.14159

class thread_manager
{
	int maxx, maxy;
	volatile bool should_run = true;

	std::mutex balls_mutex;	 // mutex guarding the access to the list of balls and their threads
	std::mutex A_zone_mutex; // mutex guarding acess to the first zone - part of the second assignment

	std::vector<ball *> balls;
	std::vector<std::thread *> thread_pool;

	// initializes colour pairs of the ncurses library
	void col_init()
	{
		start_color();
		init_pair(0, 1, 0);
		init_pair(1, 2, 0);
		init_pair(2, 3, 0);
		init_pair(3, 4, 0);
		init_pair(4, 5, 0);
		init_pair(5, 6, 0);
	}

	// function for the thread that gathers all user input via keyboard
	void key_listening_loop()
	{
		while (should_run)
		{
			int key_pressed = getch();
			// exit
			if (key_pressed == ' ')
				should_run = false;
		}
	}

	// function for the thread that handles all display functionality
	void display_loop(std::vector<ball *> &balls)
	{
		while (should_run)
		{
			clear();

			// display screen size
			printw("%d\t%d", maxx, maxy);

			// display the zone borders
			for (int i = 1; i <= 2; i++)
			{
				for (int j = 0; j < maxy; j++)
				{
					mvprintw(j, i * maxx / 3, "|");
				}
			}

			balls_mutex.lock();
			// display balls
			for (ball *b : balls)
			{
				// do not display dead balls
				if (b->alive == false)
				{
					continue;
				}
				int px = b->get_X();
				int py = b->get_y();
				int col = b->get_color();
				int sprite = b->get_life();
				// int sprite = b->get_zone();

				attron(COLOR_PAIR(col));
				mvprintw(py, px, "%d", sprite);
				attroff(COLOR_PAIR(col));
			}
			balls_mutex.unlock();

			refresh();
			// about 50 Hz refresh rate
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
		}
	}

	// function for the thread that destroys dead balls and creates new ones
	void ball_managing_loop(std::vector<ball *> &balls, std::vector<std::thread *> &thread_pool)
	{
		int counter = 0;
		while (should_run)
		{
			// destroy dead balls and their threads
			balls_mutex.lock();
			for (unsigned int i = 0; i < balls.size();)
			{
				if (balls[i]->alive == false)
				{
					// allow thread to end naturally
					thread_pool[i]->join();

					// delete ball
					delete balls[i];
					balls.erase(balls.begin() + i);

					// delete thread
					delete thread_pool[i];
					thread_pool.erase(thread_pool.begin() + i);
				}
				else
				{
					i++;
				}
			}

			// create a new ball
			int px = maxx / 2;
			int py = (maxy / 4) * 3;

			int dir_x = rand() % 3 - 1;
			int dir_y = -1;

			ball *b = new ball(px, py, dir_x, dir_y, 3.0, counter % 6, maxx, maxy, &A_zone_mutex);

			balls.push_back(b);									   // store the new ball
			thread_pool.push_back(new std::thread(&ball::run, b)); // assign a thread to it

			balls_mutex.unlock(); // release owenership of the ball vector

			counter++;
			counter %= 5;

			// run every 3 seconds
			std::this_thread::sleep_for(std::chrono::milliseconds(3000));
		}
	}

public:
	
	int run()
	{
		// key listening thread start
		std::thread key_listening_thread(&thread_manager::key_listening_loop, this);

		// display thread start
		// std::thread display_thread(display_loop, std::ref(balls));
		std::thread display_thread(&thread_manager::display_loop, this, std::ref(balls));

		// ball managing thread start
		std::thread ball_managing_thread(&thread_manager::ball_managing_loop, this, std::ref(balls), std::ref(thread_pool)); // std::ref is a wrapper class that is needed to pass
																															 // vectors by reference to new threads

		// await for the end of execution (governed by the keyboard-listening thread via the should_run variable)
		key_listening_thread.join();

		// let all the ball threads die on their own terms
		for (unsigned int i = 0; i < balls.size(); i++)
		{
			balls[i]->alive = false;
		}

		display_thread.join();
		ball_managing_thread.join();
		
		for (unsigned int i = 0; i < thread_pool.size(); i++)
		{
			thread_pool[i]->join();
		}
		// from this point onwards this is the only thread alive

		// cleanup
		while (!balls.empty())
		{
			ball *b = balls.back();
			balls.pop_back();
			delete b;
		}

		while (!thread_pool.empty())
		{
			std::thread *t = thread_pool.back();
			thread_pool.pop_back();
			delete t;
		}
		endwin();
		return 0;
	}

	thread_manager()
	{
		// initialization
		srand(time(NULL));
		initscr();
		getmaxyx(stdscr, maxy, maxx);
		noecho();
		curs_set(0);
		col_init();
	}
};

int main()
{
	thread_manager app;
	app.run();
	return 0;
}