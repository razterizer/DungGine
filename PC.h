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
#include "ScreenHelper.h"
#include <Core/StlUtils.h>
#include <Termin8or/ParticleSystem.h>

//#define DEBUG_FIRE_SMOKE


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
    float weight_capacity_soft = 50.f;
    float weight_capacity_hard = 70.f;
    float curr_tot_inv_weight = 0.f;
    
    ParticleHandler fire_smoke_engine { 500 };
    
    ParticleGradientGroup smoke_0
    {
      {
        {
          { 0.00f, Color::White },
          { 0.27f, Color::Red },
          { 0.60f, Color::Yellow },
          { 0.75f, Color::LightGray },
          { 0.88f, Color::DarkGray },
        }
      },
      {
        {
          { 0.12f, Color::Blue },
          { 0.20f, Color::White },
          { 0.30f, Color::Yellow },   // 0.125f
          { 0.55f, Color::DarkRed },  // 0.375f
          { 0.70f, Color::DarkGray }, // 0.625f
          { 0.85f, Color::Black },    // 0.875f
        }
      },
      {
        {
          { 0.0000f, "o" },
          { 0.25f, "v" },
          { 0.45f, "s" },
          { 0.65f, "%" },
          { 0.6667f, "&" },
          { 0.8333f, "@" }
        }
      }
    };
    
    ParticleGradientGroup smoke_1
    {
      {
        {
          { 0.10f, Color::White },
          { 0.37f, Color::Red },
          { 0.70f, Color::Yellow },
          { 0.85f, Color::LightGray },
          { 0.98f, Color::DarkGray },
        }
      },
      {
        {
          { 0.12f, Color::Blue },
          { 0.30f, Color::White },
          { 0.50f, Color::Yellow },   // 0.125f
          { 0.74f, Color::DarkRed },  // 0.375f
          { 0.86f, Color::DarkGray }, // 0.625f
          { 1.00f, Color::Black },    // 0.875f
        }
      },
      {
        {
          { 0.0000f, "." },
          { 0.1667f, "*" },
          { 0.3333f, "s" },
          { 0.5000f, "%" },
          { 0.6667f, "&" },
          { 0.8333f, "@" }
        }
      }
    };
    

    std::vector<std::pair<float, ParticleGradientGroup>> smoke_color_gradients;
    
    // ////////////////////////////////
    
  private:
  
    void update_fire_smoke(ScreenHelper* screen_helper, Inventory* inventory, float sim_time, float sim_dt)
    {
      const float vel_r = -10*los_r;
      const float vel_c = -10*los_c;
      const float acc = 0.f, life_time = 0.2f;
      float spread = 23.f;
      const int cluster_size = 10;
      auto* curr_lamp = get_selected_lamp(inventory);
      bool trg = false;
      if (curr_lamp != nullptr)
      {
        curr_lamp->update(sim_dt);
        if (curr_lamp->t_life_time < 1.f)
          trg = curr_lamp->lamp_type == Lamp::LampType::Torch;
        spread = curr_lamp->radius*2.f;
      }
      fire_smoke_engine.update(screen_helper->get_screen_pos(pos), trg, vel_r, vel_c, acc, spread, life_time, cluster_size, sim_dt, sim_time);
    }
    
  public:
    
    PC()
    {
      character = '@';
      style = { Color::Magenta, Color::White };
      smoke_color_gradients.emplace_back(0.5f, smoke_0);
      smoke_color_gradients.emplace_back(0.6f, smoke_1);
    }
    
    void update(ScreenHelper* screen_helper, Inventory* inventory, float sim_time, float sim_dt)
    {
      update_los();
      update_terrain();
      update_fire_smoke(screen_helper, inventory, sim_time, sim_dt);
      
      weight_strain = math::value_to_param_clamped(curr_tot_inv_weight, weight_capacity_soft, weight_capacity_hard);
    }
    
    template<int NR, int NC>
    void draw(SpriteHandler<NR, NC>& sh, float sim_time)
    {
      fire_smoke_engine.draw(sh, smoke_color_gradients, sim_time);
#ifdef DEBUG_FIRE_SMOKE
      int c_offs = 0;
      for (const auto& grad : smoke_color_gradients)
      {
        const auto& g = grad.second;
        float t_prev = 0.f;
        for (int i = 0; i < NR; ++i)
        {
          float t = static_cast<float>(i)/(NR - 1);
          sh.write_buffer(g.string_gradient(t), i, 30 + c_offs, g.fg_color_gradient(t), g.bg_color_gradient(t));
          if (std::floor(t*4) - std::floor(t_prev*4) == 1)
            sh.write_buffer("#", i, 29, Color::White);
          t_prev = t;
        }
        c_offs++;
      }
#endif
    }
    
    bool has_weight_capacity(float item_weight) const
    {
      return curr_tot_inv_weight + item_weight <= weight_capacity_hard;
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
          auto idx = stlutils::find_if_idx(all_potions, [potion](const auto& p) { return &p == potion; });
          subgroup->remove_item(potion);
          stlutils::erase(potion_idcs, idx);
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
