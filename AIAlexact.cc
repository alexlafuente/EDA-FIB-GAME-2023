#include "Player.hh"
#include <list>

/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME lognHater
#define MIN_HOR 0
#define MAX_HOR 79
#define MIN_VER 0
#define MAX_VER 39

// Definitions for types of elements in maps
#define ROCK 1
#define ELEVATOR 2
#define HELLHOUND 3
#define FURYAN 4
#define PIONEER 5
#define NECROMONGER 6
#define GEM 7
#define SUN 8

struct PLAYER_NAME : public Player {

    /**
    * Factory: returns a new instance of this class.
    * Do not modify this function.
    */
    static Player* factory () {
        return new PLAYER_NAME;
    }

    /**
    * Types and attributes for your player can be defined here.
    */
    
    const vector<Dir> directions = {Bottom, BR, Right, RT, Top, TL, Left, LB};
    const vector<char> cellRepresentation = {'.', 'R', 'E', 'H', 'F', 'P', 'N', 'G', 'S'}; // Characters needed for the visual representation of maps
    
    /* Pre: mapCave and mapSurface are the same size as each floors map, and contain the the required information of the map for this round. id is the identifier
       of a unit. MIN_VER <= Pos.i <= MAX_VER, MIN_HOR <= Pos.j <= MAX_HOR, 0 <= Pos.k <= 1. */
    // Post: retunrs wether the position p is safe for the pioneer or it isn't.
    bool pioneer_safe_and_not_obstructed(Pos p, vector<vector<int>> &mapCave, vector<vector<int>> &mapSurface){
        bool able = true;
        if(p.k == 0){
            if(mapCave[p.i][p.j] == ROCK or mapCave[p.i][p.j] == HELLHOUND or mapCave[p.i][p.j] == FURYAN or cell(p).id != -1){
                able = false;
            }
        }
        else{
            if(mapSurface[p.i][p.j] == NECROMONGER or mapSurface[p.i][p.j] == FURYAN or mapSurface[p.i][p.j] == SUN or cell(p).id != -1){
                able = false;
            }
        }
        return able;
    }
    
    /* Pre: mapCave and mapSurface are the same size as each floors map, and contain the the required information of the map for this round. id is the identifier
       of a unit. MIN_VER <= Pos.i <= MAX_VER, MIN_HOR <= Pos.j <= MAX_HOR, 0 <= Pos.k <= 1. */
    // Post: retunrs wether the position p is safe for the furyan or it isn't.
    bool furyan_safe_and_not_obstructed(Pos p, vector<vector<int>> &mapCave, vector<vector<int>> &mapSurface){
        bool able = true;
        if(p.k == 0){
            if(mapCave[p.i][p.j] == ROCK or mapCave[p.i][p.j] == HELLHOUND or mapCave[p.i][p.j] == FURYAN){
                able = false;
            }
        }
        else{
            if(mapSurface[p.i][p.j] == SUN or mapSurface[p.i][p.j] == NECROMONGER or mapSurface[p.i][p.j] == FURYAN){
                able = false;
            }
        }
        return able;
    }
    
    // Pre: MIN_VER <= Pos.i <= MAX_VER, MIN_HOR <= Pos.j <= MAX_HOR, 0 <= Pos.k <= 1.
    // Post: returns wether p is the targeted position or it isn't.
    bool found_objective_pioneer(Pos p, bool useElevator, bool findGem){
        bool found = false;
        // Check both floors
        if(useElevator and cell(p).type == Elevator) found = true;
        // Check underground
        else if(p.k == 0 and not useElevator and cell(p).type != Elevator and cell(p).owner != me()) found = true;
        // Check surface
        else if(p.k == 1 and not useElevator and cell(p).gem and findGem) found = true;
        return found;
    }
    
    // Pre: MIN_VER <= Pos.i <= MAX_VER, MIN_HOR <= Pos.j <= MAX_HOR, 0 <= Pos.k <= 1.
    // Post: returns wether p is the targeted position or it isn't.
    bool found_objective_furyan(Pos p, bool useElevator, bool findGem){
        bool found = false;
        // Check both floors
        if(useElevator and cell(p).type == Elevator) found = true;
        // Check underground
        else if(p.k == 0 and not useElevator and cell(p).id != -1 and unit(cell(p).id).type == Pioneer and unit(cell(p).id).player != me()) found = true;
        // Check surface
        else if(p.k == 1 and not useElevator and cell(p).gem and findGem) found = true;
        return found;
    }
    
