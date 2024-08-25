//
//  PC.h
//  DungGine
//
//  Created by Rasmus Anthin on 2024-06-21.
//

#pragma once
#include "Globals.h"
#include "Terrain.h"
#include "Items.h"
#include "PlayerBase.h"
#include "Inventory.h"
#include <Core/StlUtils.h>


namespace dung
{
  
  struct PC final : PlayerBase
  {
    bool is_spawned = false;
    
    int base_ac = 10;
        
    std::vector<int> key_idcs;
    std::vector<int> lamp_idcs;
    std::vector<int> weapon_idcs;
    std::vector<int> potion_idcs;
    std::vector<int> armour_idcs;
    bool show_inventory = false;
    float weight_capacity = 50.f;
    
    PC()
    {
      character = '@';
      style = { Color::Magenta, Color::White };
    }
    
    void update()
    {
      update_los();
      update_terrain();
    }
    
    int calc_armour_class(Inventory* inventory) const
    {
      int tot_protection = 0;
      auto* armour_group = inventory->fetch_group("Armour:");
      for (auto& armour_subgroup : *armour_group)
      {
        auto* selected_inv_item = armour_subgroup.get_selected_item();
        if (selected_inv_item != nullptr && selected_inv_item->item != nullptr)
        {
          auto* armour = dynamic_cast<Armour*>(selected_inv_item->item);
          if (armour != nullptr)
            tot_protection += armour->protection;
        }
      }
      return base_ac + tot_protection + (dexterity / 2); // Example: Include dexterity bonus
    }
    
    // Function to calculate melee attack bonus
    int get_melee_attack_bonus() const
    {
      return strength / 2; // Example: Strength bonus to attack rolls
    }

    // Function to calculate melee damage bonus
    int get_melee_damage_bonus() const
    {
      return strength / 2; // Example: Strength bonus to damage
    }
    
    bool using_key_id(Inventory* inventory, int key_id) const
    {
      auto* group = inventory->fetch_group("Keys:");
      auto* subgroup = group->fetch_subgroup(0);
      auto* selected_inv_item = subgroup->get_selected_item();
      if (selected_inv_item != nullptr && selected_inv_item->item != nullptr)
      {
        auto* key = dynamic_cast<Key*>(selected_inv_item->item);
        if (key != nullptr)
          return key->key_id == key_id;
      }
      return false;
    }
    
    void remove_key_by_key_id(Inventory* inventory, std::vector<Key>& all_keys, int key_id)
    {
      auto it = stlutils::find_if(all_keys, [key_id](const auto& key) { return key.key_id == key_id; });
      if (it != all_keys.end())
        inventory->remove_item(&(*it));
      stlutils::erase_if(key_idcs, [&](int key_idx) { return all_keys[key_idx].key_id == key_id; });
      stlutils::erase_if(all_keys, [&](const auto& key) { return key.key_id == key_id; });
    }
    
    void remove_selected_potion(Inventory* inventory, std::vector<Potion>& all_potions)
    {
      auto* group = inventory->fetch_group("Potions:");
      auto* subgroup = group->fetch_subgroup(0);
      auto* selected_inv_item = subgroup->get_selected_item();
      if (selected_inv_item != nullptr && selected_inv_item->item != nullptr)
      {
        auto* potion = dynamic_cast<Potion*>(selected_inv_item->item);
        if (potion != nullptr)
        {
          subgroup->remove_item(potion);
          auto idx = stlutils::find_if_idx(all_potions, [potion](const auto& p) { return &p == potion; });
          stlutils::erase_at(potion_idcs, idx);
          stlutils::erase_if(all_potions, [potion](const auto& p) { return &p == potion; });
        }
      }
    }
    
    Key* get_selected_key(Inventory* inventory) const
    {
      auto* group = inventory->fetch_group("Keys:");
      auto* subgroup = group->fetch_subgroup(0);
      auto* selected_inv_item = subgroup->get_selected_item();
      if (selected_inv_item != nullptr && selected_inv_item->item != nullptr)
      {
        auto* key = dynamic_cast<Key*>(selected_inv_item->item);
        if (key != nullptr)
          return key;
      }
      return nullptr;
    }
    
    Lamp* get_selected_lamp(Inventory* inventory) const
    {
      auto* group = inventory->fetch_group("Lamps:");
      auto* subgroup = group->fetch_subgroup(0);
      auto* selected_inv_item = subgroup->get_selected_item();
      if (selected_inv_item != nullptr && selected_inv_item->item != nullptr)
      {
        auto* lamp = dynamic_cast<Lamp*>(selected_inv_item->item);
        if (lamp != nullptr)
          return lamp;
      }
      return nullptr;
    }
    
    Weapon* get_selected_melee_weapon(Inventory* inventory)
    {
     auto* group = inventory->fetch_group("Weapons:");
      auto* subgroup = group->fetch_subgroup(0);
      auto* selected_inv_item = subgroup->get_selected_item();
      if (selected_inv_item != nullptr && selected_inv_item->item != nullptr)
      {
        auto* weapon = dynamic_cast<Weapon*>(selected_inv_item->item);
        if (weapon != nullptr)
          return weapon;
      }
      return nullptr;
    }
    
    Potion* get_selected_potion(Inventory* inventory)
    {
      auto* group = inventory->fetch_group("Potions:");
      auto* subgroup = group->fetch_subgroup(0);
      auto* selected_inv_item = subgroup->get_selected_item();
      if (selected_inv_item != nullptr && selected_inv_item->item != nullptr)
      {
        auto* potion = dynamic_cast<Potion*>(selected_inv_item->item);
        if (potion != nullptr)
          return potion;
      }
      return nullptr;
    }
    
    Armour* get_selected_armour(Inventory* inventory, ArmourType type)
    {
      auto* group = inventory->fetch_group("Armour:");
      auto* subgroup = group->fetch_subgroup(type);
      auto* selected_inv_item = subgroup->get_selected_item();
      if (selected_inv_item != nullptr && selected_inv_item->item != nullptr)
      {
        auto* armour = dynamic_cast<Armour*>(selected_inv_item->item);
        if (armour != nullptr)
          return armour;
      }
      return nullptr;
    }
    
    bool is_inside_curr_room() const
    {
      if (curr_room != nullptr)
        return curr_room->bb_leaf_room.is_inside_offs(pos, -1);
      return false;
    }
    
    bool is_inside_curr_corridor() const
    {
      if (curr_corridor != nullptr)
        return curr_corridor->is_inside_corridor(pos);
      return false;
    }
  };
  
}
