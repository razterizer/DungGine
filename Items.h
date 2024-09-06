//
//  Items.h
//  DungGine
//
//  Created by Rasmus Anthin on 2024-06-21.
//

#pragma once
#include "Globals.h"

namespace dung
{
  
  struct Item
  {
    virtual ~Item() = default;
  
    RC pos; // world pos
    bool picked_up = false;
    Style style = { Color::White, Color::Transparent2 };
    char character = '?';
    bool fog_of_war = true;
    bool light = false;
    bool visible = false;
    bool visible_near = false;
    float weight = 0.f; // kg-ish.
    float price = 0.f;  // SEK-ish.
    bool is_underground = false;
    BSPNode* curr_room = nullptr;
    Corridor* curr_corridor = nullptr;
    
    virtual void set_visibility(bool use_fog_of_war, bool fow_near, bool is_night)
    {
      visible = !(picked_up ||
                  (use_fog_of_war && this->fog_of_war) ||
                  ((this->is_underground || is_night) && !this->light));
      visible_near = !(picked_up ||
                       (use_fog_of_war && (this->fog_of_war || !fow_near)) ||
                       ((this->is_underground || is_night) && !this->light));
    }
  };
  
  struct Key : Item
  {
    Key()
    {
      character = 'F';
      style.fg_color = color::get_random_color(key_fg_palette);
      weight = rnd::randn_range_clamp(0.01f, 0.1f);
      price = math::roundI(20*rnd::randn_clamp(20.f, 30.f, 0.f, 1e4f))/20.f;
    }
    
    int key_id = 0;
  };
  
  struct Lamp : Item
  {
    enum class LampType { MagicLamp, Lantern, Torch, NUM_ITEMS };
  
    Lamp()
    {
      character = 'Y';
      style.fg_color = Color::Yellow;
      weight = 0.4f;
      price = math::roundI(20*rnd::randn_clamp(200.f, 100.f, 0.f, 1e4f))/20.f;
    }
    void init_rand()
    {
      init_rand(rnd::rand_enum<Lamp::LampType>());
    }
    void init_rand(Lamp::LampType a_lamp_type)
    {
      radius = rnd::randn_clamp(globals::max_fow_radius*0.8f, globals::max_fow_radius*0.25f, 1.5f, globals::max_fow_radius);
      radius_0 = radius;
      lamp_type = a_lamp_type;
      switch (lamp_type)
      {
        case LampType::MagicLamp:
          light_type = LightType::Isotropic;
          angle_deg = 0.f;
          life_time_s = rnd::randn_clamp(800.f, 350.f, 420.f, 1800.f); // 7 - 30 min.
          character = '*';
          style.fg_color = Color::Magenta;
          weight = rnd::randn_range_clamp(0.05f, 0.2f);
          break;
        case LampType::Lantern:
          light_type = LightType::Directional;
          angle_deg = rnd::randn_range_clamp(2.f, 90.f);
          life_time_s = rnd::randn_clamp(400.f, 350.f, 180.f, 900.f); // 3 - 15 min.
          character = 'G';
          style.fg_color = color::get_random_color({ Color::Red, Color::Green });
          weight = rnd::randn_range_clamp(0.05f, 0.3f);
          break;
        case LampType::Torch:
          light_type = LightType::Directional;
          angle_deg = rnd::randn_range_clamp(80.f, 358.f);
          life_time_s = rnd::randn_clamp(150.f, 350.f, 30.f, 300.f); // 0.5 - 5 min.
          character = 'Y';
          style.fg_color = Color::Yellow;
          weight = rnd::randn_range_clamp(0.4f, 1.5f);
          break;
        default:
          break;
      }
    }
    enum class LightType { Isotropic, Directional, NUM_ITEMS };
    LightType light_type = LightType::Isotropic;
    LampType lamp_type = LampType::MagicLamp;
    float radius = 2.5f;
    float radius_0 = 2.5f;
    float angle_deg = 45.f;
    float life_time_s = 200.f;
    float t_life_time = 0.f;
    float time_used_s = 0.f;
    
    void update(float dt)
    {
      time_used_s += dt;
      t_life_time = math::value_to_param(time_used_s, 0.f, life_time_s);
      radius = math::lerp(t_life_time, radius_0, 0.f);
    }
    
    virtual void set_visibility(bool use_fog_of_war, bool fow_near, bool is_night) override
    {
      visible = !(picked_up ||
                  (use_fog_of_war && this->fog_of_war));
                  
      visible_near = !(picked_up ||
                       (use_fog_of_war && (this->fog_of_war || !fow_near)));
    }
    
    std::string get_type_str() const
    {
      switch (lamp_type)
      {
        case LampType::MagicLamp:
          return "magic lamp";
        case LampType::Lantern:
          return "lantern";
        case LampType::Torch:
          return "torch";
        default:
          return "unknown lamp";
      }
    }
  };
  
  struct Weapon : Item
  {
    int damage = 1;
    bool rusty = false;
    bool sharpened = false;
    bool poisonous = false;
    std::string type;
  };
  
