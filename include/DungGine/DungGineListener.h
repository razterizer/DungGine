//
//  DungGineListener.h
//  DungGine
//
//  Created by Rasmus Anthin on 2024-08-17.
//

#pragma once
#include <Core/events/IListener.h>
#include <string>

namespace dung
{

  struct NPC;

  struct DungGineListener : IListener
  {
    virtual void on_fight_begin(NPC* npc) {}
    virtual void on_fight_end(NPC* npc) {}
    
    virtual void on_pc_death() {}
    virtual void on_npc_death() {}
    
    //virtual void on_pc_damage_begin() {}
    //virtual void on_pc_damage_end() {}
    
    virtual void on_scene_rebuild_request() {}
    virtual void on_save_game_request(std::string& filepath, unsigned int& curr_rnd_seed) {}
    virtual void on_load_game_request_pre(std::string& filepath) {}
    virtual void on_load_game_request_post(unsigned int rnd_seed) {}
    virtual void on_screenshot_request(std::string& filepath) {}
  };

}
