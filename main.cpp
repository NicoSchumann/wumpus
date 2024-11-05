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

constexpr int CAVE_SIZE = 20;       // Count of rooms inside the cave.
constexpr int NO_OF_PITS = 2;       // Number of total pits.
constexpr int NO_OF_BATS = 2;       // Number of total bats.
constexpr int NO_OF_ARROWS = 5;     // Number of arrows that player owns.
constexpr int EDGES = 3;            // Number of edges (doors) of each node (room).
constexpr int ARROW_TRAVERSALS = 4; // Number of rooms which may be traversed by an arrow.

// This is the graphical correlation of its edges of each room of the cave.
// The numbers of a row represent the connected rooms of a room.
// There are 'CAVE_SIZE' rooms, each of them has 'EDGES' nighbour rooms.
// All in all must the length of the array be of 'CAVE_SIZE'.
// The graph below represents a dodecahedron.
//
constexpr int CONNECTIONS[CAVE_SIZE][EDGES] =
    {
        {13, 16, 19},
        {2, 8, 5},
        {1, 3, 10},
        {2, 4, 12},
        {3, 5, 14},
        {1, 4, 6},
        {5, 7, 15},
        {6, 8, 17},
        {1, 7, 9},
        {8, 10, 18},
        {2, 9, 11},
        {10, 12, 19},
        {3, 11, 13},
        {12, 14, 0},
        {4, 13, 15},
        {6, 14, 16},
        {15, 17, 0},
        {7, 16, 18},
        {9, 17, 19},
        {11, 18, 0}
    };

// This content could be inside a room.
enum Content
{
    EMPTY,
    WUMPUS,
    BAT,
    PIT
};

std::ostream &operator<<(std::ostream &os, const Content c)
{
    // Thank you @seeplus for your idea using a lookup array.
    constexpr const char *const lookup[] = {"EMPTY", "WUMPUS", "BAT", "PIT"};
    return os << lookup[c];
}

// A cave is structured as a graph, with rooms as nodes and doors at edges.
struct Cave
{
    struct Room
    {
        Room *neighbors[EDGES]; // Connections to other rooms.
        int room_no;
        Content content;
    };

    Room rooms[CAVE_SIZE];
    Room *wumpus_room; // Here resides the Wumpus.

    Cave()
    {
        // Connecting the rooms.
        for (int r = 0; r < CAVE_SIZE; ++r)
        {
            rooms[r].room_no = r + 1;
            for (int e = 0; e < EDGES; ++e)
            {
                rooms[r].neighbors[e] = &rooms[CONNECTIONS[r][e]];
            }
        }

        // Populating the cave.
        while (true)
        {
            // Thank you @seeplus for the hint that if-statements at C++17 could define its
            // own variable in its header.
            if (const auto i{std::rand() % CAVE_SIZE}; rooms[i].content == EMPTY)
            {
                rooms[i].content = WUMPUS;
                wumpus_room = &rooms[i];
                break;
            }
        }
        for (int count = 0; count < NO_OF_PITS; ++count)
        {
            while (true)
            {
                if (const auto i{std::rand() % CAVE_SIZE}; rooms[i].content == EMPTY)
                {
                    rooms[i].content = PIT;
                    break;
                }
            }
        }
        for (int count = 0; count < NO_OF_BATS; ++count)
        {
            while (true)
            {
                if (const auto i{std::rand() % CAVE_SIZE}; rooms[i].content == EMPTY)
                {
                    rooms[i].content = BAT;
                    break;
                }
            }
        }
    }

    ~Cave()
    { /* Nothing to do. */
    }
};

std::ostream &operator<<(std::ostream &os, const Cave &cave)
{

    os << "room_no\tpr[0]\tpr[1]\tpr[2]\tcontent\n"
          "_______________________________________\n";
    for (int i = 0; i < CAVE_SIZE; ++i)
    {
        os << cave.rooms[i].room_no;
        for (int k = 0; k < EDGES; ++k)
        {
            os << '\t' << cave.rooms[i].neighbors[k]->room_no;
        }
        os << '\t' << cave.rooms[i].content << '\n';
    }
    return os << '\n';
}

class Game
{

private:

    enum class Commando
    {
        NONE,
        MOVE,
        SHOOT
    };

