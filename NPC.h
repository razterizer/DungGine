//
//  NPC.h
//  DungGine
//
//  Created by Rasmus Anthin on 2024-07-19.
//

#pragma once
#include "Items.h"
#include "Globals.h"
#include "PlayerBase.h"
#include <Core/OneShot.h>


namespace dung
{
  
  enum class Race { Human, Elf, Half_Elf, Gnome, Halfling, Dwarf, Half_Orc, Ogre, Hobgoblin, Goblin, Orc, Troll, Monster, Lich, Lich_King, Basilisk, Bear, Kobold, Skeleton, Giant, Huge_Spider, Wolf, Wyvern, Griffin, Ghoul, Dragon, NUM_ITEMS };
  enum class Class { Warrior_Fighter, Warrior_Ranger, Warrior_Paladin, Warrior_Barbarian, Priest_Cleric, Priest_Druid, Priest_Monk, Priest_Shaman, Wizard_Mage, Wizard_Sorcerer, Rogue_Thief, Rogue_Bard, NUM_ITEMS };
  
  std::string race2str(Race race)
  {
    switch (race)
    {
      case Race::Human: return "human";
      case Race::Elf: return "elf";
      case Race::Half_Elf: return "half elf";
      case Race::Gnome: return "gnome";
      case Race::Halfling: return "halfling";
      case Race::Dwarf: return "dwarf";
      case Race::Half_Orc: return "half orc";
      case Race::Ogre: return "ogre";
      case Race::Hobgoblin: return "hobgoblin";
      case Race::Goblin: return "goblin";
      case Race::Orc: return "orc";
      case Race::Troll: return "troll";
      case Race::Monster: return "monster";
      case Race::Lich: return "lich";
      case Race::Lich_King: return "lich king";
      case Race::Basilisk: return "basilisk";
      case Race::Bear: return "bear";
      case Race::Kobold: return "kobold";
      case Race::Skeleton: return "skeleton";
      case Race::Giant: return "giant";
      case Race::Huge_Spider: return "huge spider";
      case Race::Wolf: return "wolf";
      case Race::Wyvern: return "wyvern";
      case Race::Griffin: return "griffin";
      case Race::Ghoul: return "ghoul";
      case Race::Dragon: return "dragon";
      case Race::NUM_ITEMS: return "n/a";
    }
  }
  
  enum class State { Patroll, Pursue, Fight, NUM_ITEMS };
  
  struct NPC final : PlayerBase
  {
    float pos_r = 0.f;
    float pos_c = 0.f;
    float vel_r = 0.f;
    float vel_c = 0.f;
    float acc_r = 0.f;
    float acc_c = 0.f;
    const float px_aspect = globals::px_aspect;
    float acc_step = 10.f;
    float acc_lim = 25.f;
    float vel_lim = 12.f;
    int prob_change_acc = 7;
    int prob_slow_fast = 20;
    const float acc_slowness_factor = 0.6f;
    const float vel_slowness_factor = 0.2f;
    float acc_factor = 1.f;
    float vel_factor = 1.f;
    bool slow = false;
    State state = State::Patroll;
    
    bool debug = false;
    
    bool wall_coll_resolve = false;
    int wall_coll_resolve_ctr = 0;
    
    bool fog_of_war = true;
    bool light = false;
    bool visible = false;
    bool visible_near = false;
    bool is_underground = false;
    bool inside_room = false;
    bool inside_corr = false;
    
    bool enemy = true;
    
    int armor_class = 2;
    
    Race npc_race = Race::Ogre;
    Class npc_class = Class::Warrior_Barbarian;
    int weapon_idx = -1;
    
    const float c_dist_fight = 2.f + 1e-2f;
    const float c_dist_pursue = 7.f + 1e-2;
    const float c_dist_patroll = 12.f + 1e-2;
    const float c_dist_hostile_hyst_on = 2.f + 1e-2f;
    const float c_dist_hostile_hyst_off = 3.f + 1e-2f;
    const int c_fight_min_dist = 1;
    bool is_hostile = false;
    bool was_hostile = false;
    OneShot trg_info_hostile_npc;
    
    float death_time_s = 0.f;
    OneShot trg_death;
    
  private:
    
