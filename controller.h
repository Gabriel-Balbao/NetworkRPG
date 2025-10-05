#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <vector>
#include "characters/character.h"

class Controller {
    private:
        std::vector<Character *> players; // List of player characters
        int currentTurn;                  // Index of the current player's turn

    public:
        Controller(const std::vector<Character*>& chars); 

        void nextTurn();                                  

        Character* getCurrentPlayer();                   

        ActionResult applyAction(Character *attacker, int action, Character *target); 

        bool isBattleOver();                              
};

#endif
