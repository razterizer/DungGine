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
    const float px_aspect = 1.5f;
    float acc_sigma = 100.f;
    float acc_step = 10.f;
    float acc_lim = 25.f;
    float vel_lim = 12.f;
    int prob_change_acc = 7;
    bool wall_coll_resolve = false;
    int wall_coll_resolve_ctr = 0;
    
    Style style { Color::Green, Color::DarkYellow };
    char character = 'O';
    bool fog_of_war = true;
    bool light = false;
    bool is_underground = false;
    BSPNode* curr_room = nullptr;
    Corridor* curr_corridor = nullptr;
    bool last_in_room = false;
    bool last_in_corridor = false;
    
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
      if (wall_coll_resolve)
      {
        if (wall_coll_resolve_ctr++ < 2)
        {
          wall_coll_resolve_ctr = 0;
          wall_coll_resolve = false;
        }
      }
      else if (rnd::rand_int(0, prob_change_acc) == 0)
      {
        acc_r += rnd::randn_clamp(0.f, acc_sigma, -acc_step, +acc_step);
        acc_c += rnd::randn_clamp(0.f, acc_sigma*px_aspect, -acc_step*px_aspect, +acc_step*px_aspect);
        acc_r = math::clamp<float>(acc_r, -acc_lim, +acc_lim);
        acc_c = math::clamp<float>(acc_c, -acc_lim*px_aspect, +acc_lim*px_aspect);
      }
      vel_r += acc_r*dt;
      vel_c += acc_c*dt;
      vel_r = math::clamp<float>(vel_r, -vel_lim, +vel_lim);
      vel_c = math::clamp<float>(vel_c, -vel_lim*px_aspect, +vel_lim*px_aspect);
      pos_r += vel_r*dt;
      pos_c += vel_c*dt;
      auto r = math::roundI(pos_r);
      auto c = math::roundI(pos_c);
      auto location_corr = ttl::BBLocation::None;
      auto location_room = ttl::BBLocation::None;
      if (curr_corridor != nullptr)
        curr_corridor->is_inside_corridor({r, c}, &location_corr);//check_bb_location(r, c);
      if (curr_room != nullptr)
        curr_room->is_inside_room({r, c}, &location_corr);
      if (location_corr == ttl::BBLocation::Inside || location_room == ttl::BBLocation::Inside)
      {
        pos.r = r;
        pos.c = c;
        wall_coll_resolve_ctr = 0;
        wall_coll_resolve = false;
      }
      else if (!wall_coll_resolve && rnd::rand_int(0, 5) == 0)
      {
        auto location = ttl::BBLocation::None;
        if (location_room != ttl::BBLocation::None && location_corr != ttl::BBLocation::None)
        {
          //if (rnd::rand_int(0, 1) == 0)
          //  location = location_room;
          //else
          //  location = location_corr;
          location = ttl::BBLocation::None;
          acc_r = 0.f;
          vel_r = 5.f;
        }
        else if (location_room != ttl::BBLocation::None)
          location = location_room;
        else if (location_corr != ttl::BBLocation::None)
          location = location_corr;
        pos_r = pos.r;
        pos_c = pos.c;
        const float c_res_acc = 0.f;
        const float c_res_vel = 5.f;
        switch (location)
        {
          case ttl::BBLocation::OutsideTop:
            acc_r = c_res_acc;
            vel_r = c_res_vel;
            break;
          case ttl::BBLocation::OutsideTopLeft:
            acc_r = c_res_acc;
            acc_c = c_res_acc;
            vel_r = c_res_vel;
            vel_c = c_res_vel;
            break;
          case ttl::BBLocation::OutsideLeft:
            acc_c = c_res_acc;
            vel_c = c_res_vel;
            break;
          case ttl::BBLocation::OutsideBottomLeft:
            acc_r = -c_res_acc;
            acc_c = c_res_acc;
            vel_r = -c_res_vel;
            vel_c = c_res_vel;
            break;
          case ttl::BBLocation::OutsideBottom:
            acc_r = -c_res_acc;
            vel_r = -c_res_vel;
            break;
          case ttl::BBLocation::OutsideBottomRight:
            acc_r = -c_res_acc;
            acc_c = -c_res_acc;
            vel_r = -c_res_vel;
            vel_c = -c_res_vel;
            break;
          case ttl::BBLocation::OutsideRight:
            acc_c = -c_res_acc;
            vel_c = -c_res_vel;
            break;
          case ttl::BBLocation::OutsideTopRight:
            acc_r = c_res_acc;
            acc_c = -c_res_acc;
            vel_r = c_res_vel;
            vel_c = -c_res_vel;
            break;
          default:
            acc_r = 0.f;
            acc_c = 0.f;
            vel_r = 0.f;
            vel_c = 0.f;
            break;
        }
        
        wall_coll_resolve = true;
      }
      
      if (wall_coll_resolve)
        style.bg_color = Color::DarkBlue;
      else
        style.bg_color = Color::DarkYellow;
      
      // Update current room and current corridor.
      if (curr_corridor != nullptr)
      {
        auto* door_0 = curr_corridor->doors[0];
        auto* door_1 = curr_corridor->doors[1];
        if (door_0 != nullptr && pos == door_0->pos)
        {
          curr_room = door_0->room;
          //curr_corridor = nullptr;
        }
        else if (door_1 != nullptr && pos == door_1->pos)
        {
          curr_room = door_1->room;
          //curr_corridor = nullptr;
        }
        else
        {
          last_in_corridor = true;
          last_in_room = false;
        }
      }
      if (curr_room != nullptr)
      {
        bool found_door = false;
        for (auto* door : curr_room->doors)
          if (pos == door->pos)
          {
            curr_corridor = door->corridor;
            //curr_room = nullptr;
            found_door = true;
            break;
          }
        if (!found_door)
        {
          last_in_corridor = false;
          last_in_room = true;
        }
      }
    }
  };
  
}
