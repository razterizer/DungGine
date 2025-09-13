//
//  Items.h
//  DungGine
//
//  Created by Rasmus Anthin on 2024-06-21.
//

#pragma once
#include "Globals.h"
#include "DungObject.h"
#include "SaveGame.h"

namespace dung
{
  
  struct Item : DungObject
  {
    virtual ~Item() = default;
  
    // exists is set to false when dropping an item in the water
    //   or disposing it using a vanishing spell.
    //   This helps to preserve index-based item idcs
    //   without having to erase items from item vectors.
    bool exists = true;
    
    bool picked_up = false;
    Style style = { Color::White, Color::Transparent2 };
    char character = '?';
    bool visible_near = false;
    float weight = 0.f; // kg-ish.
    float price = 0.f;  // SEK-ish.
    
    virtual void change_fg_color() = 0;
    
    virtual void set_visibility(bool use_fog_of_war, bool fow_near, bool is_night)
    {
      visible = !(!exists ||
                  picked_up ||
                  (use_fog_of_war && this->fog_of_war) ||
                  ((this->is_underground || is_night) && !this->light));
      visible_near = !(!exists ||
                       picked_up ||
                       (use_fog_of_war && (this->fog_of_war || !fow_near)) ||
                       ((this->is_underground || is_night) && !this->light));
    }
    
    virtual void serialize(std::vector<std::string>& lines) const override
    {
      DungObject::serialize(lines);
      
      sg::write_var(lines, SG_WRITE_VAR(exists));
      sg::write_var(lines, SG_WRITE_VAR(picked_up));
      sg::write_var(lines, SG_WRITE_VAR(style));
      sg::write_var(lines, SG_WRITE_VAR(character));
      sg::write_var(lines, SG_WRITE_VAR(visible_near));
      sg::write_var(lines, SG_WRITE_VAR(weight));
      sg::write_var(lines, SG_WRITE_VAR(price));
    }
    
