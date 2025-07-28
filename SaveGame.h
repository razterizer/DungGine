//
//  GameSave.h
//  DungGine
//
//  Created by Rasmus Anthin on 2025-07-27.
//

#pragma once

#include <Core/StringHelper.h>
#include <string>
#include <strstream>

namespace sg
{
  
  bool read_var(const std::string& curr_line, const std::string& var_name, auto* var_ptr)
  {
    if (var_ptr == nullptr || !curr_line.starts_with(var_name))
      return false;
    
    auto tokens = str::tokenize(curr_line, { '=', ' ' });
    if (tokens.size() != 2)
      return false;
    
    std::istringstream iss(tokens[1]);
    iss >> *var_ptr;
    return true;
  };
  
  void write_var(std::vector<std::string>& lines_vec, const std::string& var_name, const auto& var)
  {
    lines_vec.emplace_back(var_name + " = " + std::to_string(var));
  };

}
