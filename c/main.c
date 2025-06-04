#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define CAVE_SIZE 20    // Count of rooms inside the cave.
#define NO_OF_PITS 2    // Number of pits.
#define NO_OF_BATS 2    // Number of bats.
#define NO_OF_ARROWS 5  // Number of arrows.
#define EDGES 3         // Number of edges (doors) of each node (room).
#define ARROW_TRAVERSALS 3  // Number of rooms which may be traversed by an arrow.

// Graphical correlation of edges of each room of the cave.
const int CONNECTIONS[CAVE_SIZE][EDGES] = {
    {13, 16, 19}, {2, 8, 5}, {1, 3, 10}, {2, 4, 12}, {3, 5, 14},
    {1, 4, 6}, {5, 7, 15}, {6, 8, 17}, {1, 7, 9}, {8, 10, 18},
    {2, 9, 11}, {10, 12, 19}, {3, 11, 13}, {12, 14, 0}, {4, 13, 15},
    {6, 14, 16}, {15, 17, 0}, {7, 16, 18}, {9, 17, 19}, {11, 18, 0}
};

// Content inside a room.
typedef enum { EMPTY, BAT, PIT } Content;
// Player command.
typedef enum { NONE, MOVE, SHOOT } Command;

Content cave[CAVE_SIZE] = { EMPTY };

typedef struct {
    int room_no;
} Wumpus;

typedef struct {
    Command command;
    int room_no;
    int arrows;
    int room_to_move;
    int arrow_traversal_rooms[ARROW_TRAVERSALS];
} Player;

typedef struct {
    Wumpus wumpus;
    Player player;
    bool does_game_run;
} Game;

// Function prototypes
void initialize_game(Game *game);
void handle_input(Game *game);
void process_shoot(Game *game);
void process_move(Game *game);
void process_game(Game *game);
void print_status(Game *game);
void print_help();
void intro();
void bye();

void initialize_game(Game *game) {
    game->does_game_run = true;
    game->player.command = NONE;
    game->player.arrows = NO_OF_ARROWS;

    // Populate cave
    for (int i = 0; i < NO_OF_BATS; ++i) {
        cave[rand() % CAVE_SIZE] = BAT;
    }
    for (int i = 0; i < NO_OF_PITS; ++i) {
        cave[rand() % CAVE_SIZE] = PIT;
    }

    // Set room number of Wumpus
    while (true) {
        int room_no = rand() % CAVE_SIZE;
        if (cave[room_no] == EMPTY) {
            game->wumpus.room_no = room_no;
            break;
        }
    }

    // Set room number of player
    while (true) {
        int room_no = rand() % CAVE_SIZE;
        if (cave[room_no] == EMPTY && room_no != game->wumpus.room_no) {
            game->player.room_no = room_no;
            break;
        }
    }
}

void handle_input(Game *game) {
    char input[256];
    while (true) {
        game->player.command = NONE;

        printf("Move or shoot (m-s)? ");
        if (!fgets(input, sizeof(input), stdin)) {
            continue;
        }

        if (input[0] == 'q') {
            game->does_game_run = false;
            return;
        }

        char command;
        int room;

        if (sscanf(input, "%c %d", &command, &room) == 2) {
            if (command == 'm') {
                game->player.command = MOVE;
                game->player.room_to_move = room;
                return;
            } else if (command == 's') {
                if (game->player.arrows <= 0) {
                    printf("You have no arrows in your quiver!\n");
                    continue;
                }
                game->player.command = SHOOT;

                // reset arrow_traversal_rooms
                for (int i = 0; i < ARROW_TRAVERSALS; ++i) {
                    game->player.arrow_traversal_rooms[i] = -1;
                }

                char *token = strtok(input + 2, " ");
                for (int i = 0; i < ARROW_TRAVERSALS && token; ++i) {
                    game->player.arrow_traversal_rooms[i] = atoi(token);
                    token = strtok(NULL, " ");
                }
                return;
            }
        }
        print_help();
        print_status(game);
    }
}

