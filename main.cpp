// SPDX-FileCopyrightText: 2024 Nico Schumann <nico.schumann@startmail.com>
// SPDX-License-Identifier: MIT

// Hunt the Wumpus! ( https://en.wikipedia.org/Hunt_the_Wumpus )
// My implementation in C++, according to description at wikipedia article.
// No external libraries needed. Code is public domain.

// Refined from some suggesteions at https://www.cplusplus.com/forum/general/280996/
// Thank you all :-)

#include <iostream>
#include <string>
#include <sstream>
#include <ctime>
#include <cstdlib>

constexpr int CAVE_SIZE = 20;    // Count of rooms inside the cave.
constexpr int NO_OF_PITS = 2;    // Number of pits.
constexpr int NO_OF_BATS = 2;    // Number of bats.
constexpr int NO_OF_ARROWS = 5;  // Number of arrows.
constexpr int EDGES = 3;         // Number of edges (doors) of each node (room).
constexpr int ARROW_TRAVERSALS = 4;  // Number of caves which may be traversed by an arrow.


// This is the graphical correlation of its edges of each room of the cave.
// The numbers of a row represent the connected rooms of a room.
// There are 'CAVE_SIZE' rooms, each of them has 'EDGES' nighbour rooms.
// All in all must the length of the array be of 'CAVE_SIZE'.
// The graph below represents a dodecahedron.
constexpr int CONNECTIONS [CAVE_SIZE][EDGES] =
  { { 13, 16, 19 },
    {  2,  8,  5 },
    {  1,  3, 10 },
    {  2,  4, 12 },
    {  3,  5, 14 },
    {  1,  4,  6 },
    {  5,  7, 15 },
    {  6,  8, 17 },
    {  1,  7,  9 },
    {  8, 10, 18 },
    {  2,  9, 11 },
    { 10, 12, 19 },
    {  3, 11, 13 },
    { 12, 14,  0 },
    {  4, 13, 15 },
    {  6, 14, 16 },
    { 15, 17,  0 },
    {  7, 16, 18 },
    {  9, 17, 19 },
    { 11, 18,  0 } };

// This content could be inside a room.
enum Content { EMPTY, BAT, PIT };

Content cave[CAVE_SIZE] = { EMPTY };

std::ostream & operator<<( std::ostream & os, const Content c ) {
    // Thank you @seeplus for the idea using a lookup array.
    constexpr const char * const lookup[] = { "EMPTY", "WUMPUS", "BAT", "PIT" };
    return os << lookup[c];
}

struct Game
{
    Game() 
    : os{std::cout}, is{std::cin}, player(std::cout)
    {
        // Populate cave
        std::srand(0);
        for (int i = 0; i < NO_OF_BATS; ++i)
        {
            cave[std::rand() % CAVE_SIZE] = BAT;
        }
        for (int i = 0; i < NO_OF_PITS; ++i)
        {
            cave[std::rand() % CAVE_SIZE] = PIT;
        }

        // Set room no of Wumpus
        while (true) {
            auto room_no = rand() % CAVE_SIZE;
            if (cave[room_no] == EMPTY) {
                wumpus.room_no = room_no;
                break;
            }
        } 

        // Set room no of player
        while (true) {
            auto room_no = rand() % CAVE_SIZE;
            if (cave[room_no] == EMPTY && room_no != wumpus.room_no) {
                player.room_no = room_no;
                break;
            }
        }
    }

    struct Wumpus
    {
        int room_no;

        void  move()
        {
            do
            {
                int edge = std::rand() % EDGES;
                if (cave[CONNECTIONS[this->room_no][edge]] == EMPTY)
                {
                    room_no = CONNECTIONS[this->room_no][edge];
                    break;
                }
            } while (true);
        }

    } wumpus;

    struct Player
    {
        int room_no;
        int arrows;  // No of arrows inside the player's quiver.
        std::ostream &os;
        int room_to_move;
        int arrow_traversal_rooms[ARROW_TRAVERSALS] = {-1};
 
    

        Player(std::ostream &ostr) 
        : os (ostr), arrows(NO_OF_ARROWS)
        {}

        void move()
        {
            for (int edge = 0; edge < EDGES; ++edge)
            {
                if (room_to_move == CONNECTIONS[room_no][edge])
                {
                    room_no = room_to_move;
                    return;
                }
            }
            os << "Whoops! You just tried to walk through solid rock :-)\n";
        }

    } player;

