//
//  Environment.h
//  DungGine
//
//  Created by Rasmus Anthin on 2024-08-12.
//

#pragma once
#include "BSPTree.h"
#include "Dungeon.h"
#include "RoomStyle.h"
#include "Terrain.h"
#include "ScreenHelper.h"
#include "Comparison.h"
#include <Termin8or/ScreenHandler.h>
#include <optional>


namespace dung
{

  struct DungGineTextureParams
  {
    double dt_anim_s = 0.1;
    std::vector<std::string> texture_file_names_surface_level_fill;
    std::vector<std::string> texture_file_names_surface_level_shadow;
    std::vector<std::string> texture_file_names_underground_fill;
    std::vector<std::string> texture_file_names_underground_shadow;
  };

  class Environment final
  {
    const Dungeon* m_dungeon = nullptr;
    //std::vector<BSPNode*> m_leaves;
    
    // One map per floor.
    std::vector<std::map<BSPNode*, RoomStyle, PtrLess<BSPNode>>> m_room_styles;
    std::vector<std::map<Corridor*, RoomStyle, PtrLess<Corridor>>> m_corridor_styles;
    
    double dt_texture_anim_s = 0.1;
    double texture_anim_time_stamp = 0.;
    unsigned short texture_anim_ctr = 0;
    std::vector<drawing::Texture> texture_sl_fill;
    std::vector<drawing::Texture> texture_sl_shadow;
    std::vector<drawing::Texture> texture_ug_fill;
    std::vector<drawing::Texture> texture_ug_shadow;
    drawing::Texture texture_empty;
    
    
  public:
    Environment() = default;
    ~Environment() = default;
    
    void load_textures(const std::string& exe_folder, DungGineTextureParams texture_params)
    {
      for (const auto& fn : texture_params.texture_file_names_surface_level_fill)
        texture_sl_fill.emplace_back().load(folder::join_path({ exe_folder, fn }));
      for (const auto& fn : texture_params.texture_file_names_surface_level_shadow)
        texture_sl_shadow.emplace_back().load(folder::join_path({ exe_folder, fn }));
      for (const auto& fn : texture_params.texture_file_names_underground_fill)
        texture_ug_fill.emplace_back().load(folder::join_path({ exe_folder, fn }));
      for (const auto& fn : texture_params.texture_file_names_underground_shadow)
        texture_ug_shadow.emplace_back().load(folder::join_path({ exe_folder, fn }));
        
      dt_texture_anim_s = texture_params.dt_anim_s;
    }
    
    void load_dungeon(const Dungeon& dungeon)
    {
      m_dungeon = &dungeon;
      m_room_styles.clear();
      m_corridor_styles.clear();
    }
    
