#ifndef HALFLING_H
#define HALFLING_H

#include "character.h"

// Halfling class: a small, nimble character with unique attack and protection abilities
class Halfling : public Character {
    public:
        Halfling(const std::string& name);  // Constructor: sets stats and starting items

        std::string getClass() const override;

        // Overridden actions
        ActionResult attack() override;       
        ActionResult castSpell() override;    
        ActionResult specialMove() override;  

        void handleAttackProtection() override; 
};

#endif
