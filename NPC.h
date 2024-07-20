//
//  NPC.h
//  DungGine
//
//  Created by Rasmus Anthin on 2024-07-19.
//

#pragma once
#include "Items.h"

namespace dung
{
  
  enum class Race { Human, Elf, Half_Elf, Gnome, Halfling, Dwarf, Half_Orc, Ogre, Hobgoblin, Goblin, Orc, Troll, Monster, Lich, Lich_King, Basilisk, Bear, Kobold, Skeleton, Giant, Huge_Spider, Wolf, Wywern, Griffin, Ghoul, Dragon, NUM_ITEMS };
  enum class Class { Warrior_Fighter, Warrior_Ranger, Warrior_Paladin, Warrior_Barbarian, Priest_Cleric, Priest_Druid, Priest_Monk, Priest_Shaman, Wizard_Mage, Wizard_Sorcerer, Rogue_Thief, Rogue_Bard, NUM_ITEMS };
  
  struct NPC final
  {
    RC pos;
    float pos_r = 0.f;
    float pos_c = 0.f;
    float vel_r = 0.f;
    float vel_c = 0.f;
    float acc_r = 0.f;
    float acc_c = 0.f;
    bool hit_wall = false;
    int hit_wall_ctr = 0;
    
    Style style { Color::Green, Color::DarkYellow };
    char character = 'O';
    bool fog_of_war = true;
    bool light = false;
    bool is_underground = false;
    BSPNode* curr_room = nullptr;
    Corridor* curr_corridor = nullptr;
    
    bool enemy = true;
    
    int health = 100;
    int strength = 10;
    int dexterity = 10;
    int constitution = 10;
    int thac0 = 1;
    int armor_class = 2;
    
    Race npc_race = Race::Ogre;
    Class npc_class = Class::Warrior_Barbarian;
    int weapon_idx = -1;
    
    void init()
    {
      pos_r = pos.r;
      pos_c = pos.c;
    }
    
    void update(float dt)
    {
      auto is_inside_curr_bb = [&](int r, int c) -> bool
      {
        if (curr_corridor != nullptr && curr_corridor->is_inside_corridor({r, c}))
          return true;
        if (curr_room != nullptr && curr_room->is_inside_room({r, c}))
          return true;
        return false;
      };
    
      if (hit_wall)
      {
        if (hit_wall_ctr++ < 20)
        {
          hit_wall_ctr = 0;
          hit_wall = false
        }
      }
      else
      {
        acc_r += rnd::randn_clamp(0.f, 5.f, -1.f, +1.f);
        acc_c += rnd::randn_clamp(0.f, 10.f, -2.f, +2.f);
        acc_r = math::clamp<float>(acc_r, -25.f, +25.f);
        acc_c = math::clamp<float>(acc_c, -50.f, +50.f);
      }
      vel_r += acc_r*dt;
      vel_c += acc_c*dt;
      vel_r = math::clamp<float>(vel_r, -12.f, +12.f);
      vel_c = math::clamp<float>(vel_c, -24.f, +24.f);
      pos_r += vel_r*dt;
      pos_c += vel_c*dt;
      auto r = std::round(pos_r);
      auto c = std::round(pos_c);
      if (is_inside_curr_bb(r, c))
      {
        pos.r = r;
        pos.c = c;
        hit_wall = false;
      }
      else
      {
        if (rnd::rand_int(0, 10) == 0)
        {
          acc_r = 0.f;
          vel_r = -4.f*math::sgn(vel_r);
          hit_wall = true;
        }
        else if (rnd::rand_int(0, 10) == 0)
        {
          acc_c = 0.f;
          vel_c = -4.f*math::sgn(vel_c);
          hit_wall = true;
        }
        //else
        //{
        //  acc_r *= 0.4f;
        //  acc_c *= 0.8f;
        //  vel_r *= 0.4f;
        //  vel_c *= 0.8f;
        //}
      }
      
      // Update current room and current corridor.
      if (curr_corridor != nullptr)
      {
        auto* door_0 = curr_corridor->doors[0];
        auto* door_1 = curr_corridor->doors[1];
        if (door_0 != nullptr && pos == door_0->pos)
          curr_room = door_0->room;
        else if (door_1 != nullptr && pos == door_1->pos)
          curr_room = door_1->room;
      }
      if (curr_room != nullptr)
      {
        for (auto* door : curr_room->doors)
          if (pos == door->pos)
          {
            curr_corridor = door->corridor;
            break;
          }
      }
    }
  };
  
}