    void style_dungeon(Latitude latitude_0, Longitude longitude_0)
    {
      bool first_floor_is_surface_level = m_dungeon->is_first_floor_is_surface_level();
      for (int f_idx = 0; f_idx < m_dungeon->num_floors(); ++f_idx)
      {
        auto* bsp_tree = m_dungeon->get_tree(f_idx);
        auto world_size = bsp_tree->get_world_size();
        // Default lat_offs = 0 @ Latitude::Equator.
        auto lat_offs = static_cast<int>(latitude_0) - static_cast<int>(Latitude::Equator);
        // Default long_offs = 0 @ Longitude::F.
        auto long_offs = static_cast<int>(longitude_0);
        
        auto f_calc_lat_long = [&world_size, lat_offs, long_offs](auto& room_style, const ttl::Rectangle& bb)
        {
          const auto num_lat = static_cast<int>(Latitude::NUM_ITEMS);
          const auto num_long = static_cast<int>(Longitude::NUM_ITEMS);
          RC cp { bb.r + bb.r_len/2, bb.c + bb.c_len };
          auto lat_idx = math::clamp(static_cast<int>(num_lat*cp.r/world_size.r), 0, num_lat - 1);
          auto long_idx = math::clamp(static_cast<int>(num_long*cp.c/world_size.c), 0, num_long - 1);
          room_style.latitude = static_cast<Latitude>((lat_offs + lat_idx) % num_lat);
          room_style.longitude = static_cast<Longitude>((long_offs + long_idx) % num_long);
        };
        
        const auto* room_vec = m_dungeon->get_rooms(bsp_tree);
        if (room_vec == nullptr)
          std::cerr << "ERROR in Environment::style_dungeon() : Unable to find rooms for the BSP tree of floor " + std::to_string(f_idx) + "!\n";
        else
        {
          for (auto* leaf : *room_vec)
          {
            RoomStyle room_style;
            room_style.init_rand(f_idx == 0, first_floor_is_surface_level);
          
            const auto& fill_textures = room_style.is_underground ? texture_ug_fill : texture_sl_fill;
            if (!fill_textures.empty())
            {
              // #NOTE: Here we assume all textures in the animation batch are of the same size.
              const auto& tex = fill_textures.front();
              if (tex.size.r >= leaf->bb_leaf_room.r_len - 1 && tex.size.c >= leaf->bb_leaf_room.c_len - 1)
              {
                room_style.tex_pos.r = rnd::rand_int(0, tex.size.r - leaf->bb_leaf_room.r_len);
                room_style.tex_pos.c = rnd::rand_int(0, tex.size.c - leaf->bb_leaf_room.c_len);
              }
            }
          
            f_calc_lat_long(room_style, leaf->bb_leaf_room);
          
            stlutils::at_growing(m_room_styles, f_idx)[leaf] = room_style;
          }
        }
        
        const auto& room_corridor_map = bsp_tree->get_room_corridor_map();
        for (const auto& cp : room_corridor_map)
        {
          RoomStyle room_style;
          room_style.is_underground = is_underground(f_idx, cp.first.first) || is_underground(f_idx, cp.first.second);
          room_style.wall_type = WallType::Masonry4;
          room_style.wall_style = { Color::LightGray, Color::Black }; //wall_palette[WallBasicType::Masonry]
          room_style.floor_type = FloorType::Stone2;
          
          f_calc_lat_long(room_style, cp.second->bb);
          
          stlutils::at_growing(m_corridor_styles, f_idx)[cp.second] = room_style;
        }
      }
    }
    
    const Dungeon* get_dungeon() const
    {
      return m_dungeon;
    }
    
    int num_floors() const
    {
      return m_dungeon->num_floors();
    }
    
    int get_init_floor() const
    {
      return m_dungeon->get_init_floor();
    }
    
    int rand_floor() const
    {
      return rnd::rand_int(0, m_dungeon->num_floors() - 1);
    }
    
    RC get_world_size(int floor) const
    {
      auto* bsp_tree = m_dungeon->get_tree(floor);
      return bsp_tree->get_world_size();
    }
    
    std::map<std::pair<BSPNode*, BSPNode*>, Corridor*, PtrPairLess<BSPNode>> get_room_corridor_map(int floor) const
    {
      auto* bsp_tree = m_dungeon->get_tree(floor);
      return bsp_tree->get_room_corridor_map();
    }
    
    std::vector<Door*> fetch_doors(int floor) const
    {
      auto* bsp_tree = m_dungeon->get_tree(floor);
      return bsp_tree->fetch_doors();
    }
    
    std::vector<Staircase*> fetch_staircases(int floor) const
    {
      return m_dungeon->fetch_staircases(floor);
    }
    
    // #NOTE: Only for unwalled area!
    bool is_inside_any_room(int floor, const RC& pos, BSPNode** room_node = nullptr) const
    {
      auto* bsp_tree = m_dungeon->get_tree(floor);
      return is_inside_any_room(bsp_tree, pos, room_node);
    }
    
