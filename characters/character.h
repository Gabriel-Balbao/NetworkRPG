#ifndef CHARACTER_H
#define CHARACTER_H

#include <iostream>
#include <sstream>
#include <string>
#include <map>

enum ActionType {
    ATTACK = 0,
    CAST_SPELL = 1,
    SPECIAL_MOVE = 2
};

// Result of an action (damage, healing, or error)
struct ActionResult {
    std::string message;
    int damage = 0;
    int heal = 0;
    bool isError = false;
};

// Base class for game characters
class Character {
    protected:
        int socketIndex;

        std::string name;
        int health;
        int mana;

        int maxHealth = 100;
        int maxMana = 100;
        
        bool nextAttackProtected;
        int baseAttackDamage;
        int temporaryAttackBonus[2] = {0, 0};

        std::map<std::string, int> inventory;
        
    public:
        Character(const std::string& name, int health, int mana, const std::map<std::string,int>& inventory);
        virtual ~Character() = default;

        virtual ActionResult attack() = 0;   
        virtual ActionResult castSpell() = 0;
        virtual ActionResult specialMove() = 0;

        // Socket
        void setSocketIndex(int idx) { socketIndex = idx; }
        int getSocketIndex() const { return socketIndex; }

        // Stats
        std::string getName() const;
        virtual std::string getClass() const = 0;

        int getHealth() const;
        int getMana() const;
        bool isAlive() const;
        void setDead();
        bool getNextAttackProtected() const;
        int getTemporaryAttackBonusDuration();
        int getTemporaryAttackBonusValue();

        void setNextAttackProtected(bool nextAttackProtected);
        void setTemporaryAttackBonus(int temporaryAttackBonusValue, int duration);

        // Attack
        int getAttackDamage();
        void takeDamage(int dmg);
        void gainHealth(int amt);
        virtual void handleAttackProtection() = 0;

        // Inventory
        void addItem(const std::string& item);      
        void removeItem(const std::string& item);   
        std::string lookInventory() const; 
        void useItem(const std::string& item);         
};

#endif
