#include "halfling.h"
#include <iostream>

// Constructor: sets base stats and starting items
Halfling::Halfling(const std::string& name) 
    : Character(name, 85, 15, std::map<std::string,int>{}) 
{
    baseAttackDamage = 10;
    nextAttackProtected = false;
    maxHealth = 85;

    // Initial items
    addItem("Halfling Pipe");
    addItem("Apple Pie");

    std::cout << "A Halfling named " << name << " was created! Life: " 
              << this->getHealth() << " Mana: " << this->getMana() << std::endl;
}

std::string Halfling::getClass() const{
    return "Halfling";
}

// Basic attack
ActionResult Halfling::attack(){
    int damage = getAttackDamage();

    ActionResult action = {
        .message = name + " attacks with courage! Causes " + std::to_string(damage) + " of damage.",
        .damage = damage
    };
                            
    return action;
}

// Spell: gives a temporary attack bonus if enough mana
ActionResult Halfling::castSpell() {
    ActionResult action;
    if(mana >= 10){
        mana -= 10;
        int bonus = rand() % 200 + 150;
        this->setTemporaryAttackBonus(bonus, 1);

        action.message = name + " uses Unexpected Luck! Next attack with a bonus of " 
                         + std::to_string(bonus) + "%! Mana left: " + std::to_string(mana);
    } 
    else{
        action.message = "Mana is not sufficient!";
        action.isError = true;
    }
    return action;
}

// Special move: causes damage and protects from next attack
ActionResult Halfling::specialMove() {
    this->nextAttackProtected = true;

    int damage = 15;
    ActionResult action = {
        .message = name + " uses Traveler's Trick! Causes " + std::to_string(damage) 
                   + " of damage and guarantees dodge in next turn!",
        .damage = damage
    };

    return action;
}

// Handle protection effect when attacked
void Halfling::handleAttackProtection(){
    std::cout << name << " dodges the attack!\n";
}
