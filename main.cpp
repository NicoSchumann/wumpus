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
constexpr int ARROW_TRAVERSALS = 4; // Number of rooms which may be traversed by an arrow.

constexpr int IS_DEAD = -1;       // This could be used for setting player or wumpus to dead state,
                                  // should be a negative value.
constexpr int INVALID_SHOOT = -1; // For shoot handling, should be a negative value.

constexpr int EDGES = 3; // Number of edges (doors) of each node (room).

// This is the graphical correlation of its edges of each room of the cave.
// The numbers of a row represent the connected rooms of a room.
// There are 'CAVE_SIZE' rooms, each of them has 'EDGES' nighbour rooms.
// All in all must the length of the array be of 'CAVE_SIZE'.
// The graph below represents a dodecahedron.
//
constexpr int CAVE[CAVE_SIZE][EDGES] =
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
    BAT,
    PIT
};


/// @brief Prints  'Content' enum.
///
std::ostream &operator<<(std::ostream &os, const Content content)
{
    // Thank you @seeplus for your idea using a lookup array.
    constexpr const char *const lookup[] = {"EMPTY", "BAT", "PIT"};
    return os << lookup[content];
}

/// @brief The guts of the game.
///
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
        _os << "\nHint: you need to enter either 'm[ove]', followed by a number\n"
              "of the connected rooms, or 's[hoot]', followed by 1 up to 5 room numbers\n"
              "of consecutive connected rooms. If the entered arrow path is faulty,\n"
              "the arrow fly a random path through the cave.\n"
              "Examples:\n"
              " 'm 4 <Enter>' let you move into room 4 (if possible).\n"
              " 's 3 8 7 <Enter>' tries to shoot through rooms 3, 8 and 7.\n"
              "h[elp] prints this help\n"
            << std::endl;
    }


    /// @brief An intro which will printed at game start.
    ///
    void print_intro()
    {
        _os << "\nYou are an adventurer inside a cave. It's pretty dark.\n"
           << "All you have, is your courage and a bow,"
           << " together with " << NO_OF_ARROWS
           << " arrow" << (NO_OF_ARROWS > 1 ? "s" : "") << " in your quiver."
           << std::endl;
    }


    /// @brief Printed at end of game.
    ///
    void bye()
    {
        _os << "\nbye..." << std::endl;
    }


    /// @brief Prints game state for orientation of the player.
    ///
    void print_status()
    {
        _os << "There are tunnels, leading to rooms ";
        for (int i = 0; i < EDGES; ++i)
        {
            _os << (EDGES > 1 && i == EDGES - 1 ? "and " : "")
               << CAVE[_player_room_no][i]
               << (i < EDGES - 2 ? "," : "") 
               << (i == EDGES - 1 ? "" : " ") 
               << "\n";

            switch (CAVE[_player_room_no][i])
            {
                case BAT:
                   _os << "I hear a light rustle in the air.\n"
                      " Maybe there are bats near.\n";
                   continue;
                case PIT:
                    _os << "I feel a light cold drought in the air.\n"
                         " Maybe there is a pit somewhere.\n";
                    continue;
            }
            if (CAVE[_player_room_no][i] == _wumpus_room_no)
            {
                _os << "It smells terrible. Maybe the wumpus is near.\n";
            }
        }
        if (_arrows)
        {
            _os << ".\nYou have " << _arrows << " arrow" << (_arrows == 1 ? "" : "s")
               << " in your quiver.\n";
        }
            
        _os << "\n";
    }


    /// @brief Handles user input.
    ///
    void handle_input()
    {
        while (true)
        {
            std::string input_line; // Holds chunks of input
            std::getline(_is, input_line);
            std::size_t next_digit_pos {};
            _player_commando = Commando::NONE;

            _os << "Move or shoot (m-s)? ";

            if (input_line[0] == 'q')
            {
                _does_game_run = false;
                return;
            }
            if (input_line[0] == 'h' || (input_line[0] != 'm' && input_line[0] != 'm'))
            {
                print_help;
                continue;
            }

            // Handles the 'move' directive.
            if (input_line[0] == 'm')
            {
                // position of the next number iside input_line
                for (next_digit_pos = 1; !std::isdigit(input_line[next_digit_pos]) 
                && next_digit_pos < input_line.size(); ++next_digit_pos)
                    ;
            
                _room_to_move = std::stoi(input_line.substr(next_digit_pos));
                _player_commando = Commando::MOVE;
                return;
            }

            // Handles the 'shoot' directive.
            else if (input_line[0] == 's')
            {
                _player_commando = Commando::SHOOT;

                // position to the next number inside input_line
                for (next_digit_pos = 1; !isdigit(input_line[next_digit_pos])
                 && next_digit_pos < input_line.size(); ++next_digit_pos)
                    ;

                // clear array of arrow traversal rooms.
                for (int i = 0; i < ARROW_TRAVERSALS; ++i)
                {
                    _arrow_traversal_list[i] = -1;
                }

                // Setting array of arrow traversal rooms.
                try
                {
                    // Trying to extract numbers from input.
                    for (int i = 0; i < ARROW_TRAVERSALS; ++i)
                    {
                        std::size_t processed = 0;
                        _arrow_traversal_list[i] = std::stoi(input_line.substr(next_digit_pos), &processed);
                        next_digit_pos += processed;
                    }
                }
                catch (...)  // Error at reading in the numbers.
                {
                }
                break;
            }
            else
                continue; // Wrong command happend.
        }
    }


    /// @brief Trims the arrow traversal list.
    ///
    void trim_arrow_traversal_list()
    {

        // The main question is:
        // The User had provided a list of requested target-rooms whose the arrow shall conquer.
        // Now we must check whether all requested rooms are valid shoot targets.
        // This means, all the requested rooms, next to player's possition, must represent a linked list.
        //
        int arrow_room_no = _player_room_no; // At start, the arrow is in player's room.
        int width;                           // Depth of fled of the arrow.
        bool room_match_flag{false};         // Matches the room of shoot request with a valid room?

        for (width = 0; width < ARROW_TRAVERSALS; ++width)
        {
            for (int edge = 0; edge < EDGES; ++edge)
            {
                // Shoot target matches with room no.
                if (_arrow_traversal_list[width] == CAVE[arrow_room_no][edge])
                {
                    arrow_room_no = CAVE[arrow_room_no][edge];
                    room_match_flag = true;
                    break; // We found a match so we break out of search.
                }
            }
            // If a shoot target room didn't match edge room of previous room,
            // so we shrink the traversal_list by setting the edge room in the list
            // to INVALID_SHOOT.
            if (room_match_flag == false)
            {
                _arrow_traversal_list[width] = INVALID_SHOOT;
                break; // The search iis concluded due mismatch, so we are at end.
            }
        }
    }


    /// @brief Moves the player.
    ///
    void process_move()
    {
        for (int edge = 0; edge < EDGES; ++edge)
        {
            // We check whether _room_to_move is valid;
            if (_room_to_move == CAVE[_player_room_no][edge])
            {
                _player_room_no = CAVE[_player_room_no][edge];
                return;
            }
        }
        _os << "Whoops! You just tried to walk through solid rock ;-)\n";
        return;
    }


    /// @brief Handles arrow shooting.
    ///
    void process_shoot()
    { 
        if( _arrows <= 0)
        {
            _os << "You have no arrows in your quiver!\n";
            return;
        }
        --_arrows;
 
        for( int i; _arrow_traversal_list[i] != INVALID_SHOOT &&  i < ARROW_TRAVERSALS; ++i)
        {

            // player killed itself
            if (_arrow_traversal_list[i] == _player_room_no)
            {
                _player_room_no = IS_DEAD;
                _os << "Your arrow flew in your own room and killed you.\n";
                return;
            }

            // player killed the wumpus
            if (_arrow_traversal_list[i] == _wumpus_room_no)
            {
                _wumpus_room_no = IS_DEAD;
                _os << "You killed the wumpus.\n";
                return;
            }
        }

        // Moving the Wumpus inside the cave to an empty adjacent room.
        {
            int random_edge = std::rand() % EDGES;
            if (CAVE[_wumpus_room_no][random_edge] == EMPTY)
            {
                _wumpus_room_no = CAVE[_wumpus_room_no][random_edge];
                if (_wumpus_room_no == _player_room_no)
                {
                    _os << "The wumpus walks straight ahead into your room and eats you. *CRUNCH*\n";
                    _player_room_no = IS_DEAD;
                    return;
                }
            }
        }
    }


    /// @brief Processes the game.
    ///
    void process_game()
    {
        if (_player_commando == Commando::MOVE)
        {
            process_move();

            if( _player_room_no == _wumpus_room_no)
            {
                _os << "You walked straight ahead into wumpus' mouth. *CRUNCH*\n";
                _does_game_run = false;
            }
            else if (_rooms_content[_player_room_no] == PIT)
            {
                _os << "*AAAHhhhhh... ..you felt into a deep deep hole. For the luck you are\n"
                       "instantly dead when you shattered on the bottom.\n";
                _does_game_run = false;
            }
            else if( _rooms_content[_player_room_no] == BAT)
            {
                _os << "A huge bat grabbed you and fled with you through the cave.\n"
                     "Somewhere it lets you fall.\n";
                _rooms_content[_player_room_no] = EMPTY;
                do {
                    _player_room_no = std::rand() % CAVE_SIZE;
                } while (_rooms_content[_player_room_no] != EMPTY || _player_room_no ==_wumpus_room_no);
                int bat_room_no;
                do {
                    bat_room_no = std::rand() % CAVE_SIZE;
                } while (_rooms_content[bat_room_no] != EMPTY || bat_room_no == _player_room_no || bat_room_no == _wumpus_room_no);
                _rooms_content[bat_room_no] = BAT;
            }
        }
        else if (_player_commando == Commando::SHOOT)
        {
            process_shoot();

            if (_player_room_no == IS_DEAD || _wumpus_room_no == IS_DEAD)
            {
                _does_game_run = false;
            }
        }
    }


    int _arrows; // No of arrows inside in the player's quiver.
    bool _does_game_run;
    std::ostream &_os;
    std::istream &_is;
    int _player_room_no;  // Here resides the player.
    int _wumpus_room_no;  // We must know where the wumpus resides.
    int _arrow_traversal_list[ARROW_TRAVERSALS] {INVALID_SHOOT}; // For the room numbers, the arrow would traverse.
    int _room_to_move;
    Commando _player_commando;
    Content _rooms_content[CAVE_SIZE] {EMPTY};


