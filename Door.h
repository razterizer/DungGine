//
//  Door.h
//  DungGine
//
//  Created by Rasmus Anthin on 2024-06-25.
//

#pragma once
#include "SaveGame.h"
#include <Termin8or/RC.h>


namespace dung
{

  struct Corridor;
  struct BSPNode;
  

  struct Door
  {
    RC pos; // world pos
    
    bool is_door = false;
    bool is_open = false;
    bool is_locked = false;
    int key_id = 0;
    
    BSPNode* room = nullptr;
    Corridor* corridor = nullptr;
    
    bool fog_of_war = true;
    bool light = false;
    
    bool open_or_no_door() const
    {
      return !is_door || is_open;
    };
    
    void serialize(std::vector<std::string>& lines) const
    {
      sg::write_var(lines, SG_WRITE_VAR(is_open));
      sg::write_var(lines, SG_WRITE_VAR(is_locked));
      sg::write_var(lines, SG_WRITE_VAR(fog_of_war));
      sg::write_var(lines, SG_WRITE_VAR(light));
    }
    
    std::vector<std::string>::iterator deserialize(std::vector<std::string>::iterator it_line_begin,
                                                   std::vector<std::string>::iterator it_line_end)
    {
      for (auto it_line = it_line_begin; it_line != it_line_end; ++it_line)
      {
        if (sg::read_var(&it_line, SG_READ_VAR(is_open))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(is_locked))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(fog_of_war))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(light)))
        {
          return it_line;
        }
      }
      
      return it_line_end;
    }
  };

}
