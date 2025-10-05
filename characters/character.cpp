#include "character.h"

// Constructor
Character::Character(
    const std::string& name,
    int health,
    int mana,
    const std::map<std::string,int>& inventory) 
    : name(name), health(health), mana(mana), inventory(inventory)
{}

// Check if character is alive
bool Character::isAlive() const{
    return this->health > 0;
}

// Set health to zero
void Character::setDead(){
    this->health = 0;
}

// Getters
std::string Character::getName() const{
    return this->name;
}

int Character::getHealth() const{
    return this->health;
}

int Character::getMana() const{
    return this->mana;
}

bool Character::getNextAttackProtected() const{
    return this->nextAttackProtected;
}

int Character::getTemporaryAttackBonusValue(){
    return this->temporaryAttackBonus[0];
}

int Character::getTemporaryAttackBonusDuration(){
    return this->temporaryAttackBonus[1];
}

// Calculate attack damage including temporary bonus
int Character::getAttackDamage() {
    return baseAttackDamage + (baseAttackDamage * temporaryAttackBonus[0] / 100);
}

// Setters
void Character::setNextAttackProtected(bool nextAttackProtected){
    this->nextAttackProtected = nextAttackProtected;
}

void Character::setTemporaryAttackBonus(int temporaryAttackBonus, int duration){
    this->temporaryAttackBonus[0] = temporaryAttackBonus;
    this->temporaryAttackBonus[1] = duration;
}

// Modify health
void Character::takeDamage(int dmg){
    this->health -= dmg;
    if(health < 0) health = 0;
}

void Character::gainHealth(int amt){
    this->health += amt;
    if(health > maxHealth) health = maxHealth;
}

// Inventory management
void Character::addItem(const std::string& item){
    inventory[item]++; 
}

void Character::removeItem(const std::string& item){
    auto it = inventory.find(item);
    if(it != inventory.end()) {
        it->second--;
        if(it->second <= 0)
            inventory.erase(it);
    }
}

// Return inventory as string
std::string Character::lookInventory() const {
    std::stringstream ss;
    ss << "Inventory:\n";
    for (const auto& item : inventory) {
        ss << "- " << item.first << ": " << item.second << "\n";
    }
    return ss.str();
}

// Use an item
void Character::useItem(const std::string& item){
    auto it = inventory.find(item);

    if(it != inventory.end()){
        it->second--;
        if(it->second <= 0)
            inventory.erase(it);
        std::cout << name << " used " << item << "!\n";
    } 
    else {
        std::cout << name << " does not have " << item << " in inventory!\n";
    }
}
