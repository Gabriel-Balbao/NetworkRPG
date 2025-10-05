#ifndef MAGE_H
#define MAGE_H

#include "character.h"

// Mage class: a spellcaster with high mana and healing abilities
class Mage : public Character {
    public:
        Mage(const std::string& name);  // Constructor: sets stats and starting items

        std::string getClass() const override;

        // Overridden actions
        ActionResult attack() override;       
        ActionResult castSpell() override;    
        ActionResult specialMove() override;  

        void handleAttackProtection() override; 
};

#endif