    // #NOTE: Only for unwalled area!
    bool is_inside_any_room(BSPTree* bsp_tree, const RC& pos, BSPNode** room_node = nullptr) const
    {
      const auto* room_vec = m_dungeon->get_rooms(bsp_tree);
      if (room_vec == nullptr)
        return false;
      for (auto* leaf : *room_vec)
        if (leaf->bb_leaf_room.is_inside_offs(pos, -1))
        {
          utils::try_set(room_node, leaf);
          return true;
        }
      return false;
    }
    
    bool is_underground(int floor, BSPNode* room) const
    {
      if (!stlutils::in_range(m_room_styles, floor))
        return false;
      auto it = m_room_styles[floor].find(room);
      if (it != m_room_styles[floor].end())
        return it->second.is_underground;
      return false;
    }
    
    bool is_underground(int floor, Corridor* corr) const
    {
      if (!stlutils::in_range(m_corridor_styles, floor))
        return false;
      auto it = m_corridor_styles[floor].find(corr);
      if (it != m_corridor_styles[floor].end())
        return it->second.is_underground;
      return false;
    }
    
    std::optional<RoomStyle> find_room_style(int floor, BSPNode* room) const
    {
      if (!stlutils::in_range(m_room_styles, floor))
        return std::nullopt;
      auto itr = m_room_styles[floor].find(room);
      if (itr != m_room_styles[floor].end())
        return itr->second;
      return std::nullopt;
    }
    
    std::optional<RoomStyle> find_corridor_style(int floor, Corridor* corridor) const
    {
      if (!stlutils::in_range(m_corridor_styles, floor))
        return std::nullopt;
      auto itc = m_corridor_styles[floor].find(corridor);
      if (itc != m_corridor_styles[floor].end())
        return itc->second;
      return std::nullopt;
    }
    
    std::optional<const drawing::Texture*> fetch_texture(const auto& texture_vector) const
    {
      if (texture_vector.empty())
        return std::nullopt; //texture_empty;
      return &texture_vector[texture_anim_ctr % texture_vector.size()];
    };
    
    std::optional<const drawing::Texture*> fetch_curr_fill_texture(const RoomStyle& room_style) const
    {
      auto texture_fill = room_style.is_underground ?
        fetch_texture(texture_ug_fill) : fetch_texture(texture_sl_fill);
      return texture_fill;
    }
    
    std::optional<const drawing::Texture*> fetch_curr_shadow_texture(const RoomStyle& room_style) const
    {
      auto texture_shadow = room_style.is_underground ?
      fetch_texture(texture_ug_shadow) : fetch_texture(texture_sl_shadow);
      return texture_shadow;
    }
    
