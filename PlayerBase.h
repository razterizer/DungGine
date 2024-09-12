//
//  PlayerBase.h
//  DungGine
//
//  Created by Rasmus Anthin on 2024-08-15.
//

#pragma once

namespace dung
{

  struct BloodSplat
  {
    RC pos;
    int shape = 1;
    BloodSplat(const RC& p, int s) : pos(p), shape(s) {}
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
      if (pos != last_pos)
      {
        los_r = pos.r - last_pos.r;
        los_c = pos.c - last_pos.c;
        los_r += last_los_r;
        los_c += last_los_c;
        math::normalize(los_r, los_c);
        last_los_r = los_r;
        last_los_c = los_c;
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
        weakness = weight_strain * strength;
      }
      else
      {
        if (rnd::one_in(2) && 0 < weakness)
          weakness--;
      }
    }
  };

}
