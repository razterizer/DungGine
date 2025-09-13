//
//  DungObject.h
//  DungGine
//
//  Created by Rasmus Anthin on 2024-09-15.
//

#pragma once
#include "SaveGame.h"
#include "Environment.h"

struct BSPNode;
struct Corridor;


namespace dung
{

  struct DungObject
  {
    virtual ~DungObject() = default;
  
    RC pos; // world pos
    bool fog_of_war = true;
    bool light = false;
    bool visible = false;
    bool is_underground = false;
    
    bool shade = false; // Shade with room/corridor light.
    
    int curr_floor = 0;
    BSPNode* curr_room = nullptr;
    Corridor* curr_corridor = nullptr;
    
    virtual void serialize(std::vector<std::string>& lines) const
    {
      sg::write_var(lines, SG_WRITE_VAR(pos));
      sg::write_var(lines, SG_WRITE_VAR(fog_of_war));
      sg::write_var(lines, SG_WRITE_VAR(light));
      sg::write_var(lines, SG_WRITE_VAR(visible));
      sg::write_var(lines, SG_WRITE_VAR(is_underground));
      sg::write_var(lines, SG_WRITE_VAR(curr_floor));
      lines.emplace_back("curr_room:id");
      lines.emplace_back(std::to_string(curr_room != nullptr ? curr_room->id : -1));
      lines.emplace_back("curr_corridor:id");
      lines.emplace_back(std::to_string(curr_corridor != nullptr ? curr_corridor->id : -1));
    }
    
    virtual std::vector<std::string>::iterator deserialize(std::vector<std::string>::iterator it_line_begin,
                                                           std::vector<std::string>::iterator it_line_end,
                                                           Environment* environment)
    {
      for (auto it_line = it_line_begin; it_line != it_line_end; ++it_line)
      {
        if (sg::read_var(&it_line, SG_READ_VAR(pos))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(fog_of_war))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(light))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(visible))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(is_underground))) {}
        else if (sg::read_var(&it_line, SG_READ_VAR(curr_floor))) {}
        else if (*it_line == "curr_room:id")
        {
          ++it_line;
          auto room_id = std::atoi(it_line->c_str());
          auto* room = environment->find_room(curr_floor, room_id);
          if (room != nullptr)
            curr_room = room;
          if (room_id != -1 && room == nullptr)
            std::cerr << "Error in DungObject when parsing curr_room:id = \"" << room_id << "\"!\n";
        }
        else if (*it_line == "curr_corridor:id")
        {
          ++it_line;
          auto corr_id = std::atoi(it_line->c_str());
          auto* corr = environment->find_corridor(curr_floor, corr_id);
          if (corr != nullptr)
            curr_corridor = corr;
          if (corr_id != -1 && corr == nullptr)
            std::cerr << "Error in DungObject when parsing curr_corridor:id = \"" << corr_id << "\"!\n";
          return it_line;
        }
      }
    
      return it_line_end;
    }
  };

}
