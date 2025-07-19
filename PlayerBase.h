//
//  PlayerBase.h
//  DungGine
//
//  Created by Rasmus Anthin on 2024-08-15.
//

#pragma once
#include "DungObject.h"


namespace dung
{

  struct BloodSplat : DungObject
  {
    int shape = 1;
    
    // For diffusion on liquids.
    RC dir { 0, 0 };
    float pos_r = 0.f;
    float pos_c = 0.f;
    const float life_time = 5.f;
    float time_stamp = 0.f;
    float speed = 0.05f;
    bool alive = true;
    Terrain terrain = Terrain::Void;
    Environment* environment;
    
    BloodSplat(Environment* env, const RC& p, int s, float ts, const RC& d)
      : shape(s)
      , dir(d)
      , time_stamp(ts)
      , environment(env)
    {
      pos = p;
      pos_r = static_cast<float>(p.r);
      pos_c = static_cast<float>(p.c);
    }
    
    void set_visibility(bool use_fog_of_war, bool is_night)
    {
      visible = !((use_fog_of_war && this->fog_of_war) ||
                  ((this->is_underground || is_night) && !this->light));
    }
    
    void update(float curr_time)
    {
      terrain = environment->get_terrain(pos);
      
      alive = curr_time < time_stamp + life_time;
    
      if (is_wet(terrain) && alive)
      {
        pos_r += speed * (dir.r + rnd::rand_float(-1.5f, +1.5f) + 1.5f*std::sin(math::c_2pi*2.5f*curr_time));
        pos_c += speed * (dir.c + rnd::rand_float(-1.5f, +1.5f) + 1.5f*std::cos(math::c_2pi*2.5f*curr_time));
        auto pos_ri = static_cast<int>(math::roundI(pos_r));
        auto pos_ci = static_cast<int>(math::roundI(pos_c));
        if (environment->is_inside_any_room({ pos_ri, pos_ci }))
        {
          pos.r = pos_ri;
          pos.c = pos_ci;
        }
      }
      
      terrain = environment->get_terrain(pos);
    }
  };

  struct PlayerBase
  {
    char character;
    Style style;
  
    RC pos, last_pos;
    float los_r = 0.f;
    float los_c = 0.f;
    float last_los_r = 0.f;
    float last_los_c = 0.f;
    bool is_moving = false;
    
    BSPNode* curr_room = nullptr;
    Corridor* curr_corridor = nullptr;
    
    int health = globals::max_health;
    int strength = 10;
    int dexterity = 10;
    int endurance = 10;
    int weakness = 0;
    int thac0 = 1;
    float weight_strain = 0.f;
    
    Terrain on_terrain = Terrain::Default;
    bool can_swim = true;
    bool can_fly = false;
    
    std::vector<BloodSplat> blood_splats;
    
    RC cached_fight_offs { 0, 0 };
    styles::Style cached_fight_style;
    std::string cached_fight_str;
    
    bool allow_move()
    {
      bool can_move_base = true;
      if (weakness > 0)
        can_move_base = !rnd::one_in(2 + strength - weakness);
        
      if (can_move_base)
      {
        auto dry_resistance = get_dry_resistance(on_terrain);
        if (dry_resistance.has_value())
          return rnd::rand() >= dry_resistance.value();
        
        auto wet_viscosity = get_wet_viscosity(on_terrain);
        if (wet_viscosity.has_value())
          return rnd::rand() >= wet_viscosity.value();
      }
        
      return false;
    }
    
  protected:
    void update_los()
    {
      is_moving = false;
      if (pos != last_pos)
      {
        los_r = static_cast<float>(pos.r - last_pos.r);
        los_c = static_cast<float>(pos.c - last_pos.c);
        los_r += last_los_r;
        los_c += last_los_c;
        math::normalize(los_r, los_c);
        last_los_r = los_r;
        last_los_c = los_c;
        is_moving = true;
      }
      last_pos = pos;
    }
  
    void update_terrain()
    {
      float fluid_damage = 0.01f;
      switch (on_terrain)
      {
        case Terrain::Water: fluid_damage = 0.005f; break;
        case Terrain::Poison: fluid_damage = 0.02f; break;
        case Terrain::Acid: fluid_damage = 0.04f; break;
        case Terrain::Tar: fluid_damage = 0.015f; break;
        case Terrain::Lava: fluid_damage = 0.4f; break;
        case Terrain::Swamp: fluid_damage = 0.01f; break;
        default: break;
      }
      
      if (is_wet(on_terrain) && can_swim && !can_fly)
      {
        if (rnd::one_in(endurance) && weakness < strength)
          weakness++;
        
        if (rnd::one_in(1 + strength - weakness))
          health -= math::roundI(globals::max_health*fluid_damage);
      }
      else if (weight_strain > 0.f)
      {
        weakness = static_cast<int>(weight_strain * strength);
      }
      else
      {
        if (rnd::one_in(2) && 0 < weakness)
          weakness--;
      }
    }
  };

}