    /* Pre: mapCave and mapSurface are the same size as each floors map, and contain the the required information of the map for this round. id is the identifier
       of a unit. MIN_VER <= Pos.i <= MAX_VER, MIN_HOR <= Pos.j <= MAX_HOR, 0 <= Pos.k <= 1. kind is PIONEER or FURYAN.*/
    // Post: returns the next movement for the unit
    Pos path_finder(int id, vector<vector<int>> &mapCave, vector<vector<int>> &mapSurface, int kind, Pos ini, bool useElevator, bool findGem){
        Pos dest = ini;
        bool found = false;
        vector<vector<bool>> visited(MAX_VER+1, vector<bool>(MAX_HOR+1, false)); // Visited cells
        vector<vector<Pos>> parent; // Parent cells for path reconstruction
        if(ini.k == 0) parent = vector<vector<Pos>> (MAX_VER+1, vector<Pos>(MAX_HOR+1, Pos(-1, -1, 0)));
        else parent = vector<vector<Pos>> (MAX_VER+1, vector<Pos>(MAX_HOR+1, Pos(-1, -1, 1)));
        queue<Pos> q;
        q.push(ini);
        visited[ini.i][ini.j] = true;
        Pos reconstruct;
        while(not q.empty() and not found){
            Pos p = q.front();
            q.pop();
            for(int i = 0; i < int(directions.size()) and not found; ++i){
                Pos newP = p + directions[i];
                if(pos_ok(newP) and kind == PIONEER and pioneer_safe_and_not_obstructed(newP,mapCave,mapSurface) and not visited[newP.i][newP.j]){
                    q.push(newP);
                    visited[newP.i][newP.j] = true;
                    parent[newP.i][newP.j] = p; // Store the parent for path reconstruction
                    if(found_objective_pioneer(newP,useElevator,findGem)){
                        found = true;
                        reconstruct = newP;
                    }
                }
                else if(pos_ok(newP) and kind == FURYAN and furyan_safe_and_not_obstructed(newP,mapCave,mapSurface) and not visited[newP.i][newP.j]){
                    q.push(newP);
                    visited[newP.i][newP.j] = true;
                    parent[newP.i][newP.j] = p; // Store the parent for path reconstruction
                    if(found_objective_furyan(newP,useElevator,findGem)){
                        found = true;
                        reconstruct = newP;
                    }
                }
            }
        }
        // Reconstruct the path
        if(found){
            Pos current = reconstruct; // path to follow by the individual
            list<Pos> path;
            while(not (current.i == ini.i and current.j == ini.j)){
                path.push_front(current);
                current = parent[current.i][current.j];
            }
            dest = path.front();
        }
        return dest;
    }
    
    /* Pre: mapCave and mapSurface are the same size as each floors map, and contain the the required information of the map for this round. id is the identifier
       of a unit */
    // Post: the unit with identifier id is given this round's order
    void commander(int id, vector<vector<int>> &mapCave, vector<vector<int>> &mapSurface, int kind){
        Unit u = unit(id);
        Pos next;
        // Change both of the following lines when wanting to explore the surface
        if(kind == PIONEER) next = path_finder(id, mapCave, mapSurface, kind, u.pos, false, false);
        else next = path_finder(id, mapCave, mapSurface, kind, u.pos, false, false);
        Dir movement = None;
        if(next != u.pos){
            bool found = false;
            for(int i = 0; i < int(directions.size()) and not found; ++i){
                if(u.pos + directions[i] == next){
                    found = true;
                    movement = directions[i];
                }
            }
        }
        command(id, movement);
    }
    
    void move_pioneers(vector<vector<int>> &mapCave, vector<vector<int>> &mapSurface){
        vector<int> P = pioneers(me());
        for(int id : P){
            commander(id, mapCave, mapSurface, PIONEER);
        }
    }
    
