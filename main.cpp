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

constexpr int CAVE_SIZE = 20;    // Count of rooms in the cave.
constexpr int NO_OF_PITS = 2;    // Number of pits.
constexpr int NO_OF_BATS = 2;    // Number of bats.
constexpr int NO_OF_ARROWS = 5;  // Number of arrows.
constexpr int EDGES = 3;         // Number of edges (doors) of each node (room).


// This is the graphical correlation of its edges of each room of the cave.
// The numbers of a row represent the connected rooms of a room.
// There are 'CAVE_SIZE' rooms, each of them has 'EDGES' nighbour rooms.
// All in all must the length of the array be of 'CAVE_SIZE'.
// The graph below represents a dodecahedron.
constexpr int CONNECTIONS[CAVE_SIZE][EDGES] =
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
enum Content { EMPTY, WUMPUS, BAT, PIT };

std::ostream & operator<<( std::ostream & os, const Content c ) {
    // Thank you @seeplus for the idea using a lookup array.
    constexpr const char * const lookup[] = { "EMPTY", "WUMPUS", "BAT", "PIT" };
    return os << lookup[c];
}


// A cave is structured as a graph, with rooms as nodes and doors at edges.
struct Cave
{
    struct Room
    {
        Room * pr[EDGES];  // Connections to other rooms.
        int room_no;
        Content content;
    };

    Room * rooms;
    Room * wumpus_room;  // Here resides the Wumpus.

    Cave() : rooms{new Room[CAVE_SIZE]} // thanks @seeplus
    {
        // Connecting the rooms.
        for( int r {}; r < CAVE_SIZE; ++ r ) {
            rooms[r].room_no = r + 1;
            for( int e = 0; e < EDGES; ++ e) {
                rooms[r].pr[e] = & rooms[ CONNECTIONS[r][e] ];
            }
        }

        // Populating the cave.
        while( true ) {
            // Thank you @seeplus for the hint that if-statements at C++17 could define its
            // own variable in its header.
            if( const auto i {std::rand() % CAVE_SIZE}; rooms[i].content == EMPTY ) {
               rooms[i].content = WUMPUS;
               wumpus_room = & rooms[i];
               break;
           }
        }
        for( int count = 0; count < NO_OF_PITS; ++ count ) {
            while( true ) {
                if( const auto i {std::rand() % CAVE_SIZE}; rooms[i].content == EMPTY ) {
                    rooms[i].content = PIT;
                    break;
                }
            }
        }
        for( int count = 0; count < NO_OF_BATS; ++ count ) {
            while( true ) {
                if( const auto i {std::rand() % CAVE_SIZE}; rooms[i].content == EMPTY ) {
                    rooms[i].content = BAT;
                    break;
                }
            }
        }
    }

    ~Cave() {  delete[] rooms; }
};

std::ostream & operator<<( std::ostream & os, const Cave & cave ){

    os << "room_no\tpr[0]\tpr[1]\tpr[2]\tcontent\n"
          "_______________________________________\n";
    for( int i = 0; i < CAVE_SIZE; ++ i ) {
        os << cave.rooms[i].room_no;
        for( int k = 0; k < EDGES; ++ k ) {
           os << '\t' << cave.rooms[i].pr[k]->room_no;
        }
        os << '\t' << cave.rooms[i].content << '\n';
    }
    return os << '\n';
}

class Game
{

private:

    Cave cave;
    Cave::Room * room;          // Here resides the player.
    Cave::Room * wumpus_room;   // Here resides the Wumpus.
    int arrows;
    bool does_game_run;
    std::ostream & os;
    std::istream & is;
    int a[5] = {0};  // Room numbers of which could an arrow fly.

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
    void print_status() {
        os << "\n\nYou are now in room " << room->room_no << ".\n"
           << "There are tunnels, leading to rooms ";
        for( int i = 0; i < EDGES; ++ i) {
            os << (EDGES>1&&i==EDGES-1?"and ":"")
               << room->pr[i]->room_no << (i<EDGES-2?",":"") << (i==EDGES-1?"":" ");
        }
        os << ".\nYou have " << arrows << " arrow" << (arrows==1?"":"s")
           <<" in your quiver.\n";

       for( int i = 0; i < EDGES; ++ i )
           if( room->pr[i]->content == WUMPUS )
               os << "It smells somewhat. Maybe a wumpus is near.\n";
       for( int i = 0; i < EDGES; ++ i )
           if( room->pr[i]->content == BAT )
               os << "I hear a light rustle in the air.\n"
                  << " Maybe there are bats near.\n";
       for( int i = 0; i < EDGES; ++ i )
           if( room->pr[i]->content == PIT )
               os << "I feel a light cold drought in the air.\n"
                  << " Maybe there is a pit somewhere.\n";
        os << "\n";

    }

