//
//  Items.h
//  DungGine
//
//  Created by Rasmus Anthin on 2024-06-21.
//

#pragma once

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
    float weight = 0.f; // kg-ish.
    float price = 0.f;  // SEK-ish.
    bool is_underground = false;
    BSPNode* curr_room = nullptr;
    Corridor* curr_corridor = nullptr;
    
    virtual void set_visibility(bool use_fog_of_war, bool is_night)
    {
      visible = !(picked_up ||
                  (use_fog_of_war && this->fog_of_war) ||
                  ((this->is_underground || is_night) && !this->light));
    }
  };
  
  struct Key : Item
  {
    Key()
    {
      character = 'F';
      style.fg_color = color::get_random_color(key_fg_palette);
      weight = 0.1f;
      price = math::roundI(20*rnd::randn_clamp(20.f, 30.f, 0.f, 1e4f))/20.f;
    }
    
    int key_id = 0;
  };
  
  struct Lamp : Item
  {
    Lamp()
    {
      character = 'Y';
      style.fg_color = Color::Yellow;
      weight = 0.4f;
      price = math::roundI(20*rnd::randn_clamp(200.f, 100.f, 0.f, 1e4f))/20.f;
    }
    enum class LampType { Isotropic, Directional, NUM_ITEMS };
    LampType type = LampType::Isotropic;
    
    virtual void set_visibility(bool use_fog_of_war, bool is_night) override
    {
      visible = !(picked_up ||
                  (use_fog_of_war && this->fog_of_war));
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
      weight = 2.f;
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
      weight = 2.f;
      price = math::roundI(20*rnd::randn_clamp(5e2f, 500.f, 0.f, 1e4f))/20.f;
      type = "dagger";
      damage = rnd::randn_clamp_int(3.f, 3.f, 1, 7);
    }
  };
  
  struct Flail : Weapon
  {
    Flail()
    {
      character = '#';
      style.fg_color = Color::DarkGray;
      weight = 2.f;
      price = math::roundI(20*rnd::randn_clamp(1e3f, 500.f, 0.f, 5e5f))/20.f;
      type = "flail";
      damage = rnd::randn_clamp_int(5.f, 10.f, 3, 30);
    }
  };
  
  struct Potion : Item
  {
    int health = 1;
    bool poison = false;
    
    Potion()
    {
      character = rnd::rand_select<char>({ 'u', 'U', 'b' });
      style.fg_color = color::get_random_color(potion_fg_palette);
      health = rnd::randn_clamp_int(5.f, 10.f, 0, 100);
      poison = rnd::one_in(50);
    }
    
    int get_hp() const
    {
      return poison ? -health : health;
    }
  };
  
}