    void print_help()
    {
        os << "\nHint: you need to enter either 'm', followed by a number\n"
              "of the connected rooms, or 's', followed by 1 up to 5 room numbers\n"
              "of consecutive connected rooms. If the entered arrow path is faulty,\n"
              "the arrow fly a random path through the cave.\n"
              "Examples:\n"
              " 'm 4 <Enter>' let you move into room 4 (if possible).\n"
              " 's 3 8 7 <Enter>' tries to shoot through rooms 3, 8 and 7.\n\n";
    }
    void intro()
    {
        os << "\nYou are an adventurer inside a cave. It's pretty dark.\n"
           << "All you have, is your courage and a bow,"
           << " together with " << NO_OF_ARROWS
           << " arrow" << (NO_OF_ARROWS > 1 ? "s" : "") << " in your quiver.";
    }
    void bye()
    {
        os << "\nbye...\n";
    }
    void print_status()
    {
        os << "\n\nYou are now in room " << player_room->room_no << ".\n"
           << "There are tunnels, leading to rooms ";
        for (int i = 0; i < EDGES; ++i)
        {
            os << (EDGES > 1 && i == EDGES - 1 ? "and " : "")
               << player_room->neighbors[i]->room_no << (i < EDGES - 2 ? "," : "") << (i == EDGES - 1 ? "" : " ");
        }
        os << ".\nYou have " << arrows << " arrow" << (arrows == 1 ? "" : "s")
           << " in your quiver.\n";

        for (int i = 0; i < EDGES; ++i)
            if (player_room->neighbors[i]->content == WUMPUS)
                os << "It smells somewhat. Maybe a wumpus is near.\n";
        for (int i = 0; i < EDGES; ++i)
            if (player_room->neighbors[i]->content == BAT)
                os << "I hear a light rustle in the air.\n"
                   << " Maybe there are bats near.\n";
        for (int i = 0; i < EDGES; ++i)
            if (player_room->neighbors[i]->content == PIT)
                os << "I feel a light cold drought in the air.\n"
                   << " Maybe there is a pit somewhere.\n";
        os << "\n";
    }

    // Handling user input.
    void handle_input()
    {
        while (true)
        {
            player_commando = Commando::NONE;

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
                iss >> room_to_move;
                if (!iss)
                    continue;

                player_commando = Commando::MOVE;

                return;
            }

            // Handling the 'shoot' directive.
            else if (ch == 's')
            {
                if (arrows <= 0)
                {
                    os << "You have no arrows in your quiver!\n";
                    continue;
                }

                player_commando = Commando::SHOOT;

                // Setting 'pos' to 1st digit.
                std::size_t pos;
                for (pos = 0; !isdigit(input[pos]); ++pos)
                    ;

                // Clearing array of arrow traversal rooms.
                for (int i = 0; i < ARROW_TRAVERSALS; ++i)
                {
                    arrow_traversal_list[i] = 0;
                }

                // Setting array of arrow traversal rooms.
                try
                {
                    // Trying to extract numbers from input.
                    for (int i = 0; i < ARROW_TRAVERSALS; ++i)
                    {
                        std::size_t processed = 0;
                        arrow_traversal_list[i] = std::stoi(input.substr(pos), &processed);
                        pos += processed;
                    }
                }
                catch (...)
                {
                }
                break;
            }
            else
                continue; // Wrong command happend.
        }

