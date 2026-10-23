# planner

A terminal planner in C/ncurses: academic year calendar, goals, daily
habits, tasks and notes per day, all editable without leaving the
terminal.

I started this project in the summer of 2023, and this is the latest
version :)

![Calendar and goals](docs/calendar-and-goals.png)

## Install

You need ncurses (with wide character/UTF-8 support):

	sudo apt-get install libncurses-dev

## Build and run

	make
	./planner

The terminal window needs to be at least 53 lines x 203 columns (the
program tells you the exact size if it doesn't fit, and exits without
corrupting anything -- just make the window bigger and run `./planner`
again).

	make clean    # removes the executable and the .o files

## Views

| View | What it has |
|---|---|
| `CALENDAR AND GOALS` | academic year calendar (July to June) + 108 goals |
| `EVENTS: JULY TO DECEMBER` | one note per day, from July to December |
| `EVENTS: JANUARY TO JUNE` | one note per day, from January to June |
| `TODAY` | current and next month, daily habits (3 columns, 40 each) and tasks |

<details>
<summary>Screenshots of the 4 views</summary>

**CALENDAR AND GOALS**
![Calendar and goals](docs/calendar-and-goals.png)

**EVENTS: JULY TO DECEMBER**
![Events: July to December](docs/events-jul-dec.png)

**EVENTS: JANUARY TO JUNE**
![Events: January to June](docs/events-jan-jun.png)

**TODAY**
![Today](docs/today.png)

</details>

## Commands

	Arrows             move
	A / S              switch view
	ENTER              edit / stop editing the selected item
	Arrows (editing)   move the cursor within the text
	Home / End         go to the start / end of the text
	Delete             delete the character ahead of the cursor
	Ctrl+U             clear all the text
	SPACEBAR           change the color of the selected item
	Q                  quit

Goals, habits and tasks only toggle between red (to do) and
green (done) -- events and notes cycle through the 6 available colors
(no color, red, green, yellow, cyan, blue).

## Data

Everything lives in `~/.config/planner/`, one plain text file per
category (`<color><TAB><text>` per line) -- easy to open, inspect
or edit by hand. Daily habits reset to red every new day; everything
else stays as you left it.

## Stars

[![a104437ana/planner stars](https://repo-stars.vercel.app/api/stars-badge?repo=a104437ana%2Fplanner)](https://repo-stars.vercel.app)
[![a104437ana/planner stars](https://repo-stars.vercel.app/api/svg?repo=a104437ana%2Fplanner)](https://repo-stars.vercel.app)

---

<a href="https://star-history.com/#a104437ana/planner&Date">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="https://api.star-history.com/svg?repos=a104437ana/planner&type=Date&theme=dark" />
    <source media="(prefers-color-scheme: light)" srcset="https://api.star-history.com/svg?repos=a104437ana/planner&type=Date" />
    <img alt="Star History Chart" src="https://api.star-history.com/svg?repos=a104437ana/planner&type=Date" />
  </picture>
</a>