void process_shoot(Game *game) {
    game->player.arrows--;

    int tmp_room = game->player.room_no;
    bool wumpus_kill = false;
    bool self_kill = false;
    bool bat_kill = false;
    bool pit_kill = false;

    for (int i = 0; game->player.arrow_traversal_rooms[i] != -1; ++i) {
        int target_room = game->player.arrow_traversal_rooms[i];
        if (target_room < 0 || target_room >= CAVE_SIZE) {
            break;
        }

        bool valid_path = false;
        for (int edge = 0; edge < EDGES; ++edge) {
            if (target_room == CONNECTIONS[tmp_room][edge]) {
                valid_path = true;
                tmp_room = target_room;
                break;
            }
        }

        printf("The arrow flew through room ");
        for (int i = 0; game->player.arrow_traversal_rooms[i] != -1; ++i) {
            printf("%d ", game->player.arrow_traversal_rooms[i]);
            if (game->player.arrow_traversal_rooms[i] == game->wumpus.room_no) {
                wumpus_kill = true;
                break;
            }
            else if (game->player.arrow_traversal_rooms[i] == game->player.room_no) {
                self_kill = true;
                break;
            }
            else if (cave[game->player.arrow_traversal_rooms[i]] == BAT) {
                bat_kill = true;
                cave[game->player.arrow_traversal_rooms[i]] = EMPTY;
                break;
            }
            else if (cave[game->player.arrow_traversal_rooms[i]] == PIT) {
                pit_kill = true;
                break;
            }
        }
        printf(".\n");
    }

    if (bat_kill) {
        printf("*iiik*...\n");
    }
    if (pit_kill) {
        printf("*clonk*....\n");
    }
    if (wumpus_kill) {
        printf("Your shoot killed the Wumpus.\n");
        game->does_game_run = false;
        return;
    }
    if (self_kill) {
        printf("Your arrow flew in your own room and killed you.\n");
        game->does_game_run = false;
        return;
    }

    // Move the Wumpus
    int edge = rand() % EDGES;
    game->wumpus.room_no = CONNECTIONS[game->wumpus.room_no][edge];
    if (game->wumpus.room_no == game->player.room_no) {
        printf("The Wumpus enters the room you stay.\n");
        game->does_game_run = false;
    }
}

void process_move(Game *game) {
    bool tunnel_found = false;
    for (int edge = 0; edge < EDGES; ++edge) {
        if (game->player.room_to_move == CONNECTIONS[game->player.room_no][edge])
        {
            game->player.room_no = game->player.room_to_move;
            tunnel_found = true;
            break;
        }
    }
    if (!tunnel_found)
    {
        printf("Whoops! You just tried to walk through solid rock :-)\n");
    }
}

void process_game(Game *game) {
    if (game->player.command == MOVE) {
        process_move(game);
    }    
    else if(game->player.command == SHOOT) {
        process_shoot(game);
    }

    if (game->player.room_no == game->wumpus.room_no) {
        printf("You walked straight ahead into the mouth of the Wumpus! *crunch*\n");
        game->does_game_run = false;
    } else if (cave[game->player.room_no] == PIT) {
        printf("*AAAHhhhhh... ..you fell into a deep hole and died.\n");
        game->does_game_run = false;
    } else if (cave[game->player.room_no] == BAT) {
        cave[game->player.room_no] = EMPTY;
        game->player.room_no = rand() % CAVE_SIZE;
        printf("A huge bat takes you away and flies with you through the cave.\n");
    }
}


void print_status(Game *game) {
    printf("\n\nYou are now in room %d.\n", game->player.room_no);
    printf("There are tunnels leading to rooms ");
    for (int i = 0; i < EDGES; ++i) {
        printf("%d%s", CONNECTIONS[game->player.room_no][i], i == EDGES - 1 ? ".\n" : ", ");
    }
    printf("You have %d arrow%s in your quiver.\n", game->player.arrows, game->player.arrows == 1 ? "" : "s");
    for( int edge = 0; edge < EDGES; ++edge )
           if( CONNECTIONS[game->player.room_no][edge] == game->wumpus.room_no )
               printf("It smells somewhat. Maybe a wumpus is near.\n");
       for( int edge = 0; edge < EDGES; ++ edge )
           if( cave[CONNECTIONS[game->player.room_no][edge]] == BAT )
                printf("I hear a light rustle in the air.\n"
                     " Maybe there are bats near.\n");
       for( int edge = 0; edge < EDGES; ++ edge )
           if( cave[CONNECTIONS[game->player.room_no][edge]] == PIT )
                printf( "I feel a light cold drought in the air.\n"
                        " Maybe there is a pit somewhere.\n");
        printf("\n");
}

void print_help() {
    printf("\nHint: rooms are numbered from 0 to %d.\n"
           "Enter 'm' followed by a room number to move, or 's' followed by up to 3 room numbers to shoot.\n"
           "Example for move:  m 3 <Enter>\n"
           "Example for shoot: s 1 3 8 <Enter>\n\n",
           CAVE_SIZE - 1);
}

void intro() {
    printf("\nYou are an adventurer inside a cave. It's pretty dark.\n");
    printf("All you have is your courage and a bow, together with %d arrow%s in your quiver.\n",
           NO_OF_ARROWS, NO_OF_ARROWS > 1 ? "s" : "");
    printf("Rooms are numbered 0 through %d.\n", CAVE_SIZE - 1);
}

void bye() {
    printf("\nbye...\n");
}

int main() {

#ifdef DEBUG
    srand(0);
#else
    srand(time(NULL));
#endif

    Game game;
    initialize_game(&game);

    intro();
    while (game.does_game_run) {
        print_status(&game);
        handle_input(&game);
        process_game(&game);
    }
    bye();

    return EXIT_SUCCESS;
}