    void move_furyans(vector<vector<int>> &mapCave, vector<vector<int>> &mapSurface){
        vector<int> F = furyans(me());
        for(int id : F){
            commander(id, mapCave, mapSurface, FURYAN);
        }
    }
    
    // Pre: MIN_VER <= i <= MAX_VER, MIN_HOR <= j <= MAX_HOR
    // Post: map is updated. Every cell in the 2 cell radius of position i, j (i for rows, j for columnes) is marked with type
    void map_danger(vector<vector<int>> &map, int i, int j, int type){
        int ini_i, end_i, ini_j, end_j;
        
        if(type == HELLHOUND){
            if(i - 3 < MIN_VER) ini_i = MIN_VER;
            else ini_i = i - 3;
            
            if(i + 3 > MAX_VER) end_i = MAX_VER;
            else end_i = i + 3;
            ini_j = j - 3;
            end_j = j + 3;
        }
        else{
            if(i - 1 < MIN_VER) ini_i = MIN_VER;
            else ini_i = i - 1;
            
            if(i + 1 > MAX_VER) end_i = MAX_VER;
            else end_i = i + 1;
            ini_j = j - 1;
            end_j = j + 1;
        }
        
        for(int act_i = ini_i; act_i < end_i+1; ++act_i){
            for(int act_j = ini_j; act_j < end_j+1; ++act_j){
                if(act_j > MAX_HOR) map[act_i][act_j - MAX_HOR+1] = type;
                else if(act_j < MIN_HOR) map[act_i][act_j + MAX_HOR+1] = type;
                else map[act_i][act_j] = type;
            }
        }
    }
    
    // Pre: mapCave and mapSurface have the same size: row size = MAX_VER, column size = MAX_HOR.
    // Post: mapCave and mapSurface are updated to contain 
    void mapping(vector<vector<int>> &mapCave, vector<vector<int>> &mapSurface){
        for(int i = MIN_VER; i < MAX_VER+1; ++i){
            for(int j = MIN_HOR; j < MAX_HOR+1; ++j){
                Pos p0(i,j,0); // underground;
                Pos p1(i,j,1); // Surface
                Cell c0 = cell(p0);
                Cell c1 = cell(p1);
                
                // underground mapping
                if(c0.type == Rock) mapCave[i][j] = ROCK;
                else if(c0.id != -1 and unit(c0.id).type == Hellhound) map_danger(mapCave,i,j,HELLHOUND); // Hellhounds mapping
                
                // Surface mapping
                if(c1.id != -1 and unit(c1.id).type == Necromonger) map_danger(mapSurface,i,j,NECROMONGER);
                else if(c1.gem) mapSurface[i][j] = GEM;
                else if((daylight(p1) or daylight(Pos(i,j-1,1)) or daylight(Pos(i,j-2,1)))) mapSurface[i][j] = SUN; /* Marks two extra columnes to prevent moving
                into the sun during the next round (sun advances two columnes each round) */
                
                // Both floors mapping
                if(c0.id != -1 and unit(c0.id).player != me() and unit(c0.id).type == Furyan) map_danger(mapCave,i,j,FURYAN); // Furyans Mapping
                if(c1.id != -1 and unit(c1.id).player != me() and unit(c1.id).type == Furyan) map_danger(mapSurface,i,j,FURYAN); // Furyans Mapping
                
                if(c0.type == Elevator) mapCave[i][j] = ELEVATOR;
                if(c1.type == Elevator) mapSurface[i][j] = ELEVATOR;
            }
        }
    }
    
    void show_map(const vector<vector<int>> &map) {
        for(int i = MIN_VER; i < MAX_VER+1; ++i){
            for(int j = MIN_HOR; j < MAX_HOR+1; ++j){
                cout << cellRepresentation[map[i][j]];
            }
            cout << endl;
        }
        cout << endl << endl;
    }
    
  /**
   * Play method, invoked once per each round.
   */
    virtual void play () {
        vector<vector<int>> mapCave(MAX_VER+1, vector<int>(MAX_HOR+1));
        vector<vector<int>> mapSurface(MAX_VER+1, vector<int>(MAX_HOR+1));
        mapping(mapCave,mapSurface);
        move_furyans(mapCave,mapSurface);
        move_pioneers(mapCave,mapSurface);
    }

};


/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);
