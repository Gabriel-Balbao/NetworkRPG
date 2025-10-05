#ifndef ORC_H
#define ORC_H

#include "character.h"

// Orc class: strong and aggressive character with healing and high-damage abilities
class Orc : public Character{
    public:
        Orc(const std::string& name);  // Constructor: sets stats and starting items

        std::string getClass() const override;

        ActionResult attack() override;
        ActionResult castSpell() override; 
        ActionResult specialMove() override;

        void handleAttackProtection() override;
};

#endif
