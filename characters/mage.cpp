#include "mage.h"
#include <iostream>

// Constructor: sets base stats and starting items
Mage::Mage(const std::string& name) 
    : Character(name, 65, 100, std::map<std::string,int>{}) 
{
    baseAttackDamage = 10;
    nextAttackProtected = false;
    maxHealth = 65;

    // Initial item
    addItem("Magical Herbs");

    std::cout << "A Mage named " << name << " was created! Life: " 
              << this->getHealth() << " Mana: " << this->getMana() << std::endl;
}

std::string Mage::getClass() const{
    return "Mage";
}

// Basic attack with small chance to gain protection
ActionResult Mage::attack(){
    int damage = getAttackDamage();

    if((rand() % 100) < 10) this->nextAttackProtected = true;

    ActionResult action = {
        .message = name + " attacks with wisdom! Causes " + std::to_string(damage) + " of damage.",
        .damage = damage
    };
              
    return action;
}

// Spell: costs mana and deals fixed damage
ActionResult Mage::castSpell() {
    ActionResult action;
    if(mana >= 30){
        mana -= 30;
        int damage = 25;

        action.message = name + " uses Inherited Spell! Causes " + std::to_string(damage) 
                         + " of damage! Mana left: " + std::to_string(mana);
        action.damage = damage;
    } 
    else{
        action.message = "Mana is not sufficient!";
        action.isError = true;
    }
    return action;
}

// Special move: small damage and healing
ActionResult Mage::specialMove() {
    int damage = 5;
    int heal = 15;
    ActionResult action = {
        .message = name + " uses Divine Magic! Causes " + std::to_string(damage) 
                   + " of damage and heals " + std::to_string(heal) + "!",
        .damage = damage,
        .heal = heal
    };

    return action;
}

// Handle protection effect when attacked
void Mage::handleAttackProtection(){
    std::cout << name << " blocks the attack with a Aegis Veil!\n";
}
