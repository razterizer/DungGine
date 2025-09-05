//
//  GameSave.h
//  DungGine
//
//  Created by Rasmus Anthin on 2025-07-27.
//

#pragma once

#include "Terrain.h"
#include <Core/StringHelper.h>
#include <Termin8or/StringConversion.h>
#include <Termin8or/Styles.h>
#include <Termin8or/Color.h>
#include <string>
#include <sstream>

#define SG_READ_VAR(var) #var, &var
#define SG_WRITE_VAR(var) #var, var

namespace sg
{
  
  template<typename T>
  bool read_var(std::vector<std::string>::iterator* it_line, const std::string& var_name, T* var_ptr)
  {
    auto curr_line = **it_line;
    if (var_ptr == nullptr || !curr_line.starts_with(var_name))
      return false;
    
    auto tokens = str::tokenize(curr_line, { '=', ' ' });
    if (tokens.size() != 2)
      return false;
    
    std::istringstream iss(tokens[1]);
    iss >> *var_ptr;
    return true;
  };
  
  template<>
  bool read_var(std::vector<std::string>::iterator* it_line, const std::string& var_name, std::string* var_ptr)
  {
    auto curr_line = **it_line;
    if (var_ptr == nullptr || !curr_line.starts_with(var_name))
      return false;
    
    auto tokens = str::tokenize(curr_line, { '=', ' ' }, { '\"' });
    if (tokens.size() != 2)
      return false;
    
    std::istringstream iss(tokens[1]);
    iss >> *var_ptr;
    return true;
  };
  
  template<>
  bool read_var(std::vector<std::string>::iterator* it_line, const std::string& var_name, t8::RC* var_ptr)
  {
    if (**it_line == var_name)
    {
      auto ret = t8::str::str_to_rc(*(*it_line + 1));
      if (ret.has_value())
      {
        *var_ptr = ret.value();
        ++(*it_line);
      }
      return true;
    }
    return false;
  }
  
  template<>
  bool read_var(std::vector<std::string>::iterator* it_line, const std::string& var_name, t8::color::Style* var_ptr)
  {
    if (**it_line == var_name)
    {
      ++(*it_line);
      var_ptr->fg_color = t8::color::string2color(**it_line);
      ++(*it_line);
      var_ptr->bg_color = t8::color::string2color(**it_line);
      return true;
    }
    return false;
  }
  
  template<>
  bool read_var(std::vector<std::string>::iterator* it_line, const std::string& var_name, dung::Terrain* var_ptr)
  {
    if (**it_line == var_name)
    {
      auto ret = dung::str2terrain(*(*it_line + 1));
      if (ret.has_value())
      {
        *var_ptr = ret.value();
        ++(*it_line);
      }
      return true;
    }
    return false;
  }
  
  template<typename T>
  bool read_var(std::vector<std::string>::iterator* it_line, const std::string& var_name, std::vector<T>* var_ptr)
  {
    if (**it_line == var_name)
    {
      auto tokens = str::tokenize(*(*it_line + 1), { ',' });
      if (!tokens.empty())
      {
        size_t len = tokens.size();
        var_ptr->resize(len);
        for (size_t i = 0; i < len; ++i)
          var_ptr->operator[](i) = static_cast<T>(std::stod(tokens[i]));
        ++(*it_line);
      }
      return true;
    }
    return false;
  }
  
  template<typename EnumClass>
  bool read_var_enum(std::vector<std::string>::iterator* it_line, const std::string& var_name, EnumClass* var_ptr)
  {
    auto curr_line = **it_line;
    if (var_ptr == nullptr || !curr_line.starts_with(var_name))
      return false;
    
    auto tokens = str::tokenize(curr_line, { '=', ' ' });
    if (tokens.size() != 2)
      return false;
    
    int enum_val = 0;
    std::istringstream iss(tokens[1]);
    iss >> enum_val;
    *var_ptr = static_cast<EnumClass>(enum_val); // Not easy to check if enum_val corresponds to a valid enum item.
    return true;
  }
  
  template<typename T>
  void write_var(std::vector<std::string>& lines_vec, const std::string& var_name, const T& var)
  {
    lines_vec.emplace_back(var_name + " = " + std::to_string(var));
  };
  
  template<>
  void write_var(std::vector<std::string>& lines_vec, const std::string& var_name, const std::string& var)
  {
    lines_vec.emplace_back(var_name + " = \"" + var + "\"");
  };
  
  template<>
  void write_var(std::vector<std::string>& lines_vec, const std::string& var_name, const char& var)
  {
    lines_vec.emplace_back(var_name + " = " + std::string(1, var));
  };
  
  template<>
  void write_var(std::vector<std::string>& lines_vec, const std::string& var_name, const t8::RC& var)
  {
    lines_vec.emplace_back(var_name);
    lines_vec.emplace_back(t8::str::rc_to_str(var));
  };
  
  template<>
  void write_var(std::vector<std::string>& lines_vec, const std::string& var_name, const t8::color::Style& var)
  {
    lines_vec.emplace_back(var_name);
    lines_vec.emplace_back(t8::color::color2string(var.fg_color));
    lines_vec.emplace_back(t8::color::color2string(var.bg_color));
  };
  
  template<>
  void write_var(std::vector<std::string>& lines_vec, const std::string& var_name, const dung::Terrain& var)
  {
    lines_vec.emplace_back(var_name);
    lines_vec.emplace_back(dung::terrain2str(var));
  }
  
  template<typename T>
  void write_var(std::vector<std::string>& lines_vec, const std::string& var_name, const std::vector<T>& var)
  {
    lines_vec.emplace_back(var_name);
    auto str_vec = str::to_string_vector(var);
    auto str_flat = str::flatten(str_vec, ",");
    lines_vec.emplace_back(str_flat);
  }
  
  template<typename EnumClass>
  void write_var_enum(std::vector<std::string>& lines_vec, const std::string& var_name, const EnumClass& var)
  {
    lines_vec.emplace_back(var_name + " = " + std::to_string(static_cast<int>(var)));
  }

}