    enum Directive {NONE, MOVE, SHOOT} player_command;


    std::ostream & os;
    std::istream & is;
    bool does_game_run = true;
 

    // Handling user input.
    void handle_input()
    {
        while (true)
        {
            player_command = NONE;
            os << "Move or shoot (m-s)? ";
            std::string input;
            std::getline(is, input);
            if (input[0] == 'q')
            {
                does_game_run = false;
                return;
            }
            if (!is)
                continue;

            std::istringstream iss(input);
            char ch;
            iss >> ch;

            if (ch != 's' && ch != 'm')
            {
                print_help();
                print_status();
                continue;
            }

            // Handling the 'move' directive.
            if (ch == 'm')
            {
                iss >> player.room_to_move;
                if (!iss)
                    continue;
                player_command = MOVE;
                return;
            }

            // Handling the 'shoot' directive.
            else if (ch == 's')
            {
                if (player.arrows <= 0)
                {
                    os << "You have no arrows in your quiver!\n";
                    continue;
                }

                for(size_t i = 0; i < ARROW_TRAVERSALS; ++ i)
                    player.arrow_traversal_rooms[i] = -1;

                // Read-in of the arrow_traversal rooms form input
                std::size_t pos;
                for (pos = 0; !isdigit(input[pos]); ++pos)
                    ;
                try
                {
                    // Trying to extract numbers from input.
                    for (size_t length_idx = 0; length_idx < ARROW_TRAVERSALS - 1; ++length_idx)
                    {
                        std::size_t processed = 0;
                        player.arrow_traversal_rooms[length_idx] = std::stoi(input.substr(pos), &processed);
                        pos += processed;
                    }
                }
                catch (...)
                {
                }
                player_command = SHOOT;
                return;
            }
            else
                continue; // Wrong command happend.
        }
    }

    // Handle arrow shooting.
    void process_shoot() {

        -- player.arrows;

        size_t len = 0;
        for ( ; len < ARROW_TRAVERSALS; ++ len ) {
            // Check for out of bounds
            if( player.arrow_traversal_rooms[len] < 0 || player.arrow_traversal_rooms[len] >= CAVE_SIZE) {
                player.arrow_traversal_rooms[len] = -1;
                break;
            }
        }

        int tmp_room = player.room_no;
        bool msg_flag = true;      // Helper flag, used to switch messages.
        bool wumpus_kill = false;  // Flags whether player killed the Wumpus.
        bool self_kill = false;    // Flags whether player killed itself.

        for (size_t i = 0; i < len; ++i)
        {
            // The breakout condition
            if( player.arrow_traversal_rooms[i] == -1 ) {
                break;
            }

            bool rand_flag = true;

            for( int edge = 0; edge < EDGES; ++ edge ) {
                // Found an edge match?
                if( player.arrow_traversal_rooms[i] == CONNECTIONS[tmp_room][edge])
                {
                    rand_flag = false;
                    break;
                }
            }

            if( rand_flag ) {
                if( msg_flag ) {
                    os << "You coose the wrong path for your arrow.\n"
                       << "Your arrow fly instead through room ";
                    msg_flag = false;
                }
                tmp_room = CONNECTIONS[tmp_room][ std::rand() % EDGES ];
            }
            else {
                if( msg_flag ) {
                    os << "Your arrow fly through room ";
                    msg_flag = false;
                }
                for( int edge = 0; edge < EDGES; ++ edge ) {
                    if( player.arrow_traversal_rooms[i] == CONNECTIONS[tmp_room][edge]) {
                        tmp_room = CONNECTIONS[tmp_room][edge];
                        break;
                    }
                }
            }
            os << tmp_room;
            if( tmp_room == wumpus.room_no ) wumpus_kill = true;
            if( tmp_room == player.room_no ) self_kill = true;
        }
        os << ".\n";

        if ( wumpus_kill ) {
            os << "Your shoot killed the Wumpus.\n";
            does_game_run = false;
            return;
        }
        if( self_kill ) {
            os << "Your arrow flew in your own room and killed you.\n";
            does_game_run = false;
            return;
        }
        // Move the Wumpus inside the cave to an adjacent room.
        wumpus.move();
        if (wumpus.room_no == player.room_no ) {
            os << "The Wumpus enters the room you stay.\n";
               does_game_run = false;
        }
    }