        // Trimming the arrow traversal list;
        {
            bool mismatch_flag{false};
            Cave::Room *tmp = player_room;
            int i = 0;
            for (; i < ARROW_TRAVERSALS; ++i)
            {

                if (arrow_traversal_list[i] == 0)
                    break;

                mismatch_flag = true;

                for (int e = 0; e < EDGES; ++e)
                {
                    if (tmp->neighbors[e]->room_no == arrow_traversal_list[i])
                    {
                        mismatch_flag = false;
                        tmp = tmp->neighbors[e];
                        break;
                    }
                }
                if (mismatch_flag)
                {
                    break;
                }
            }
            for (; i < ARROW_TRAVERSALS && arrow_traversal_list[i] != 0; ++i)
            {
                tmp = tmp->neighbors[std::rand() % EDGES];
                arrow_traversal_list[i] = tmp->room_no;
            }
        }
    }

    // Moves the player.
    void process_move()
    {
        for (int i = 0; i < EDGES; ++i)
        {
            if (room_to_move == player_room->neighbors[i]->room_no)
            {
                player_room = player_room->neighbors[i];
                return;
            }
        }
        os << "Whoops! You just tried to walk through solid rock ;-)\n";
        return;
    }

    // Handles arrow shooting.
    void process_shoot()
    {
        --arrows;

        bool wumpus_kill {false},             // Flags whether the arrow killed Wumpus.
             self_kill   {false};               // Flags whether player killed itse

        for( int i = 0; arrow_traversal_list[i] > 0 && i < ARROW_TRAVERSALS; ++i)
        {
            if (arrow_traversal_list[i] == player_room->room_no)
            {
                self_kill = true;
                break;
            }
            else if (cave.rooms[arrow_traversal_list[i]-1].content == WUMPUS)
            {
                wumpus_kill = true;
                break;
            }

        }

        if (wumpus_kill)
        {
            os << "You killed the Wumpus.\n";
            does_game_run = false;
        }
        else if (self_kill)
        {
            os << "Your arrow flew in your own room and killed you.\n";
            does_game_run = false;
        }
        else
        {
            // Moving the Wumpus inside the cave to an adjacent room.
            for (int i = 0; i < EDGES; ++i)
            {
                int e = std::rand() % EDGES;

                if (wumpus_room->neighbors[e]->content == EMPTY)
                {
                    wumpus_room->content = EMPTY;
                    wumpus_room = wumpus_room->neighbors[e];
                    wumpus_room->content = WUMPUS;
                    break;
                }
            }
            if (wumpus_room == player_room)
            {
                os << "The Wumpus enters the room where you stay.\n";
                does_game_run = false;
            }
        }
    }

    void process_game()
    {
        if (does_game_run == false)
            return;

        if (player_commando == Commando::MOVE)
            process_move();

        else if (player_commando == Commando::SHOOT)
            process_shoot();

        // Handling entering a room with the Wumpus inside.
        if (player_room->content == WUMPUS)
        {
            // If the player entered a room with the Wumpus inside.
            if (player_commando == Commando::MOVE)
            {

                os << "You walked straight ahead into the mouth of the Wumpus! *crunch*\n";
                does_game_run = false;
                return;
            }
            // If the Wumpus entered the room where the player stays.
            else if (player_commando == Commando::SHOOT)
            {
                os << "The Wumpus entered your room and eat you! *crunch*\n";
                does_game_run = false;
                return;
            }
        }
        // Handling a pit.
        else if (player_room->content == PIT)
        {
            os << "*AAAHhhhhh... ..you felt into a deep deep hole. For the luck you are\n"
               << "instantly dead when you shattered on the bottom.\n";
            does_game_run = false;
            return;
        }
        // Handling entering a room where is a bat:
        else if (player_room->content == BAT)
        {
            player_room->content = EMPTY;
            while (true)
            {
                if (auto const idx{std::rand() % CAVE_SIZE}; cave.rooms[idx].content == EMPTY)
                {
                    player_room = &cave.rooms[idx];
                    os << "A huge bat takes you away and flys with you through the cave.\n";
                    while (true)
                    {
                        if (const auto idx{rand() % CAVE_SIZE}; cave.rooms[idx].content == EMPTY)
                        {
                            cave.rooms[idx].content = BAT;
                            return;
                        }
                    }
                }
            }
        }
    }

    int arrows; // No of arrows inside in the player's quiver.
    bool does_game_run;
    std::ostream &os;
    std::istream &is;
    Cave cave;
    Cave::Room *player_room;                          // Here resides the player.
    Cave::Room *wumpus_room;                          // Here resides the Wumpus.
    int arrow_traversal_list[ARROW_TRAVERSALS] = {0}; // For the room numbers, the arrow would traverse.
    int room_to_move;
    Commando player_commando;

public:
    Game()
        : arrows{NO_OF_ARROWS}, does_game_run{true}, os{std::cout}, is{std::cin}, wumpus_room{cave.wumpus_room}, player_commando{Commando::NONE}
    {
        // Get randomly an empty room for the player.
        while (true)
        {
            const auto i = std::rand() % CAVE_SIZE;
            if (cave.rooms[i].content == EMPTY)
            {
                player_room = &cave.rooms[i];
                break;
            }
        }
    }
    void run()
    {
        intro();
        while (does_game_run)
        {
            print_status();
            handle_input();
            process_game();
        }
        bye();
    }
};

int main()
{
    std::srand(std::time(nullptr));
    
    Game().run();
}