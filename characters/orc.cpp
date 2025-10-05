#include "orc.h"
#include <iostream>

// Constructor: sets base stats and starting items
Orc::Orc(const std::string& name) 
    : Character(name, 90, 10, std::map<std::string,int>{}) 
{
    baseAttackDamage = 15;
    nextAttackProtected = false;
    maxHealth = 90;

    // Initial item
    addItem("Rope");

    std::cout << "An Orc named " << name << " was created! Life: " 
              << this->getHealth() << " Mana: " << this->getMana() << std::endl;
}

std::string Orc::getClass() const{
    return "Orc";
}

// Basic attack
ActionResult Orc::attack(){
    int damage = getAttackDamage();

    ActionResult action = {
        .message = name + " attacks aggressively! Causes " + std::to_string(damage) + " of damage.",
        .damage = damage
    };
              
    return action;
}

// Spell: costs mana, deals damage and heals
ActionResult Orc::castSpell() {
    ActionResult action;
    if(mana >= 5){
        mana -= 5;
        int damage = 10;
        int heal = 5;

        action.message = name + " uses Bloody Frenzy! Causes " + std::to_string(damage) 
                         + " of damage and heals " + std::to_string(heal) 
                         + "! Mana left: " + std::to_string(mana);
        action.damage = damage;
        action.heal = heal;
    } 
    else{
        action.message = "Mana is not sufficient!";
        action.isError = true;
    }
    return action;
}

// Special move: high damage and temporary attack bonus
ActionResult Orc::specialMove() {
    int damage = 25;
    int bonus = rand() % 75 + 25;
    this->setTemporaryAttackBonus(bonus, 1);

    ActionResult action = {
        .message = name + " uses Brutal Force! Causes " + std::to_string(damage) 
                   + " of damage and bonus of " + std::to_string(bonus) + "% in next attack!",
        .damage = damage
    };

    return action;
}

// Orc has no special attack protection
void Orc::handleAttackProtection(){}
