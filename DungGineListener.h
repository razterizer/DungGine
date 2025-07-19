//
//  DungGineListener.h
//  DungGine
//
//  Created by Rasmus Anthin on 2024-08-17.
//

#pragma once
#include <Core/events/IListener.h>

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
  };

}
