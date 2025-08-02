//
//  PlayerBase.h
//  DungGine
//
//  Created by Rasmus Anthin on 2024-08-15.
//

#pragma once
#include "DungObject.h"
#include "SaveGame.h"
#include <Termin8or/StringConversion.h>


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
    
    virtual void serialize(std::vector<std::string>& lines) const override
    {
      DungObject::serialize(lines);
    
      sg::write_var(lines, SG_WRITE_VAR(shape));
      sg::write_var(lines, SG_WRITE_VAR(dir));
      sg::write_var(lines, SG_WRITE_VAR(pos_r));
      sg::write_var(lines, SG_WRITE_VAR(pos_c));
      //sg::write_var(lines, SG_WRITE_VAR(life_time));
      sg::write_var(lines, SG_WRITE_VAR(time_stamp));
      sg::write_var(lines, SG_WRITE_VAR(speed));
      sg::write_var(lines, SG_WRITE_VAR(alive));
      sg::write_var(lines, SG_WRITE_VAR(terrain));
    }
    
    virtual std::vector<std::string>::iterator deserialize(std::vector<std::string>::iterator it_line_begin,
                                                           std::vector<std::string>::iterator it_line_end,
                                                           Environment* environment) override
    {
      it_line_begin = DungObject::deserialize(it_line_begin, it_line_end, environment);
      for (auto it_line = it_line_begin + 1; it_line != it_line_end; ++it_line)
      {
        if (sg::read_var(&it_line, SG_READ_VAR(shape))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(dir))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(pos_r))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(pos_c))) {}
        //else if (sg::read_var(&it_line, SG_READ_VAR(life_time))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(time_stamp))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(speed))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(alive))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(terrain)))
        {
          return it_line;
        }
      }
    
      return it_line_end;
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
    
    virtual void serialize(std::vector<std::string>& lines) const
    {
      sg::write_var(lines, SG_WRITE_VAR(character));
      sg::write_var(lines, SG_WRITE_VAR(style));
      sg::write_var(lines, SG_WRITE_VAR(pos));
      sg::write_var(lines, SG_WRITE_VAR(last_pos));
      sg::write_var(lines, SG_WRITE_VAR(los_r));
      sg::write_var(lines, SG_WRITE_VAR(los_c));
      sg::write_var(lines, SG_WRITE_VAR(last_los_r));
      sg::write_var(lines, SG_WRITE_VAR(last_los_c));
      sg::write_var(lines, SG_WRITE_VAR(is_moving));
      lines.emplace_back("curr_room:id");
      lines.emplace_back(std::to_string(curr_room != nullptr ? curr_room->id : -1)); // PC:279
      lines.emplace_back("curr_corridor:id");
      lines.emplace_back(std::to_string(curr_corridor != nullptr ? curr_corridor->id : -1)); // PC:218
      sg::write_var(lines, SG_WRITE_VAR(health));
      sg::write_var(lines, SG_WRITE_VAR(strength));
      sg::write_var(lines, SG_WRITE_VAR(dexterity));
      sg::write_var(lines, SG_WRITE_VAR(endurance));
      sg::write_var(lines, SG_WRITE_VAR(weakness));
      sg::write_var(lines, SG_WRITE_VAR(thac0));
      sg::write_var(lines, SG_WRITE_VAR(weight_strain));
      sg::write_var(lines, SG_WRITE_VAR(on_terrain));
      sg::write_var(lines, SG_WRITE_VAR(can_swim));
      sg::write_var(lines, SG_WRITE_VAR(can_fly));
      
      lines.emplace_back("blood_splats");
      for (const auto& bs : blood_splats)
        bs.serialize(lines);
      
      sg::write_var(lines, SG_WRITE_VAR(cached_fight_offs));
      sg::write_var(lines, SG_WRITE_VAR(cached_fight_style));
      sg::write_var(lines, SG_WRITE_VAR(cached_fight_str));
    }
    
    virtual std::vector<std::string>::iterator deserialize(std::vector<std::string>::iterator it_line_begin,
                                                           std::vector<std::string>::iterator it_line_end,
                                                           Environment* environment)
    {
      for (auto it_line = it_line_begin; it_line != it_line_end; ++it_line)
      {
        if (sg::read_var(&it_line, SG_READ_VAR(character))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(style))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(pos))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(last_pos))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(los_r))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(los_c))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(last_los_r))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(last_los_c))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(is_moving))) {}
        else if (*it_line == "curr_room:id")
        {
          ++it_line;
          auto room_id = std::atoi(it_line->c_str());
          auto* room = environment->find_room(room_id);
          if (room != nullptr)
            curr_room = room;
          if (room_id != -1 && room == nullptr)
            std::cerr << "Error in PlayerBase when parsing curr_room:id = \"" << room_id << "\"!\n";
        }
        else if (*it_line == "curr_corridor:id")
        {
          ++it_line;
          auto corr_id = std::atoi(it_line->c_str());
          auto* corr = environment->find_corridor(corr_id);
          if (corr != nullptr)
            curr_corridor = corr;
          if (corr_id != -1 && corr == nullptr)
            std::cerr << "Error in PlayerBase when parsing curr_corridor:id = \"" << corr_id << "\"!\n";
        }
        else if (sg::read_var(&it_line, SG_READ_VAR(health))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(strength))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(dexterity))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(endurance))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(weakness))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(thac0))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(weight_strain))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(on_terrain))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(can_swim))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(can_fly))) {}
        else if (*it_line == "blood_splats")
          for (auto& bs : blood_splats)
            it_line = bs.deserialize(it_line + 1, it_line_end, environment);
        else if(sg::read_var(&it_line, SG_READ_VAR(cached_fight_offs))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(cached_fight_style))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(cached_fight_str)))
        {
          return it_line;
        }
      }
      return it_line_end;
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