    Terrain get_terrain(int floor, const RC& pos) const
    {
      auto* bsp_tree = m_dungeon->get_tree(floor);
      if (bsp_tree == nullptr)
        return Terrain::Default;
      BSPNode* room = nullptr;
      if (!is_inside_any_room(bsp_tree, pos, &room))
        return Terrain::Default;
      const auto& bb = room->bb_leaf_room;
      if (bb.is_inside_offs(pos, -1))
      {
        auto its = m_room_styles[floor].find(room);
        if (its == m_room_styles[floor].end())
          return Terrain::Default;
        const auto& room_style = its->second;
        auto local_pos = pos - bb.pos() - RC { 1, 1 };
        auto tex_pos = room_style.tex_pos + local_pos;
        auto texture = fetch_curr_fill_texture(room_style);
        if (texture.has_value())
        {
          int curr_mat = (*texture.value())(tex_pos).mat;
          // #FIXME: Canonize material idcs.
          switch (curr_mat)
          {
            case 0: return Terrain::Void;
            case 1: return Terrain::Tile;
            case 2: return Terrain::Water;
            case 3: return Terrain::Sand;
            case 4: return Terrain::Stone;
            case 5: return Terrain::Masonry;
            case 6: return Terrain::Brick;
            case 7: return Terrain::Grass;
            case 8: return Terrain::Shrub;
            case 9: return Terrain::Tree;
            case 10: return Terrain::Metal;
            case 11: return Terrain::Wood;
            case 12: return Terrain::Ice;
            case 13: return Terrain::Mountain;
            case 14: return Terrain::Lava;
            case 15: return Terrain::Cave;
            case 16: return Terrain::Swamp;
            case 17: return Terrain::Poison;
            case 18: return Terrain::Path;
            case 19: return Terrain::Mine;
            case 20: return Terrain::Gold;
            case 21: return Terrain::Silver;
            case 22: return Terrain::Gravel;
            case 23: return Terrain::Bone;
            case 24: return Terrain::Acid;
            case 25: return Terrain::Column;
            case 26: return Terrain::Tar;
            case 27: return Terrain::Rope;
            default: return Terrain::Default;
          }
        }
        else
        {
          switch (room_style.floor_type)
          {
            case FloorType::None: return Terrain::Default;
            case FloorType::Sand: return Terrain::Sand;
            case FloorType::Grass: return Terrain::Grass;
            case FloorType::Stone: return Terrain::Stone;
            case FloorType::Stone2: return Terrain::Stone;
            case FloorType::Water: return Terrain::Water;
            case FloorType::Wood: return Terrain::Wood;
            default: return Terrain::Default;
          }
        }
      }
      return Terrain::Default;
    }
    
    Terrain get_terrain(int floor, int r, int c) const
    {
      return get_terrain(floor, RC { r, c });
    }
    
    bool allow_move_to(int floor, int r, int c) const
    {
      auto terrain = get_terrain(floor, RC { r, c });
    
      return dung::allow_move_to(terrain);
    }
    
