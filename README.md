## Instructions to build

Clone the repository and cd into it via the terminal & execute the following commands:

```jsx
mkdir build
cd build
cmake ..;make
```

This builds the application and run `./app`  to get the game up & running.

## Characters Present in the game

### User

Always present on the left of the screen and moves in the following way:

**spacebar to move up**  

**release spacebar to come down.**

→ The movement of the user takes into account the effect of gravity(harder to go up with resistance and free falling effect on the way down).

### Coins

Coins come towards the user from the right and upon colliding with it, the count of coins collected so far increases.

### Zappers

Zappers spin and come towards the user from the right and upon colliding with it, the game is over.

## Scaling the level of difficulty

The game consists of 3 levels with an increasing amount of duration for each level. 

At each level, the number of zappers increases, making it hard for the user to navigate across the screen.

If all the levels are successfully completed without any zapper collision in between, then the user wins the game else he loses upon collision.
