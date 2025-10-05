#include <iostream>

#include "controller.h"

// Constructor: initializes controller with player list and sets first turn
Controller::Controller(const std::vector<Character *> &chars) : players(chars), currentTurn(0) {}

// Advances to the next player's turn
void Controller::nextTurn() {
    currentTurn = (currentTurn + 1) % players.size();
}

// Returns the current player
Character* Controller::getCurrentPlayer() {
    return players[currentTurn];
}

// Applies the chosen action from attacker to target and returns result
ActionResult Controller::applyAction(Character *attacker, int action, Character *target){
    ActionResult result;

    switch(action){
        case ATTACK: 
            result = attacker->attack(); break;
        case CAST_SPELL: 
            result = attacker->castSpell(); break;
        case SPECIAL_MOVE: 
            result = attacker->specialMove(); break;
        default:
            result.isError = true;
            result.message = "Invalid action type.";
            return result;
    }

    if(!result.isError){
        if(target->getNextAttackProtected()) {
            target->handleAttackProtection();
            target->setNextAttackProtected(false);
        } 
        else{
            target->takeDamage(result.damage);
        }
        
        int temporaryAttack[2] = {attacker->getTemporaryAttackBonusValue(), attacker->getTemporaryAttackBonusDuration()};
        if(temporaryAttack[0] > 0){
            attacker->setTemporaryAttackBonus(temporaryAttack[0], temporaryAttack[1]--);
        }
        else{
            attacker->setTemporaryAttackBonus(0, 0);
        }

        if(result.heal > 0) attacker->gainHealth(result.heal);
    }

    return result;
}

// Returns true if only one or no players are alive
bool Controller::isBattleOver() {
    int aliveCount = 0;
    for(auto c : players) if (c->isAlive()) aliveCount++;
    return aliveCount <= 1;
}