    template<int NR, int NC>
    void draw_environment(ScreenHandler<NR, NC>& sh, double real_time_s,
                          int curr_floor, bool use_fog_of_war,
                          SolarDirection sun_dir, SolarMotionPatterns& solar_motion,
                          float t_solar_period, Season season,
                          bool use_per_room_lat_long_for_sun_dir,
                          ScreenHelper* screen_helper,
                          bool debug)
    {
      auto shadow_type = sun_dir;
      for (const auto& room_pair : m_room_styles[curr_floor])
      {
        auto* room = room_pair.first;
        const auto& bb = room->bb_leaf_room;
        const auto& room_style = room_pair.second;
        auto bb_scr_pos = screen_helper->get_screen_pos(bb.pos());
        if (use_per_room_lat_long_for_sun_dir)
          shadow_type = solar_motion.get_solar_direction(room_style.latitude, room_style.longitude, season, t_solar_period);
        
        if (debug)
        {
          sh.write_buffer(std::to_string(room_style.is_underground), bb_scr_pos.r + 1, bb_scr_pos.c + 1, Color::White, Color::Black);
        }
        
        // Fog of war
        if (use_fog_of_war)
        {
          for (int r = 0; r < bb.r_len; ++r)
          {
            for (int c = 0; c < bb.c_len; ++c)
            {
              if (room->fog_of_war[r * bb.c_len + c])
                sh.write_buffer(".", bb_scr_pos.r + r, bb_scr_pos.c + c, Color::Black, Color::Black);
            }
          }
        }
        
        drawing::draw_box_outline(sh,
                                  bb_scr_pos.r, bb_scr_pos.c, bb.r_len, bb.c_len,
                                  room_style.wall_type,
                                  room_style.wall_style,
                                  room->light);
                                  
        if (room_style.is_underground ? texture_ug_fill.empty() : texture_sl_fill.empty())
        {
          drawing::draw_box(sh,
                            bb_scr_pos.r, bb_scr_pos.c, bb.r_len, bb.c_len,
                            room_style.get_fill_style(),
                            room_style.get_fill_char(),
                            room_style.is_underground ? SolarDirection::Nadir : shadow_type,
                            styles::shade_style(room_style.get_fill_style(), color::ShadeType::Dark),
                            room_style.get_fill_char(),
                            room->light);
        }
        else
        {
          if (real_time_s - texture_anim_time_stamp > dt_texture_anim_s)
          {
            texture_anim_ctr++;
            texture_anim_time_stamp = real_time_s;
          }
          
          const auto& texture_fill = *(fetch_curr_fill_texture(room_style).value_or(&texture_empty));
          const auto& texture_shadow = *(fetch_curr_shadow_texture(room_style).value_or(&texture_empty));
        
          drawing::draw_box_textured(sh,
                                     bb_scr_pos.r, bb_scr_pos.c, bb.r_len, bb.c_len,
                                     room_style.is_underground ? SolarDirection::Nadir : shadow_type,
                                     texture_fill,
                                     texture_shadow,
                                     room->light,
                                     room_style.is_underground,
                                     room_style.tex_pos);
        }
      }
      
      shadow_type = sun_dir;
      for (const auto& corr_pair : m_corridor_styles[curr_floor])
      {
        auto* corr = corr_pair.first;
        const auto& bb = corr->bb;
        const auto& corr_style = corr_pair.second;
        auto bb_scr_pos = screen_helper->get_screen_pos(bb.pos());
        if (use_per_room_lat_long_for_sun_dir)
          shadow_type = solar_motion.get_solar_direction(corr_style.latitude, corr_style.longitude, season, t_solar_period);
        
        // Fog of war
        if (use_fog_of_war)
        {
          for (int r = 0; r < bb.r_len; ++r)
          {
            for (int c = 0; c < bb.c_len; ++c)
            {
              if (corr->fog_of_war[r * bb.c_len + c])
                sh.write_buffer(".", bb_scr_pos.r + r, bb_scr_pos.c + c, Color::Black, Color::Black);
            }
          }
        }
        
        
        drawing::draw_box_outline(sh,
                                  bb_scr_pos.r, bb_scr_pos.c, bb.r_len, bb.c_len,
                                  corr_style.wall_type,
                                  corr_style.wall_style,
                                  corr->light);
        drawing::draw_box(sh,
                          bb_scr_pos.r, bb_scr_pos.c, bb.r_len, bb.c_len,
                          corr_style.get_fill_style(),
                          corr_style.get_fill_char(),
                          corr_style.is_underground ? SolarDirection::Nadir : shadow_type,
                          styles::shade_style(corr_style.get_fill_style(), color::ShadeType::Dark, true),
                          corr_style.get_fill_char(),
                          corr->light);
      }
    }
    
    BSPNode* find_room(int floor, int id) const
    {
      auto* bsp_tree = m_dungeon->get_tree(floor);
      const auto* room_vec = m_dungeon->get_rooms(bsp_tree);
      if (room_vec == nullptr)
        return nullptr;
      for (auto* leaf : *room_vec)
        if (leaf->id == id)
          return leaf;
      return nullptr;
    }
    
    Corridor* find_corridor(int floor, int id) const
    {
      auto* bsp_tree = m_dungeon->get_tree(floor);
      const auto& room_corridor_map = bsp_tree->get_room_corridor_map();
      for (const auto& cp : room_corridor_map)
        if (cp.second->id == id)
          return cp.second;
      return nullptr;
    }
    
    void serialize(std::vector<std::string>& lines) const
    {
      for (auto* bsp_tree : m_dungeon->get_trees())
        bsp_tree->serialize(lines);
    }
    
    std::vector<std::string>::iterator deserialize(std::vector<std::string>::iterator it_line_begin,
                                                   std::vector<std::string>::iterator it_line_end)
    {
      // #FIXME: Reimplement.
      return it_line_begin;
      //return m_bsp_tree->deserialize(it_line_begin, it_line_end);
    }
  };

}