    void print_status() {
        os << "\n\nYou are now in room " << player.room_no << ".\n"
           << "There are tunnels, leading to rooms ";
        for( int i = 0; i < EDGES; ++ i) {
            os << (EDGES>1&&i==EDGES-1?"and ":"")
               << CONNECTIONS[player.room_no][i] << (i<EDGES-2?",":"") << (i==EDGES-1?"":" ");
        }
        os << ".\nYou have " << player.arrows << " arrow" << (player.arrows==1?"":"s")
           <<" in your quiver.\n";

       for( int edge = 0; edge < EDGES; ++ edge )
           if( CONNECTIONS[player.room_no][edge] == wumpus.room_no )
               os << "It smells somewhat. Maybe a wumpus is near.\n";
       for( int edge = 0; edge < EDGES; ++ edge )
           if( cave[CONNECTIONS[player.room_no][edge]] == BAT )
               os << "I hear a light rustle in the air.\n"
                  << " Maybe there are bats near.\n";
       for( int edge = 0; edge < EDGES; ++ edge )
           if( cave[CONNECTIONS[player.room_no][edge]] == PIT )
               os << "I feel a light cold drought in the air.\n"
                  << " Maybe there is a pit somewhere.\n";
        os << "\n";
    }
    void print_help() {
        os << "\nHint: you need to enter either 'm', followed by a number\n"
              "of the connected rooms, or 's', followed by 1 up to 5 room numbers\n"
              "of consecutive connected rooms. If the entered arrow path is faulty,\n"
              "the arrow fly a random path through the cave.\n"
              "Examples:\n"
              " 'm 4 <Enter>' let you move into room 4 (if possible).\n"
              " 's 3 8 7 <Enter>' tries to shoot through rooms 3, 8 and 7.\n\n";
    }
    void intro() {
        os << "\nYou are an adventurer inside a cave. It's pretty dark.\n"
           << "All you have, is your courage and a bow,"
           << " together with " << NO_OF_ARROWS
           << " arrow" << (NO_OF_ARROWS>1?"s":"") << " in your quiver.";
    }
    void bye() {
        os << "\nbye...\n";
    }

    int run() {
       intro();
       while( does_game_run ) {
           print_status();
           handle_input();
           process_game();
       }
       bye();
       return EXIT_SUCCESS;
    }

    void process_game()
    {

        if (player_command == MOVE)
            player.move();

        else if(player_command == SHOOT) {
            process_shoot();
            if (wumpus.room_no == player.room_no) {
                os << "The Wumpus entered your room and eat you! *crunch*\n";
                does_game_run = false;
            }
        }

        // Handling entering a room with the Wumpus inside.
        if (player.room_no == wumpus.room_no)
        {
            // If the player entered a room with the Wumpus inside.
            if (player_command == MOVE && player.room_no == wumpus.room_no)
            {
               os << "You walked straight ahead into the mouth of the Wumpus! *crunch*\n";
               does_game_run = false;
               return;
            }
            // If the Wumpus entered the room where the player stays.
        }
        // Handling a pit.
        else if( cave[player.room_no] == PIT ) {
            os << "*AAAHhhhhh... ..you felt into a deep deep hole. For the luck you are\n"
               << "instantly dead when you shattered on the bottom.\n";
            does_game_run = false;
            return;
        }
        // Handling entering a room where is a bat:
        else if( cave[player.room_no] == BAT ) {
            cave[player.room_no] = EMPTY;
            while (true) {
                if( auto const idx {rand() % CAVE_SIZE}; cave[idx] == EMPTY ) {
                    player.room_no = cave[idx];
                    os << "A huge bat takes you away and flys with you through the cave.\n";
                    while( true ) {
                        if( const auto idx{rand() % CAVE_SIZE}; cave[idx] == EMPTY ) {
                            cave[idx] = BAT;
                            return;
                        }
                    }
                }
            }
        }
    }
};

std::ostream &operator<<(std::ostream &os, const Content cave[])
{
    os << "room_no\tpr[0]\tpr[1]\tpr[2]\tcontent\n"
          "_______________________________________\n";
    for (int i = 0; i < CAVE_SIZE; ++i)
    {
        os << i;
        for (int k = 0; k < EDGES; ++k)
        {
            os << '\t' << CONNECTIONS[i][k];
        }
        os << '\t' << cave[i] << '\n';
    }
    return os << '\n';
}

int main()
{
    return Game().run();
}
