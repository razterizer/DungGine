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
  using BBLocation = t8::BBLocation;
  
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
      case Race::NUM_ITEMS: return "error";
      default: return "n/a";
    }
  }
  
  enum class State { Patroll, Pursue, FightMelee, FightRanged, NUM_ITEMS };
  
  struct NPC final : PlayerBase
  {
    Style orig_style;
  
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
    bool was_debug = false;
    
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
    int melee_weapon_idx = -1;
    int ranged_weapon_idx = -1;
    int armour_idx = -1;
    int fierceness = 0; // Body as weapon.
    bool animal = false; // Animals cannot have weapons nor armour.
    
    static constexpr float c_dist_fight_melee = 2.f + 1e-2f;
    static constexpr float c_dist_fight_ranged = 10.f + 1e-2f;
    static constexpr float c_dist_pursue = 8.f + 1e-2f;
    static constexpr float c_dist_patroll = 12.f + 1e-2f;
    static constexpr float c_dist_hostile_hyst_on = 2.f + 1e-2f;
    static constexpr float c_dist_hostile_hyst_off = 3.f + 1e-2f;
    static constexpr int c_fight_min_dist_melee = 1;
    static constexpr int c_fight_min_dist_ranged = 3;
    bool is_hostile = false;
    bool was_hostile = false;
    OneShot trg_info_hostile_npc;
    float dist_to_pc = math::get_max<float>();
    bool can_see_pc = false;
    
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
        case State::FightMelee:
        {
          const int c_fight_min_dist = state == State::FightRanged ? c_fight_min_dist_ranged : c_fight_min_dist_melee;
        
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
        }
        case State::FightRanged:
        case State::NUM_ITEMS:
          break;
      }
      vel_r = math::clamp<float>(vel_r, -vel_lim, +vel_lim);
      vel_c = math::clamp<float>(vel_c, -vel_lim*vel_factor*px_aspect, +vel_lim*vel_factor*px_aspect);
      pos_r += vel_r*dt;
      pos_c += vel_c*dt;
      auto r = math::roundI(pos_r);
      auto c = math::roundI(pos_c);
      auto location_corr = BBLocation::None;
      auto location_room = BBLocation::None;
      inside_room = false;
      inside_corr = false;
      if (curr_corridor != nullptr)
        inside_corr = curr_corridor->is_inside_corridor({r, c}, &location_corr);//check_bb_location(r, c);
      if (curr_room != nullptr)
        inside_room = curr_room->is_inside_room({r, c}, &location_corr);
      if (inside_room || inside_corr)
      {
        bool ok_move_to = environment->allow_move_to(curr_floor, r, c);
        bool wet = is_wet(environment->get_terrain(curr_floor, r, c));
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
          pos_r = static_cast<float>(pos.r);
          pos_c = static_cast<float>(pos.c);
        }
        wall_coll_resolve_ctr = 0;
        wall_coll_resolve = false;
      }
      else if (!wall_coll_resolve && rnd::one_in(6))
      {
        auto location = BBLocation::None;
        if (location_room != BBLocation::None && location_corr != BBLocation::None)
        {
          acc_r = 0.f;
          acc_c = 0.f;
          vel_r = 0.f;
          vel_c = 0.f;
        }
        else if (inside_room && location_room != BBLocation::None)
          location = location_room;
        else if (inside_corr && location_corr != BBLocation::None)
          location = location_corr;
        pos_r = static_cast<float>(pos.r);
        pos_c = static_cast<float>(pos.c);
        const float c_res_acc = 0.f;
        const float c_res_vel = 5.f;
        switch (location)
        {
          case BBLocation::OutsideTop:
            acc_r = c_res_acc;
            vel_r = c_res_vel;
            break;
          case BBLocation::OutsideTopLeft:
            acc_r = c_res_acc;
            acc_c = c_res_acc;
            vel_r = c_res_vel;
            vel_c = c_res_vel;
            break;
          case BBLocation::OutsideLeft:
            acc_c = c_res_acc;
            vel_c = c_res_vel;
            break;
          case BBLocation::OutsideBottomLeft:
            acc_r = -c_res_acc;
            acc_c = c_res_acc;
            vel_r = -c_res_vel;
            vel_c = c_res_vel;
            break;
          case BBLocation::OutsideBottom:
            acc_r = -c_res_acc;
            vel_r = -c_res_vel;
            break;
          case BBLocation::OutsideBottomRight:
            acc_r = -c_res_acc;
            acc_c = -c_res_acc;
            vel_r = -c_res_vel;
            vel_c = -c_res_vel;
            break;
          case BBLocation::OutsideRight:
            acc_c = -c_res_acc;
            vel_c = -c_res_vel;
            break;
          case BBLocation::OutsideTopRight:
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
  
    void init(const std::vector<std::unique_ptr<Weapon>>& all_weapons,
              const std::vector<std::unique_ptr<Armour>>& all_armour)
    {
      pos_r = static_cast<float>(pos.r);
      pos_c = static_cast<float>(pos.c);
      
      npc_race = rnd::rand_enum<Race>();
      npc_class = rnd::rand_enum<Class>();
      
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
          fierceness = 2;
          animal = false;
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
          fierceness = 1;
          animal = false;
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
          fierceness = 1;
          animal = false;
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
          fierceness = 0;
          animal = false;
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
          fierceness = 0;
          animal = false;
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
          fierceness = 2;
          animal = false;
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
          fierceness = 4;
          animal = false;
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
          fierceness = 5;
          animal = false;
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
          fierceness = 4;
          animal = false;
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
          fierceness = 4;
          animal = false;
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
          fierceness = 5;
          animal = false;
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
          fierceness = 7;
          animal = false;
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
          fierceness = 7;
          animal = rnd::rand_bool();
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
          fierceness = 6;
          animal = false;
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
          fierceness = 7;
          animal = false;
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
          fierceness = 5;
          animal = true;
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
          fierceness = 6;
          animal = true;
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
          fierceness = 3;
          animal = false;
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
          fierceness = 1;
          animal = false;
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
          fierceness = 7;
          animal = false;
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
          fierceness = 4;
          animal = true;
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
          fierceness = 4;
          animal = true;
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
          fierceness = 5;
          animal = true;
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
          fierceness = 4;
          animal = true;
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
          fierceness = 2;
          animal = false;
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
          fierceness = 8;
          animal = true;
          break;
        case Race::NUM_ITEMS:
          std::cerr << "Illegal race: NUM_ITEMS!" << std::endl;
          break;
        default:
          std::cerr << "Unknown race (" + std::to_string(static_cast<int>(npc_race)) + ")!";
          break;
      }
      
      orig_style = style;
      
      if (!animal)
      {
        if (!all_weapons.empty())
        {
          auto f_pick_weapon = [&](WeaponDistType dist_type)
          {
            const int num_weapons = stlutils::sizeI(all_weapons);
            int ctr = 0;
            do
            {
              int idx = rnd::rand_idx(num_weapons);
              auto* weapon = all_weapons[idx].get();
              if (weapon->exists && weapon->dist_type == dist_type && !weapon->picked_up)
              {
                weapon->picked_up = true;
                return idx;
              }
            } while (ctr++ < 10);
            return -1;
          };
          
          auto f_pick_armour = [&]()
          {
            const int num_armour = stlutils::sizeI(all_armour);
            int ctr = 0;
            do
            {
              int idx = rnd::rand_idx(num_armour);
              auto* armour = all_armour[idx].get();
              if (armour->exists && !armour->picked_up)
              {
                armour->picked_up = true;
                return idx;
              }
            } while (ctr++ < 10);
            return -1;
          };
          
          if (!rnd::one_in(4)) // 3 of 4 have a melee weapon.
            melee_weapon_idx = f_pick_weapon(WeaponDistType_Melee);
          if (rnd::one_in(4)) // 1 of 4 have a ranged weapon.
            ranged_weapon_idx = f_pick_weapon(WeaponDistType_Ranged);
          if (!rnd::one_in(3)) // 2 of 3 have armour.
            armour_idx = f_pick_armour();
        }
      }
    }
    
    void set_visibility(bool use_fog_of_war, bool fow_near, bool is_night)
    {
      visible = !((use_fog_of_war && this->fog_of_war) ||
                  ((this->is_underground || is_night) && !this->light));
      visible_near = !((use_fog_of_war && (this->fog_of_war || !fow_near)) ||
                       ((this->is_underground || is_night) && !this->light));
    }
    
    void trigger_hostility(const RC& pc_pos)
    {
      if (dist_to_pc < c_dist_hostile_hyst_on)
          is_hostile = true;
    }
    
    void update(const RC& pc_pos, BSPNode* pc_room, Corridor* pc_corr,
                Environment* environment,
                bool do_los_terrainos, bool do_move,
                float time, float dt)
    {
      if (health <= 0)
      {
        if (trg_death.once())
          death_time_s = time;
        character = '&';
        style = { Color::Red, Color::DarkGray };
        return;
      }
      
      if (do_los_terrainos)
      {
        update_los();
        update_terrain();
      }
      
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
      
      dist_to_pc = distance(pos, pc_pos);
      
      was_hostile = is_hostile;
      if (enemy)
      {
        if (dist_to_pc < c_dist_hostile_hyst_on)
          is_hostile = true;
      }
      if (dist_to_pc > c_dist_hostile_hyst_off)
        is_hostile = false;
      
      can_see_pc = false;
      if (inside_room)
      {
        if (pc_corr != nullptr)
        {
          for (int i = 0; i < 2; ++i)
            if (stlutils::contains(curr_room->doors, pc_corr->doors[i]))
              if (pc_corr->doors[i]->open_or_no_door())
              {
                can_see_pc = true;
                break;
              }
        }
        else if (pc_room == curr_room)
          can_see_pc = true;
      }
      if (inside_corr)
      {
        if (pc_room != nullptr)
        {
          for (int i = 0; i < 2; ++i)
            if (stlutils::contains(pc_room->doors, curr_corridor->doors[i]))
              if (curr_corridor->doors[i]->open_or_no_door())
              {
                can_see_pc = true;
                break;
              }
        }
        else if (pc_corr == curr_corridor)
          can_see_pc = true;
      }
      
      if (wants_to_attack() && dist_to_pc < c_dist_fight_melee)
        state = State::FightMelee;
      else if (wants_to_attack() && dist_to_pc < c_dist_fight_ranged && ranged_weapon_idx != -1)
        state = State::FightRanged;
      else if (wants_to_attack() && dist_to_pc < c_dist_pursue)
        state = State::Pursue;
      else if (!can_see_pc || dist_to_pc > c_dist_patroll)
        state = State::Patroll;
      
      if (allow_move())
        move(pc_pos, environment, dt);
      
      if (inside_room && curr_room != nullptr)
      {
        fog_of_war = curr_room->is_in_fog_of_war(pos);
        light = curr_room->is_in_light(pos);
        is_underground = environment->is_underground(curr_floor, curr_room);
      }
      else if (inside_corr && curr_corridor != nullptr)
      {
        fog_of_war = curr_corridor->is_in_fog_of_war(pos);
        light = curr_corridor->is_in_light(pos);
        is_underground = environment->is_underground(curr_floor, curr_corridor);
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
          case State::FightMelee:
            style.bg_color = Color::DarkRed;
            break;
          case State::FightRanged:
            style.bg_color = Color::DarkCyan;
            break;
          case State::NUM_ITEMS:
            break;
        }
      }
      else if (was_debug && health > 0)
        style = orig_style;
        
      was_debug = debug;
      
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
    
    float distance_to_pc() const
    {
      return dist_to_pc;
    }
    
    bool wants_to_attack() const
    {
      return (enemy || is_hostile) && can_see_pc;
    }
    
    int calc_armour_class(const std::vector<std::unique_ptr<Armour>>& all_armour) const
    {
      return armor_class + (armour_idx == -1 ? 0 : all_armour[armour_idx]->protection) + (dexterity / 2);
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
    
    int get_ranged_attack_bonus() const
    {
      return dexterity / 2 + strength / 4;
    }
    
    virtual void serialize(std::vector<std::string>& lines) const override
    {
      PlayerBase::serialize(lines);
      
      sg::write_var(lines, SG_WRITE_VAR(pos_r));
      sg::write_var(lines, SG_WRITE_VAR(pos_c));
      sg::write_var(lines, SG_WRITE_VAR(vel_r));
      sg::write_var(lines, SG_WRITE_VAR(vel_c));
      sg::write_var(lines, SG_WRITE_VAR(acc_r));
      sg::write_var(lines, SG_WRITE_VAR(acc_c));
      sg::write_var(lines, SG_WRITE_VAR(acc_step));
      sg::write_var(lines, SG_WRITE_VAR(acc_lim));
      sg::write_var(lines, SG_WRITE_VAR(vel_lim));
      sg::write_var(lines, SG_WRITE_VAR(prob_change_acc));
      sg::write_var(lines, SG_WRITE_VAR(prob_slow_fast));
      sg::write_var(lines, SG_WRITE_VAR(acc_factor));
      sg::write_var(lines, SG_WRITE_VAR(vel_factor));
      sg::write_var(lines, SG_WRITE_VAR(slow));
      sg::write_var_enum(lines, SG_WRITE_VAR(state));
      sg::write_var(lines, SG_WRITE_VAR(debug));
      sg::write_var(lines, SG_WRITE_VAR(wall_coll_resolve));
      sg::write_var(lines, SG_WRITE_VAR(wall_coll_resolve_ctr));
      sg::write_var(lines, SG_WRITE_VAR(fog_of_war));
      sg::write_var(lines, SG_WRITE_VAR(light));
      sg::write_var(lines, SG_WRITE_VAR(visible));
      sg::write_var(lines, SG_WRITE_VAR(visible_near));
      sg::write_var(lines, SG_WRITE_VAR(is_underground));
      sg::write_var(lines, SG_WRITE_VAR(inside_room));
      sg::write_var(lines, SG_WRITE_VAR(inside_corr));
      sg::write_var(lines, SG_WRITE_VAR(enemy));
      sg::write_var(lines, SG_WRITE_VAR(armor_class));
      sg::write_var_enum(lines, SG_WRITE_VAR(npc_race));
      sg::write_var_enum(lines, SG_WRITE_VAR(npc_class));
      sg::write_var(lines, SG_WRITE_VAR(is_hostile));
      sg::write_var(lines, SG_WRITE_VAR(was_hostile));
      // OneShot trg_info_hostile_npc;
      sg::write_var(lines, SG_WRITE_VAR(death_time_s));
      // OneShot trg_death;
    }
    
    virtual std::vector<std::string>::iterator deserialize(std::vector<std::string>::iterator it_line_begin,
                                                           std::vector<std::string>::iterator it_line_end,
                                                           Environment* environment) override
    {
      it_line_begin = PlayerBase::deserialize(it_line_begin, it_line_end, environment);
      for (auto it_line = it_line_begin + 1; it_line != it_line_end; ++it_line)
      {
        if (sg::read_var(&it_line, SG_READ_VAR(pos_r))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(pos_c))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(vel_r))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(vel_c))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(acc_r))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(acc_c))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(acc_step))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(acc_lim))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(vel_lim))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(prob_change_acc))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(prob_slow_fast))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(acc_factor))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(vel_factor))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(slow))) {}
        else if (sg::read_var_enum(&it_line, SG_READ_VAR(state))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(debug))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(wall_coll_resolve))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(wall_coll_resolve_ctr))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(fog_of_war))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(light))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(visible))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(visible_near))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(is_underground))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(inside_room))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(inside_corr))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(enemy))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(armor_class))) {}
        else if (sg::read_var_enum(&it_line, SG_READ_VAR(npc_race))) {}
        else if (sg::read_var_enum(&it_line, SG_READ_VAR(npc_class))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(is_hostile))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(was_hostile))) {}
        // OneShot trg_info_hostile_npc;
        else if (sg::read_var(&it_line, SG_READ_VAR(death_time_s)))
        {
          return it_line;
        }
        // OneShot trg_death;
      }
      return it_line_end;
    }

  };
  
}