    // Handling arrow shooting.
    // TODO: The code so far is hard to understand. Maybe using of 'a[]' is
    // a bad design idea. I'm dissatified with that.
    void process_shoot() {
        int len = 0;
        for( int i = 0; i < 5; ++ i ) {
            if( a[i] <= 0 || a[i] >= CAVE_SIZE)
                break;
            ++ len;
        }

        Cave::Room * tmp = room;
        bool msg_flag = true;
        bool wumpus_kill = false;
        bool self_kill = false;

        for( int i = 0; i < len; ++i) {
            if( a[i] == 0 ) return;
            bool rand_flag = true;

            for( int e = 0; e < EDGES; ++ e )
                if( a[i] == tmp->pr[e]->room_no )
                    rand_flag = false;

            if( rand_flag ) {
                if( msg_flag ) {
                    os << "You coose the wrong path for your arrow.\n"
                       << "Your arrow fly instead through room ";
                    msg_flag = false;
                }
                tmp = tmp->pr[ std::rand() % EDGES ];
            }
            else {
                if( msg_flag ) {
                    os << "Your arrow fly through room ";
                    msg_flag = false;
                }
                for( int k = 0; k < EDGES; ++ k ) {
                    if( a[i] == tmp->pr[k]->room_no ) {
                        tmp = tmp->pr[k];
                        break;
                    }
                }
            }
            os << tmp->room_no << ' ';
            if( tmp->content == WUMPUS ) wumpus_kill = true;
            if( tmp->room_no == room->room_no ) self_kill = true;
        }
        os << ".\n";
        -- arrows;
        if( wumpus_kill ) {
            os << "Your shoot killed the Wumpus.\n";
            does_game_run = false;
        } else if( self_kill ) {
            os << "Your arrow flew in your own room and killed you.\n";
            does_game_run = false;
        } else {
           // Moving the Wumpus inside the cave to an adjacent room.
           for( int i = 0; i < EDGES; ++i ) {
               int e = rand() % EDGES;

               if( wumpus_room->pr[e]->content == EMPTY ) {
                  wumpus_room->content = EMPTY;
                  wumpus_room = wumpus_room->pr[e];
                  wumpus_room->content = WUMPUS;
                  break;
               }
           }
           if( wumpus_room == room ) {
               os << "The Wumpus enters the room where you stay and eats you.\n"
                     "*crunch*\n";
               does_game_run = false;
           }
       }
    }

    // TODO: I'm dissatisfied with using 'a[]' as moving and shooting temporary storage.
    void handle_input() {
       while( true ) {
           os << "Move or shoot (m-s)? ";
           std::string input;
           std::getline( is, input );
           if( input[0] == 'q' ) {
               does_game_run = false;
               return;
           }
           if( ! is )
               continue;
           std::istringstream iss( input );
           char ch;
           iss >> ch;

           if( ch != 's' && ch != 'm' ) {
               print_help();
               print_status();
               continue;
           }

           // Handling the 'move' directive.
           if ( ch == 'm' ) {
               iss >> a[0];
               if( ! iss ) continue;

               for( int i = 0; i < EDGES; ++ i ) {
                   if( a[0] == room->pr[i]->room_no ) {
                       room = room->pr[i];
                       a[0] = 0;
                       return;
                   }
               }
               os << "Whoops! You just tried to walk through solid rock :-)\n";
               continue;
           }

            // Handling the 'shoot' directive.
           else if( ch == 's' ) {
               if( arrows <= 0) {
                   os << "You have no arrows in your quiver!\n";
                   continue;
               }
               std::size_t pos = 0;
               for( ; ! isdigit( input[pos] ); ++pos );
               try {
                   // Trying to extract numbers from input.
                   for( int i = 0; i < 5; ++ i ) {
                       std::size_t processed = 0;
                       a[i] = std::stoi( input.substr( pos), &processed );
                       pos += processed;
                   }
                   return;
               }
               catch(...) {}
               return;
           }
           else
               continue; // Wrong command happend.
        }
    }

    void process_game()
    {
        if( does_game_run == false )
            return;
        // Handling entering a room with the Wumpus inside.
        if( room->content == WUMPUS ) {
            os << "You walked straight ahead into the mouth of the Wumpus! *crunch*\n";
            does_game_run = false;
            return;
        }
        // Handling a pit.
        else if( room->content == PIT ) {
            os << "*AAAHhhhhh... ..you felt into a deep deep hole. For the luck you are\n"
               << "instantly dead when you shattered on the bottom.\n";
            does_game_run = false;
            return;
        }
        // Handling entering a room where is a bat:
        else if( room->content == BAT ) {
            room->content = EMPTY;
            while( true ) {
                if( auto const idx {rand() % CAVE_SIZE}; cave.rooms[idx].content == EMPTY ) {
                    room = & cave.rooms[idx];
                    os << "A huge bat takes you away and flys with you through the cave.\n";
                    while( true ) {
                        int idx = rand() % CAVE_SIZE;
                        if( const auto idx{rand() % CAVE_SIZE}; cave.rooms[idx].content == EMPTY ) {
                            cave.rooms[idx].content = BAT;
                            return;
                        }
                    }
                }
            }
        }

        // If there was a plain move:
        else if( a[0] == 0 )
            return;

        // Handling arrow shooting.
        process_shoot();

        // Clearing the arrow array.
        for( int i = 0; i < 5; ++ i ) a[i] = 0;
    }

public:

    Game()
    : arrows{NO_OF_ARROWS}, does_game_run{true}, os{ std::cout }, is{ std::cin }
    , wumpus_room{ cave.wumpus_room }
    {
        // Get randomly a empty room for the player.
        while( true ) {
            if( const auto i{rand() % CAVE_SIZE}; cave.rooms[i].content == EMPTY ) {
                room = & cave.rooms[i];
                break;
            }
        }
    }
    void run() {
       intro();
       while( does_game_run ) {
           print_status();
           // std::cerr << cave;
           handle_input();
           process_game();
       }
       bye();
    }
};

int main()
{
    Game().run();
}