  struct Sword : Weapon
  {
    Sword()
    {
      character = 'T';
      style.fg_color = Color::LightGray;
      weight = rnd::randn_range_clamp(1.f, 5.f);
      price = math::roundI(20*rnd::randn_clamp(4e3f, 500.f, 0.f, 5e6f))/20.f;
      type = "sword";
      damage = rnd::randn_clamp_int(7.f, 10.f, 4, 50);
    }
  };
  
  struct Dagger : Weapon
  {
    Dagger()
    {
      character = 'V';
      style.fg_color = Color::LightGray;
      weight = rnd::randn_range_clamp(0.02f, 0.7f);
      price = math::roundI(20*rnd::randn_clamp(5e2f, 500.f, 0.f, 1e4f))/20.f;
      type = "dagger";
      damage = rnd::randn_clamp_int(3.f, 3.f, 1, 7);
    }
  };
  
  struct Flail : Weapon
  {
    Flail()
    {
      character = 'J';
      style.fg_color = Color::DarkGray;
      weight = rnd::randn_range_clamp(1.f, 1.8f);
      price = math::roundI(20*rnd::randn_clamp(1e3f, 500.f, 0.f, 5e5f))/20.f;
      type = "flail";
      damage = rnd::randn_clamp_int(5.f, 10.f, 3, 30);
    }
  };
  
  // MorningStar
  
  struct Potion : Item
  {
    int health = 1;
    bool poison = false;
    
    Potion()
    {
      character = rnd::rand_select<char>({ 'u', 'U', 'b' });
      style.fg_color = color::get_random_color(potion_fg_palette);
      weight = rnd::randn_range_clamp(0.02f, 0.4f);
      price = math::roundI(20*rnd::randn_clamp(1e3f, 500.f, 0.f, 5e5f))/20.f;
      health = math::roundI(rnd::randn_clamp(.05f, 0.1f, 0, 1.f)*globals::max_health);
      poison = rnd::one_in(50);
    }
    
    int get_hp() const
    {
      return poison ? -health : health;
    }
  };
  
  enum ArmourType
  {
    ARMOUR_Shield = 0,
    ARMOUR_Gambeson,
    ARMOUR_ChainMailleHauberk,
    ARMOUR_PlatedBodyArmour,
    ARMOUR_PaddedCoif,
    ARMOUR_ChainMailleCoif,
    ARMOUR_Helmet
  };
  
  struct Armour : Item
  {
    int protection = 1;
    std::string type;
  };
  
  struct Shield : Armour
  {
    Shield()
    {
      character = 'D';
      style.fg_color = Color::LightGray;
      type = "shield";
      price = math::roundI(20*rnd::randn_clamp(1e3f, 500.f, 0.f, 5e4f))/20.f;
      protection = rnd::randn_clamp_int(2.f, 15.f, 0, 50);
      weight = protection * 0.5f * (1.f + 0.6f*(rnd::rand() - 0.6f));
    }
  };
  
  struct Gambeson : Armour
  {
    Gambeson()
    {
      character = 'H';
      style.fg_color = Color::White;
      type = "gambeson";
      price = math::roundI(20*rnd::randn_clamp(5e2f, 200.f, 0.f, 5e3f))/20.f;
      protection = rnd::randn_clamp_int(0.5f, 12.f, 0, 10);
      weight = protection * 0.25f * (1.f + 0.3f*(rnd::rand() - 0.6f));
    }
  };
  
  struct ChainMailleHauberk : Armour
  {
    ChainMailleHauberk()
    {
      character = '#';
      style.fg_color = Color::LightGray;
      type = "chain maille hauberk";
      protection = rnd::randn_clamp_int(5.f, 18.f, 0, 40);
      weight = protection * 0.62f * (1.f + 0.5f*(rnd::rand() - 0.6f));
    }
  };
  
  struct PlatedBodyArmour : Armour
  {
    PlatedBodyArmour()
    {
      character = 'M';
      style.fg_color = Color::LightGray;
      type = "plated body armour";
      price = math::roundI(20*rnd::randn_clamp(2e4f, 5000.f, 0.f, 1e6f))/20.f;
      protection = rnd::randn_clamp_int(10.f, 20.f, 0, 100);
      weight = protection * 0.67f * (1.f + 0.6f*(rnd::rand() - 0.6f));
    }
  };
  
  struct PaddedCoif : Armour
  {
    PaddedCoif()
    {
      character = 'C';
      style.fg_color = Color::White;
      type = "padded coif";
      protection = rnd::randn_clamp_int(0.5f, 12.f, 0, 10);
      weight = protection * 0.016f * (1.f + 0.4f*(rnd::rand() - 0.6f));
    }
  };
  
  struct ChainMailleCoif : Armour
  {
    ChainMailleCoif()
    {
      character = '2';
      style.fg_color = Color::LightGray;
      type = "chain maille coif";
      protection = rnd::randn_clamp_int(5.f, 18.f, 0, 40);
      weight = protection * 0.061f * (1.f + 0.3f*(rnd::rand() - 0.6f));
    }
  };
  
  struct Helmet : Armour
  {
    Helmet()
    {
      character = 'Q';
      style.fg_color = Color::LightGray;
      type = "helmet";
      protection = rnd::randn_clamp_int(10.f, 20.f, 0, 100);
      weight = protection * 0.087f * (1.f + 0.4f*(rnd::rand() - 0.6f));
    }
  };
}