    void move(const RC& pc_pos, Environment* environment, float dt)
    {
      if (wall_coll_resolve)
      {
        if (wall_coll_resolve_ctr++ < 2)
        {
          wall_coll_resolve_ctr = 0;
          wall_coll_resolve = false;
        }
      }
      else if (rnd::one_in(prob_change_acc))
      {
        acc_r += rnd::randn_range(-acc_step, +acc_step);
        acc_c += rnd::randn_range(-acc_step*px_aspect, +acc_step*px_aspect);
        acc_r = math::clamp<float>(acc_r, -acc_lim*acc_factor, +acc_lim*acc_factor);
        acc_c = math::clamp<float>(acc_c, -acc_lim*acc_factor*px_aspect, +acc_lim*acc_factor*px_aspect);
      }
      switch (state)
      {
        case State::Patroll:
          vel_r += acc_r*dt;
          vel_c += acc_c*dt;
          break;
        case State::Pursue:
        case State::Fight:
          //vel_r = 0.5f * (pc_pos.r - pos.r);
          //vel_c = 0.5f * (pc_pos.c - pos.c);
          
          if (pc_pos.r + c_fight_min_dist < pos.r)
            vel_r = 0.5f * ((pc_pos.r + c_fight_min_dist) - pos.r);
          else if (pc_pos.r - c_fight_min_dist > pos.r)
            vel_r = 0.5f * ((pc_pos.r - c_fight_min_dist) - pos.r);
          
          if (pc_pos.c + c_fight_min_dist < pos.c)
            vel_c = 0.5f * ((pc_pos.c + c_fight_min_dist) - pos.c);
          else if (pc_pos.c - c_fight_min_dist > pos.c)
            vel_c = 0.5f * ((pc_pos.c - c_fight_min_dist) - pos.c);
          break;
        case State::NUM_ITEMS:
          break;
      }
      vel_r = math::clamp<float>(vel_r, -vel_lim, +vel_lim);
      vel_c = math::clamp<float>(vel_c, -vel_lim*vel_factor*px_aspect, +vel_lim*vel_factor*px_aspect);
      pos_r += vel_r*dt;
      pos_c += vel_c*dt;
      auto r = math::roundI(pos_r);
      auto c = math::roundI(pos_c);
      auto location_corr = ttl::BBLocation::None;
      auto location_room = ttl::BBLocation::None;
      inside_room = false;
      inside_corr = false;
      if (curr_corridor != nullptr)
        inside_corr = curr_corridor->is_inside_corridor({r, c}, &location_corr);//check_bb_location(r, c);
      if (curr_room != nullptr)
        inside_room = curr_room->is_inside_room({r, c}, &location_corr);
      if (inside_room || inside_corr)
      {
        bool ok_move_to = environment->allow_move_to(r, c);
        bool wet = is_wet(environment->get_terrain(r, c));
        bool allow_walking = ok_move_to && !wet;
        bool allow_swimming = ok_move_to && can_swim && wet;
        bool allow_flying = can_fly;
        if (allow_walking || allow_swimming || allow_flying)
        {
          pos.r = r;
          pos.c = c;
        }
        else
        {
          pos_r = pos.r;
          pos_c = pos.c;
        }
        wall_coll_resolve_ctr = 0;
        wall_coll_resolve = false;
      }
      else if (!wall_coll_resolve && rnd::one_in(6))
      {
        auto location = ttl::BBLocation::None;
        if (location_room != ttl::BBLocation::None && location_corr != ttl::BBLocation::None)
        {
          acc_r = 0.f;
          acc_c = 0.f;
          vel_r = 0.f;
          vel_c = 0.f;
        }
        else if (inside_room && location_room != ttl::BBLocation::None)
          location = location_room;
        else if (inside_corr && location_corr != ttl::BBLocation::None)
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
    }
    
  public:
    NPC()
    {
      character = 'O';
      style = { Color::Green, Color::DarkYellow };
    }
  
    void init(const std::vector<std::unique_ptr<Weapon>>& all_weapons)
    {
      pos_r = pos.r;
      pos_c = pos.c;
      
      npc_race = rnd::rand_enum<Race>();
      npc_class = rnd::rand_enum<Class>();
      
      int ctr = 0;
      const int num_weapons = static_cast<int>(all_weapons.size());
      if (!all_weapons.empty() && !rnd::one_in(4))
      {
        do
        {
          int idx = rnd::rand_idx(num_weapons);
          if (!all_weapons[idx]->picked_up)
          {
            weapon_idx = idx;
            all_weapons[idx]->picked_up = true;
            break;
          }
        } while (ctr++ < 10);
      }
      
      const float c_min_acc_step = 0.3f;
      const float c_min_acc_lim = 0.6f;
      const float c_min_vel_lim = 0.2f;
      
      auto rand_acc_step = [c_min_acc_step](float lo, float hi)
      {
        return std::max(c_min_acc_step, rnd::randn_range(lo, hi))/10.f;
      };
      
      auto rand_acc_lim = [c_min_acc_lim](float lo, float hi)
      {
        return std::max(c_min_acc_lim, rnd::randn_range(lo, hi));
      };
      
      auto rand_vel_lim = [c_min_vel_lim](float lo, float hi)
      {
        return std::max(c_min_vel_lim, rnd::randn_range(lo, hi));
      };
      
      switch (npc_race)
      {
        case Race::Human:
          character = '@';
          style = { Color::Magenta, Color::LightGray };
          acc_step = rand_acc_step(2.f, 20.f);
          acc_lim = rand_acc_lim(20.f, 50.f);
          vel_lim = rand_vel_lim(4.f, 15.f);
          prob_change_acc = rnd::randn_range_int(4, 10);
          prob_slow_fast = rnd::randn_range_int(10, 30);
          enemy = rnd::one_in(5);
          can_swim = true;
          can_fly = false;
          break;
        case Race::Elf:
          character = '@';
          style = { Color::Magenta, Color::DarkGreen };
          acc_step = rand_acc_step(4.f, 40.f);
          acc_lim = rand_acc_lim(25.f, 70.f);
          vel_lim = rand_vel_lim(6.f, 20.f);
          prob_change_acc = rnd::randn_range_int(4, 10);
          prob_slow_fast = rnd::randn_range_int(10, 30);
          enemy = rnd::one_in(20);
          can_swim = true;
          can_fly = false;
          break;
        case Race::Half_Elf:
          character = '@';
          style = { Color::Magenta, Color::DarkYellow };
          acc_step = rand_acc_step(3.f, 30.f);
          acc_lim = rand_acc_lim(25.f, 60.f);
          vel_lim = rand_vel_lim(5.f, 17.f);
          prob_change_acc = rnd::randn_range_int(4, 10);
          prob_slow_fast = rnd::randn_range_int(10, 30);
          enemy = rnd::one_in(15);
          can_swim = true;
          can_fly = false;
          break;
        case Race::Gnome:
          character = 'b';
          style = { Color::Magenta, Color::LightGray };
          acc_step = rand_acc_step(1.f, 10.f);
          acc_lim = rand_acc_lim(10.f, 20.f);
          vel_lim = rand_vel_lim(0.5f, 2.5f);
          prob_change_acc = rnd::randn_range_int(1, 4);
          prob_slow_fast = rnd::randn_range_int(10, 30);
          enemy = rnd::one_in(20);
          can_swim = true;
          can_fly = false;
          break;
        case Race::Halfling:
          character = 'b';
          style = { Color::Magenta, Color::LightGray };
          acc_step = rand_acc_step(1.f, 15.f);
          acc_lim = rand_acc_lim(11.f, 25.f);
          vel_lim = rand_vel_lim(0.7f, 3.f);
          prob_change_acc = rnd::randn_range_int(1, 5);
          prob_slow_fast = rnd::randn_range_int(10, 30);
          enemy = rnd::one_in(20);
          can_swim = true;
          can_fly = false;
          break;
        case Race::Dwarf:
          character = '0';
          style = { Color::White, Color::DarkGray };
          acc_step = rand_acc_step(1.5f, 18.f);
          acc_lim = rand_acc_lim(12.f, 30.f);
          vel_lim = rand_vel_lim(0.4f, 4.f);
          prob_change_acc = rnd::randn_range_int(5, 20);
          prob_slow_fast = rnd::randn_range_int(10, 30);
          enemy = rnd::one_in(18);
          can_swim = rnd::rand_bool();
          can_fly = false;
          break;
        case Race::Half_Orc:
          character = '3';
          style = { Color::Yellow, Color::Green };
          acc_step = rand_acc_step(1.5f, 20.f);
          acc_lim = rand_acc_lim(30.f, 80.f);
          vel_lim = rand_vel_lim(1.5f, 5.f);
          prob_change_acc = rnd::randn_range_int(2, 18);
          prob_slow_fast = rnd::randn_range_int(10, 30);
          enemy = !rnd::one_in(10);
          can_swim = rnd::rand_bool();
          can_fly = false;
          break;
        case Race::Ogre:
          character = 'O';
          style = { Color::Green, Color::DarkYellow };
          acc_step = rand_acc_step(4.f, 10.f);
          acc_lim = rand_acc_lim(2.f, 8.f);
          vel_lim = rand_vel_lim(1.f, 6.f);
          prob_change_acc = rnd::randn_range_int(4, 10);
          prob_slow_fast = rnd::randn_range_int(10, 30);
          enemy = !rnd::one_in(5);
          can_swim = true;
          can_fly = false;
          break;
        case Race::Hobgoblin:
          character = 'a';
          style = { Color::Yellow, Color::Cyan };
          acc_step = rand_acc_step(5.f, 15.f);
          acc_lim = rand_acc_lim(10.f, 50.f);
          vel_lim = rand_vel_lim(4.f, 9.f);
          prob_change_acc = rnd::randn_range_int(4, 14);
          prob_slow_fast = rnd::randn_range_int(10, 30);
          enemy = !rnd::one_in(15);
          can_swim = false;
          can_fly = false;
          break;
        case Race::Goblin:
          character = 'G';
          style = { Color::Green, Color::DarkCyan };
          acc_step = rand_acc_step(5.f, 15.f);
          acc_lim = rand_acc_lim(8.f, 45.f);
          vel_lim = rand_vel_lim(4.5f, 10.f);
          prob_change_acc = rnd::randn_range_int(3, 12);
          prob_slow_fast = rnd::randn_range_int(10, 30);
          enemy = !rnd::one_in(15);
          can_swim = rnd::rand_bool();
          can_fly = false;
          break;
        case Race::Orc:
          character = '2';
          style = { Color::DarkYellow, Color::Cyan };
          acc_step = rand_acc_step(5.f, 25.f);
          acc_lim = rand_acc_lim(50.f, 80.f);
          vel_lim = rand_vel_lim(6.f, 18.f);
          prob_change_acc = rnd::randn_range_int(4, 8);
          prob_slow_fast = rnd::randn_range_int(10, 30);
          enemy = !rnd::one_in(15);
          can_swim = rnd::rand_bool();
          can_fly = false;
          break;
        case Race::Troll:
          character = 'R';
          style = { Color::LightGray, Color::DarkRed };
          acc_step = rand_acc_step(1.f, 14.f);
          acc_lim = rand_acc_lim(5.f, 15.f);
          vel_lim = rand_vel_lim(2.f, 12.f);
          prob_change_acc = rnd::randn_range_int(10, 40);
          prob_slow_fast = rnd::randn_range_int(10, 30);
          enemy = !rnd::one_in(14);
          can_swim = false;
          can_fly = false;
          break;
        case Race::Monster:
          character = 'M';
          style = { Color::Cyan, Color::DarkGreen };
          acc_step = rand_acc_step(0.5f, 25.f);
          acc_lim = rand_acc_lim(2.f, 25.f);
          vel_lim = rand_vel_lim(1.f, 8.f);
          prob_change_acc = rnd::randn_range_int(8, 25);
          prob_slow_fast = rnd::randn_range_int(10, 30);
          enemy = !rnd::one_in(15);
          can_swim = rnd::rand_bool();
          can_fly = false;
          break;
        case Race::Lich:
          character = 'z';
          style = { Color::DarkYellow, Color::DarkBlue };
          acc_step = rand_acc_step(4.f, 30.f);
          acc_lim = rand_acc_lim(25.f, 55.f);
          vel_lim = rand_vel_lim(2.f, 9.f);
          prob_change_acc = rnd::randn_range_int(5, 8);
          prob_slow_fast = rnd::randn_range_int(10, 30);
          enemy = !rnd::one_in(15);
          can_swim = false;
          can_fly = false;
          break;
        case Race::Lich_King:
          character = 'Z';
          style = { Color::Yellow, Color::DarkBlue };
          acc_step = rand_acc_step(5.f, 35.f);
          acc_lim = rand_acc_lim(25.f, 60.f);
          vel_lim = rand_vel_lim(2.5f, 10.f);
          prob_change_acc = rnd::randn_range_int(4, 6);
          prob_slow_fast = rnd::randn_range_int(10, 30);
          enemy = !rnd::one_in(15);
          can_swim = false;
          can_fly = false;
          break;
        case Race::Basilisk:
          character = 'S';
          style = { Color::Green, Color::DarkGray };
          acc_step = rand_acc_step(5.f, 18.f);
          acc_lim = rand_acc_lim(2.f, 25.f);
          vel_lim = rand_vel_lim(4.f, 8.f);
          prob_change_acc = rnd::randn_range_int(16, 28);
          prob_slow_fast = rnd::randn_range_int(10, 30);
          enemy = !rnd::one_in(15);
          can_swim = true;
          can_fly = false;
          break;
        case Race::Bear:
          character = 'B';
          style = { Color::Red, Color::DarkRed };
          acc_step = rand_acc_step(10.f, 25.f);
          acc_lim = rand_acc_lim(3.f, 10.f);
          vel_lim = rand_vel_lim(3.f, 18.f);
          prob_change_acc = rnd::randn_range_int(5, 8);
          prob_slow_fast = rnd::randn_range_int(10, 30);
          enemy = !rnd::one_in(10);
          can_swim = true;
          can_fly = false;
          break;
        case Race::Kobold:
          character = 'x';
          style = { Color::Blue, Color::LightGray };
          acc_step = rand_acc_step(5.f, 15.f);
          acc_lim = rand_acc_lim(25.f, 40.f);
          vel_lim = rand_vel_lim(2.f, 10.f);
          prob_change_acc = rnd::randn_range_int(3, 9);
          prob_slow_fast = rnd::randn_range_int(10, 30);
          enemy = !rnd::one_in(15);
          can_swim = false;
          can_fly = false;
          break;
        case Race::Skeleton:
          character = '%';
          style = { Color::White, Color::DarkGray };
          acc_step = rand_acc_step(5.f, 15.f);
          acc_lim = rand_acc_lim(10.f, 60.f);
          vel_lim = rand_vel_lim(1.f, 4.f);
          prob_change_acc = rnd::randn_range_int(11, 19);
          prob_slow_fast = rnd::randn_range_int(10, 30);
          enemy = !rnd::one_in(10);
          can_swim = false;
          can_fly = false;
          break;
        case Race::Giant:
          character = 'O';
          style = { Color::DarkMagenta, Color::LightGray };
          acc_step = rand_acc_step(5.f, 15.f);
          acc_lim = rand_acc_lim(1.f, 5.f);
          vel_lim = rand_vel_lim(0.5f, 4.5f);
          prob_change_acc = rnd::randn_range_int(20, 40);
          prob_slow_fast = rnd::randn_range_int(10, 30);
          enemy = !rnd::one_in(5);
          can_swim = rnd::rand_bool();
          can_fly = false;
          break;
        case Race::Huge_Spider:
          character = 'W';
          style = { Color::DarkGray, Color::White };
          acc_step = rand_acc_step(5.f, 15.f);
          acc_lim = rand_acc_lim(10.f, 70.f);
          vel_lim = rand_vel_lim(3.f, 20.f);
          prob_change_acc = rnd::randn_range_int(3, 17);
          prob_slow_fast = rnd::randn_range_int(10, 30);
          enemy = !rnd::one_in(13);
          can_swim = false;
          can_fly = false;
          break;
        case Race::Wolf:
          character = 'm';
          style = { Color::LightGray, Color::DarkGray };
          acc_step = rand_acc_step(15.f, 35.f);
          acc_lim = rand_acc_lim(15.f, 60.f);
          vel_lim = rand_vel_lim(10.f, 24.f);
          prob_change_acc = rnd::randn_range_int(2, 9);
          prob_slow_fast = rnd::randn_range_int(10, 30);
          enemy = !rnd::one_in(8);
          can_swim = rnd::rand_bool();
          can_fly = false;
          break;
        case Race::Wyvern:
          character = 'w';
          style = { Color::DarkMagenta, Color::Blue };
          acc_step = rand_acc_step(5.f, 15.f);
          acc_lim = rand_acc_lim(2.f, 15.f);
          vel_lim = rand_vel_lim(8.f, 20.f);
          prob_change_acc = rnd::randn_range_int(7, 15);
          prob_slow_fast = rnd::randn_range_int(10, 30);
          enemy = !rnd::one_in(12);
          can_swim = false;
          can_fly = true;
          break;
        case Race::Griffin:
          character = 'g';
          style = { Color::DarkRed, Color::Blue };
          acc_step = rand_acc_step(5.f, 15.f);
          acc_lim = rand_acc_lim(10.f, 25.f);
          vel_lim = rand_vel_lim(9.f, 21.f);
          prob_change_acc = rnd::randn_range_int(10, 20);
          prob_slow_fast = rnd::randn_range_int(10, 30);
          enemy = !rnd::one_in(13);
          can_swim = false;
          can_fly = true;
          break;
        case Race::Ghoul:
          character = 'h';
          style = { Color::LightGray, Color::Yellow };
          acc_step = rand_acc_step(5.f, 15.f);
          acc_lim = rand_acc_lim(30.f, 60.f);
          vel_lim = rand_vel_lim(10.f, 20.f);
          prob_change_acc = rnd::randn_range_int(1, 5);
          prob_slow_fast = rnd::randn_range_int(10, 30);
          enemy = !rnd::one_in(20);
          can_swim = false;
          can_fly = false;
          break;
        case Race::Dragon:
          character = 'R';
          style = { Color::Red, Color::DarkMagenta };
          acc_step = rand_acc_step(5.f, 45.f);
          acc_lim = rand_acc_lim(7.f, 30.f);
          vel_lim = rand_vel_lim(11.f, 29.f);
          prob_change_acc = rnd::randn_range_int(14, 30);
          prob_slow_fast = rnd::randn_range_int(10, 30);
          enemy = !rnd::one_in(7);
          can_swim = false;
          can_fly = true;
          break;
        case Race::NUM_ITEMS:
          std::cerr << "Illegal race: NUM_ITEMS!" << std::endl;
          break;
        default:
          std::cerr << "Unknown race (" + std::to_string(static_cast<int>(npc_race)) + ")!";
          break;
      }
    }
    
    void set_visibility(bool use_fog_of_war, bool fow_near, bool is_night)
    {
      visible = !((use_fog_of_war && this->fog_of_war) ||
                  ((this->is_underground || is_night) && !this->light));
      visible_near = !((use_fog_of_war && (this->fog_of_war || !fow_near)) ||
                       ((this->is_underground || is_night) && !this->light));
    }
    
    void update(const RC& pc_pos, Environment* environment, float time, float dt)
    {
      if (health <= 0)
      {
        if (trg_death.once())
          death_time_s = time;
        character = '&';
        style = { Color::Red, Color::DarkGray };
        return;
      }
      
      update_los();
      update_terrain();
      
      if (rnd::one_in(prob_slow_fast))
      {
        math::toggle(slow);
        if (slow)
        {
          acc_factor = acc_slowness_factor;
          vel_factor = vel_slowness_factor;
        }
        else
        {
          acc_factor = 1.f;
          vel_factor = 1.f;
        }
      }
      
      auto dist_to_pc = distance(pos, pc_pos);
      
      was_hostile = is_hostile;
      if (enemy)
      {
        if (dist_to_pc < c_dist_hostile_hyst_on)
          is_hostile = true;
        else if (dist_to_pc > c_dist_hostile_hyst_off)
          is_hostile = false;
      }
      
      if (enemy && dist_to_pc < c_dist_fight)
        state = State::Fight;
      else if (enemy && dist_to_pc < c_dist_pursue)
        state = State::Pursue;
      else if (dist_to_pc > c_dist_patroll)
        state = State::Patroll;
      
      if (allow_move())
        move(pc_pos, environment, dt);
      
      if (inside_room && curr_room != nullptr)
      {
        fog_of_war = curr_room->is_in_fog_of_war(pos);
        light = curr_room->is_in_light(pos);
        is_underground = environment->is_underground(curr_room);
      }
      else if (inside_corr && curr_corridor != nullptr)
      {
        fog_of_war = curr_corridor->is_in_fog_of_war(pos);
        light = curr_corridor->is_in_light(pos);
        is_underground = environment->is_underground(curr_corridor);
      }

      if (debug)
      {
        if (wall_coll_resolve)
          style.fg_color = Color::Black;
        else
          style.fg_color = Color::White;
          
        switch (state)
        {
          case State::Patroll:
            style.bg_color = Color::DarkGreen;
            break;
          case State::Pursue:
            style.bg_color = Color::DarkYellow;
            break;
          case State::Fight:
            style.bg_color = Color::DarkRed;
            break;
          case State::NUM_ITEMS:
            break;
        }
      }
      
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
      }
      if (curr_room != nullptr)
      {
        for (auto* door : curr_room->doors)
          if (pos == door->pos)
          {
            curr_corridor = door->corridor;
            //curr_room = nullptr;
            break;
          }
      }
    }
    
    int calc_armour_class() const
    {
      return armor_class + (dexterity / 2);
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

  };
  
}