public:
    Game()
        : _arrows{NO_OF_ARROWS}
        , _does_game_run{true}
        , _os{std::cout}
        , _is{std::cin}
        , _player_commando{Commando::NONE}
    {
        // populating the cave
        //

        // Set pits.
        for (int i = 0; i < NO_OF_PITS;)
        {
            if (const auto room_no{std::rand() % CAVE_SIZE}; _rooms_content[room_no] == EMPTY)
            {
                _rooms_content[room_no] = PIT;
                ++i;
            }
        }
        // Set bats.
        for (int i = 0; i < NO_OF_BATS;)
        {
            if (const auto room_no{std::rand() % CAVE_SIZE}; _rooms_content[room_no] == EMPTY)
            {
                _rooms_content[room_no] = BAT;
                ++i;
            }
        }

        // Set wumpus.
        while (true)
        {
            if (const auto room_no{std::rand() % CAVE_SIZE}; _rooms_content[room_no] == EMPTY)
            {
                _wumpus_room_no = room_no;
                break;
            }
        }

        // Set player.
        while (true)
        {
            if (const auto room_no{std::rand() % CAVE_SIZE}; _rooms_content[room_no] == EMPTY && room_no != _wumpus_room_no)
            {
                _player_room_no = room_no;
                break;
            }
        }
    }
    void run()
    {
        print_intro();
        while (_does_game_run)
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