    virtual std::vector<std::string>::iterator deserialize(std::vector<std::string>::iterator it_line_begin,
                                                           std::vector<std::string>::iterator it_line_end,
                                                           Environment* environment) override
    {
      it_line_begin = DungObject::deserialize(it_line_begin, it_line_end, environment);
      for (auto it_line = it_line_begin + 1; it_line != it_line_end; ++it_line)
      {
        if (sg::read_var(&it_line, SG_READ_VAR(exists))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(picked_up))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(style))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(character))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(visible_near))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(weight))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(price)))
        {
          return it_line;
        }
      }
    
      return it_line_end;
    }
  };
  
  struct Key : Item
  {
    Key()
    {
      character = 'F';
      style.fg_color = t8::get_random_color(key_fg_palette);
      weight = rnd::randn_range_clamp(0.01f, 0.1f);
      price = math::roundI(20*rnd::randn_clamp(20.f, 30.f, 0.f, 1e4f))/20.f;
    }
    
    virtual void change_fg_color() override
    {
      style.fg_color = t8::get_random_color(key_fg_palette);
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
    
    virtual void change_fg_color() override
    {
      switch (lamp_type)
      {
        case LampType::MagicLamp:
          style.fg_color = Color::DarkMagenta;
          break;
        case LampType::Lantern:
          style.fg_color = t8::get_random_color({ Color::Red, Color::Green });
          break;
        case LampType::Torch:
          style.fg_color = Color::DarkYellow;
          break;
        default:
          break;
      }
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
          style.fg_color = t8::get_random_color({ Color::Red, Color::Green });
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
      visible = !(!exists ||
                  picked_up ||
                  (use_fog_of_war && this->fog_of_war));
                  
      visible_near = !(!exists ||
                       picked_up ||
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
    
    virtual void serialize(std::vector<std::string>& lines) const override
    {
      Item::serialize(lines);
    
      sg::write_var(lines, SG_WRITE_VAR(life_time_s));
    }
    
    virtual std::vector<std::string>::iterator deserialize(std::vector<std::string>::iterator it_line_begin,
                                                           std::vector<std::string>::iterator it_line_end,
                                                           Environment* environment) override
    {
      it_line_begin = Item::deserialize(it_line_begin, it_line_end, environment);
      for (auto it_line = it_line_begin + 1; it_line != it_line_end; ++it_line)
      {
        if (sg::read_var(&it_line, SG_READ_VAR(life_time_s)))
        {
          return it_line;
        }
      }
      return it_line_end;
    }
  };
  
  enum WeaponDistType { WeaponDistType_Melee, WeaponDistType_Ranged };
  
  struct Weapon : Item
  {
    WeaponDistType dist_type = WeaponDistType_Melee;
    int damage = 1;
    //bool rusty = false;
    //bool sharpened = false;
    //bool poisonous = false;
    std::string type;
    float attack_speed = 15.f; // max 15 aps (demo fps).
    float projectile_speed = 15.f; // max 15 aps (demo fps).
    float spread_sigma_rad = 0.f;
    std::array<char, 8> projectile_characters; // { 0, 45, 90, 135, 180, 225, 270, 315 } degrees.
    Color projectile_fg_color = Color::Transparent2;
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
      damage = rnd::randn_clamp_int(3.f, 1.f, 1, 7);
      attack_speed = rnd::randn_range_clamp(2.f, 3.f);
      dist_type = WeaponDistType_Melee;
    }
    
    virtual void change_fg_color() override
    {
      style.fg_color = Color::DarkGray;
    }
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
      damage = rnd::randn_clamp_int(7.f, 4., 4, 50);
      attack_speed = rnd::randn_range_clamp(1.25f, 1.75f);
      dist_type = WeaponDistType_Melee;
    }
    
    virtual void change_fg_color() override
    {
      style.fg_color = Color::DarkGray;
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
      damage = rnd::randn_clamp_int(7.5f, 5.f, 3, 30);
      attack_speed = rnd::randn_range_clamp(0.8f, 1.2f);
      dist_type = WeaponDistType_Melee;
    }
    
    virtual void change_fg_color() override
    {
      style.fg_color = Color::LightGray;
    }
  };
  
  struct MorningStar : Weapon
  {
    MorningStar()
    {
      character = 'i';
      style.fg_color = Color::DarkGray;
      weight = rnd::randn_range_clamp(1.5f, 2.8f);
      price = math::roundI(20*rnd::randn_clamp(1e3f, 500.f, 0.f, 5e5f))/20.f;
      type = "morning star";
      damage = rnd::randn_clamp_int(9.f, 5.f, 5, 35);
      attack_speed = rnd::randn_range_clamp(0.7f, 1.3f);
      dist_type = WeaponDistType_Melee;
    }
    
    virtual void change_fg_color() override
    {
      style.fg_color = Color::LightGray;
    }
  };
  
  struct Sling : Weapon
  {
    Sling()
    {
      character = 's';
      style.fg_color = Color::DarkRed;
      weight = rnd::randn_range_clamp(0.02f, 0.5f);
      price = math::roundI(20*rnd::randn_clamp(200.f, 150.f, 0.f, 5e2f))/20.f;
      type = "sling";
      damage = rnd::randn_clamp_int(5.f, 2.f, 2, 15);
      attack_speed = rnd::randn_range_clamp(1.8f, 2.2f);
      projectile_speed = rnd::randn_clamp(8.5f, 0.6f, 8.f, 9.f);
      spread_sigma_rad = 0.4f;
      dist_type = WeaponDistType_Ranged;
      stlutils::fill(projectile_characters, '*');
      projectile_fg_color = Color::DarkGray;
    }
    
    virtual void change_fg_color() override
    {
      style.fg_color = Color::DarkGray;
    }
  };
  
  struct Bow : Weapon
  {
    Bow()
    {
      character = rnd::rand_select<char>({ '(', ')', '{', '}' });;
      style.fg_color = Color::DarkRed;
      weight = rnd::randn_range_clamp(0.4f, 4.f);
      price = math::roundI(20*rnd::randn_clamp(3e3f, 1500.f, 0.f, 5e5f))/20.f;
      type = "bow";
      damage = rnd::randn_clamp_int(8.f, 5.f, 3, 40);
      attack_speed = rnd::randn_range_clamp(1.f, 2.f);
      projectile_speed = rnd::randn_clamp(9.5f, 0.6f, 9.f, 10.f);
      spread_sigma_rad = 0.1f;
      dist_type = WeaponDistType_Ranged;
      projectile_characters = { '-', '/', '|', '\\', '-', '/', '|', '\\' };
      projectile_fg_color = Color::Yellow;
    }
    
    virtual void change_fg_color() override
    {
      style.fg_color = Color::DarkYellow;
    }
  };
  
  struct Crossbow : Weapon
  {
    Crossbow()
    {
      character = rnd::rand_select<char>({ '[', ']' });
      style.fg_color = t8::get_random_color(crossbow_fg_palette);
      weight = rnd::randn_range_clamp(1.f, 20.f);
      price = math::roundI(20*rnd::randn_clamp(1e4f, 1500.f, 0.f, 5e5f))/20.f;
      type = "crossbow";
      damage = rnd::randn_clamp_int(12.f, 5.f, 3, 55);
      attack_speed = rnd::randn_range_clamp(0.6f, 0.8f);
      projectile_speed = rnd::randn_clamp(8.5f, 0.6f, 8.f, 10.f);
      spread_sigma_rad = 0.02f;
      dist_type = WeaponDistType_Ranged;
      projectile_characters = { '-', '/', '|', '\\', '-', '/', '|', '\\' };
      projectile_fg_color = Color::LightGray;
    }
    
    virtual void change_fg_color() override
    {
      style.fg_color = t8::get_random_color(crossbow_fg_palette);
    }
  };
  
  struct Potion : Item
  {
    int health = 1;
    bool poison = false;
    
    Potion()
    {
      character = rnd::rand_select<char>({ 'u', 'U', 'b' });
      style.fg_color = t8::get_random_color(potion_fg_palette);
      weight = rnd::randn_range_clamp(0.02f, 0.4f);
      price = math::roundI(20*rnd::randn_clamp(1e3f, 500.f, 0.f, 5e5f))/20.f;
      health = math::roundI(rnd::randn_clamp(.05f, 0.1f, 0, 1.f)*globals::max_health);
    }
    
    virtual void change_fg_color() override
    {
      style.fg_color = t8::get_random_color(potion_fg_palette);
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
    ARMOUR_Helmet,
    ARMOUR_NUM_ITEMS
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
    
    virtual void change_fg_color() override
    {
      style.fg_color = Color::Blue;
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
    
    virtual void change_fg_color() override
    {
      style.fg_color = Color::LightGray;
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
    
    virtual void change_fg_color() override
    {
      style.fg_color = Color::Cyan;
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
    
    virtual void change_fg_color() override
    {
      style.fg_color = Color::Cyan;
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
    
    virtual void change_fg_color() override
    {
      style.fg_color = Color::LightGray;
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
    
    virtual void change_fg_color() override
    {
      style.fg_color = Color::Cyan;
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
    
    virtual void change_fg_color() override
    {
      style.fg_color = Color::Cyan;
    }
  };
}
