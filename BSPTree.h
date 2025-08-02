//
//  BSPTree.h
//  DungGine
//
//  Created by Rasmus Anthin on 2024-06-08.
//

#pragma once
#include "Door.h"
#include "Corridor.h"
#include "Comparison.h"
#include <Termin8or/RC.h>
#include <Termin8or/ScreenHandler.h>
#include <Termin8or/Drawing.h>
#include <Core/Rand.h>
#include <Core/Math.h>
#include <Core/bool_vector.h>
#include <Core/Utils.h>
#include <array>
#include <memory>


namespace dung
{
  
  // Example:
  // ####################################################
  // #                                                  #
  // #                                                  #
  // #                                                  #
  // #                                                  #
  // #                                                  #
  // #                     Root                         #
  // #                                                  #
  // #                                                  #
  // #                                                  #
  // #                                                  #
  // #                                                  #
  // #                                                  #
  // ####################################################
  //
  // ####################################################
  // #                   |                              #
  // #                   |                              #
  // #                   | Vertical                     #
  // #                   | Wall / Split                 #
  // #                   |                              #
  // #                   |                              #
  // #     Child L       |           Child R            #
  // #                   |                              #
  // #                   |                              #
  // #                   |                              #
  // #                   |                              #
  // #                   |                              #
  // ####################################################
  // |-------------------*----------------------------->|
  //              fraction = 20 / 50 = 0.4
  //
  // ####################################################--
  // #                                                  # |
  // #                                                  # |
  // #                                                  # |
  // #                   Child U                        # |
  // #                                                  # |
  // #                                                  # |
  // #                                                  # |
  // #  Horizontal Wall / Split                         # |
  // #--------------------------------------------------# * fraction = 9 / 12
  // #                                                  # |          = 0.75
  // #                   Child D                        # |
  // #                                                  # v
  // ####################################################--
  //
  // So fractional lengths are calculated from left to right (column-wise)
  //   or up to down (row-wise).
  
  // //////////////////////////////////////////////////////////////

  int global_bsp_node_id = 0;

  struct BSPNode final
  {
    int id = global_bsp_node_id++;
  
    // The current room will be split into two rooms along this direction.
    Orientation orientation = Orientation::Vertical;
    // Length fraction of current room along the direction of the other orientation.
    // If orientation = Vertical then the fraction will be along the horizontal
    //   axis.
    float split_fraction = 0.5f;
    int size_rows = 0;
    int size_cols = 0;
    
    ttl::Rectangle bb_region;
    ttl::Rectangle bb_leaf_room;
    
    std::array<std::unique_ptr<BSPNode>, 2> children;
        
    int level = 0; // root = 0;
    
    std::vector<Door*> doors;
    
    bool_vector fog_of_war;
    bool_vector light;
    
    // ///////////
    
    bool is_leaf() const { return !children[0] && !children[1]; }
    
    void generate(ttl::Rectangle bb, int lvl, int min_room_length)
    {
      bb_region = bb;
      level = lvl;
      split_fraction = rnd::rand();
      int split_length_0 = 0;
      int split_length_1 = 0;
      switch (orientation)
      {
        case Orientation::Vertical:
          split_length_0 = math::roundI(size_cols * split_fraction);
          split_length_1 = size_cols - split_length_0;
          break;
        case Orientation::Horizontal:
          split_length_0 = math::roundI(size_rows * split_fraction);
          split_length_1 = size_rows - split_length_0;
          break;
      };
      
      auto f_set_ch_size = [&](BSPNode* ch, int split_len)
      {
        switch (orientation)
        {
          case Orientation::Vertical:
            ch->size_cols = split_len;
            ch->size_rows = size_rows;
            break;
          case Orientation::Horizontal:
            ch->size_rows = split_len;
            ch->size_cols = size_cols;
            break;
        }
      };
    
      auto child_orientation = static_cast<Orientation>(1 - static_cast<int>(orientation));
      if (split_length_0 >= min_room_length && split_length_1 >= min_room_length)
      {
        auto& ch_0 = children[0] = std::make_unique<BSPNode>();
        ch_0->orientation = child_orientation;
        f_set_ch_size(ch_0.get(), split_length_0);
        int ch0_r_len = children[0]->size_rows;
        int ch0_c_len = children[0]->size_cols;
        ttl::Rectangle bb_0 { bb.r, bb.c, ch0_r_len, ch0_c_len };
        ch_0->generate(bb_0, lvl + 1, min_room_length);
        
        auto& ch_1 = children[1] = std::make_unique<BSPNode>();
        ch_1->orientation = child_orientation;
        f_set_ch_size(ch_1.get(), split_length_1);
        int ch1_r = 0;
        int ch1_c = 0;
        switch (orientation)
        {
          case Orientation::Vertical:
            ch1_r = bb.r;
            ch1_c = bb.c + ch0_c_len;
            break;
          case Orientation::Horizontal:
            ch1_r = bb.r + ch0_r_len;
            ch1_c = bb.c;
            break;
        }
        int ch1_r_len = children[1]->size_rows;
        int ch1_c_len = children[1]->size_cols;
        ttl::Rectangle bb_1 { ch1_r, ch1_c, ch1_r_len, ch1_c_len };
        ch_1->generate(bb_1, lvl + 1, min_room_length);
      }
    }
    
    template<int NR, int NC>
    void draw_regions(ScreenHandler<NR, NC>& sh,
                      int r0, int c0,
                      const styles::Style& border_style) const
    {
      drawing::draw_box_outline(sh, r0 + bb_region.r, c0 + bb_region.c, bb_region.r_len, bb_region.c_len, drawing::OutlineType::Hash, border_style);
      
      if (children[0])
        children[0]->draw_regions(sh, r0, c0, border_style);
      if (children[1])
        children[1]->draw_regions(sh, r0, c0, border_style);
    }
    
    template<int NR, int NC>
    void draw_rooms(ScreenHandler<NR, NC>& sh,
                    int r0, int c0,
                    const styles::Style& room_style) const
    {
      if (!bb_leaf_room.is_empty())
      {
        drawing::draw_box_outline(sh,
                 r0 + bb_leaf_room.r, c0 + bb_leaf_room.c, bb_leaf_room.r_len, bb_leaf_room.c_len,
                 drawing::OutlineType::Hash, room_style);
      }
                 
      if (children[0])
        children[0]->draw_rooms(sh, r0, c0, room_style);
      if (children[1])
        children[1]->draw_rooms(sh, r0, c0, room_style);
    }
    
    void print_tree(const std::string& indent = "") const
    {
      std::cout << indent << "Id: " << id << std::endl;
      std::cout << indent << "Level: " << level << std::endl;
      std::cout << indent << "Orientation: " << (orientation == Orientation::Vertical ? "V" : "H") << std::endl;
      std::cout << indent << "Size: [" << size_rows << ", " << size_cols << "]" << std::endl;
      
      auto child_indent = indent + str::rep_char(' ', 2);
      
      if (children[0])
      {
        std::cout << indent << "Child 0:" << std::endl;
        children[0]->print_tree(child_indent);
      }
      if (children[1])
      {
        std::cout << indent << "Child 1:" << std::endl;
        children[1]->print_tree(child_indent);
      }
    }
    
    void pad_rooms(int min_room_length, int min_rnd_wall_padding, int max_rnd_wall_padding)
    {
      if (is_leaf())
      {
        std::array<int, 4> padding_nswe { 0, 0, 0, 0 }; // top, bottom, left, right
        int num_tries = 0;
        do
        {
          for (int i = 0; i < 4; ++i)
            padding_nswe[i] = rnd::rand_int(min_rnd_wall_padding, max_rnd_wall_padding);
          bb_leaf_room = bb_region;
          bb_leaf_room.r += padding_nswe[0];
          bb_leaf_room.r_len -= padding_nswe[0] + padding_nswe[1];
          bb_leaf_room.c += padding_nswe[2];
          bb_leaf_room.c_len -= padding_nswe[2] + padding_nswe[3];
          if (num_tries > 20)
            min_rnd_wall_padding = 0;
          num_tries++;
        } while (bb_leaf_room.r_len < min_room_length || bb_leaf_room.c_len < min_room_length);
        
        size_t surf_area = bb_leaf_room.r_len * bb_leaf_room.c_len;
        fog_of_war.resize(surf_area, true);
        light.resize(surf_area, false);
      }
      else
      {
        if (children[0])
          children[0]->pad_rooms(min_room_length, min_rnd_wall_padding, max_rnd_wall_padding);
        if (children[1])
          children[1]->pad_rooms(min_room_length, min_rnd_wall_padding, max_rnd_wall_padding);
      }
    }
    
    void collect_leaves(std::vector<BSPNode*>& leaves)
    {
      if (is_leaf())
        leaves.emplace_back(this);
      if (children[0])
        children[0]->collect_leaves(leaves);
      if (children[1])
        children[1]->collect_leaves(leaves);
    }
    
    bool is_inside_room(const RC& pos, ttl::BBLocation* location = nullptr) const
    {
      if (!is_leaf())
        return false;
      for (auto* d : doors)
      {
        if (d->open_or_no_door() && pos == d->pos)
        {
          utils::try_set(location, ttl::BBLocation::Inside);
          return true;
        }
      }
      utils::try_set(location, bb_leaf_room.find_location_offs(pos, -1, -1, -1, -1));
      return bb_leaf_room.is_inside_offs(pos, -1);
    }
    
    bool is_in_fog_of_war(const RC& world_pos)
    {
      if (!is_leaf())
        return true;
      auto local_pos = world_pos - bb_leaf_room.pos();
      auto idx = local_pos.r * bb_leaf_room.c_len + local_pos.c;
      if (math::in_range<int>(idx, 0, stlutils::sizeI(fog_of_war), Range::ClosedOpen))
        return fog_of_war[idx];
      return true;
    }
    
    bool is_in_light(const RC& world_pos)
    {
      if (!is_leaf())
        return false;
      auto local_pos = world_pos - bb_leaf_room.pos();
      auto idx = local_pos.r * bb_leaf_room.c_len + local_pos.c;
      if (math::in_range<int>(idx, 0, stlutils::sizeI(light), Range::ClosedOpen))
        return light[idx];
      return false;
    }
  };
      
  // //////////////////////////////////////////////////////////////

  class BSPTree final
  {
    BSPNode m_root;
    int m_min_room_length = 4;
    
    std::vector<std::unique_ptr<Corridor>> corridors;
    std::vector<std::unique_ptr<Door>> doors;
    std::map<std::pair<BSPNode*, BSPNode*>, Corridor*, PtrPairLess<BSPNode>> room_corridor_map;
    
  public:
    BSPTree() = default;
    BSPTree(int min_room_length)
      : m_min_room_length(min_room_length)
    {}
    
    void reset()
    {
      global_bsp_node_id = 0;
      global_corridor_id = 0;
      m_root = BSPNode {};
      corridors.clear();
      doors.clear();
      room_corridor_map.clear();
    }
    
    void generate(int world_size_rows, int world_size_cols,
                  Orientation first_split_orientation)
    {
      m_root.orientation = first_split_orientation;
      m_root.size_rows = world_size_rows;
      m_root.size_cols = world_size_cols;
      ttl::Rectangle bb { 0, 0, m_root.size_rows, m_root.size_cols };
      m_root.generate(bb, 0, m_min_room_length);
    }
    
    std::vector<BSPNode*> fetch_leaves()
    {
      std::vector<BSPNode*> leaves;
      m_root.collect_leaves(leaves);
      return leaves;
    }
    
    RC get_world_size() const
    {
      return { m_root.size_rows, m_root.size_cols };
    }
    
    void pad_rooms(int min_rnd_wall_padding = 1, int max_rnd_wall_padding = 4)
    {
      m_root.pad_rooms(m_min_room_length, min_rnd_wall_padding, max_rnd_wall_padding);
    }
    
    void create_corridors(int min_corridor_half_width = 1)
    {
      std::vector<BSPNode*> leaves;
      m_root.collect_leaves(leaves);
      
      for (auto* leaf_A : leaves)
      {
        for (auto* leaf_B : leaves)
        {
          if (leaf_A != leaf_B)
          {
          
            auto try_make_horizontal_corridor = [&](auto* leaf_A, auto* leaf_B)
            {
              // Horizontal approach
              auto bb_A = leaf_A->bb_leaf_room;
              auto bb_B = leaf_B->bb_leaf_room;
              if (bb_A.right() > bb_B.left())
                return false;
              
              int c0 = bb_A.right();
              int c1 = bb_B.left();
              int r0 = std::max(bb_A.top(), bb_B.top());
              int r1 = std::min(bb_A.bottom(), bb_B.bottom());
              if (r0 > r1)
                return false;
              if (bb_A.top() < bb_B.top() && bb_A.bottom() - bb_B.top() < 2*min_corridor_half_width)
                return false;
              if (bb_B.top() < bb_A.top() && bb_B.bottom() - bb_A.top() < 2*min_corridor_half_width)
                return false;
              bool collided = false;
              auto collides_with_bb = [r0, r1, c0, c1](const ttl::Rectangle bb) -> bool
              {
                if (c0 <= bb.left() && bb.right() <= c1)
                {
                  if (bb.top() <= r0 && r1 <= bb.bottom())
                    return true;
                  if (r0 <= bb.top() && bb.top() <= r1)
                    return true;
                  if (r0 <= bb.bottom() && bb.bottom() <= r1)
                    return true;
                }
                return false;
              };
              for (auto* leaf_C : leaves)
              {
                auto bb_C = leaf_C->bb_leaf_room;
                collided = collides_with_bb(bb_C);
                if (collided)
                  break;
              }
              if (!collided)
              {
                for (const auto& cp : room_corridor_map)
                {
                  auto bb_corr = cp.second->bb;
                  collided = collides_with_bb(bb_corr);
                  if (collided)
                    break;
                }
              }
              if (!collided)
              {
                auto* min_leaf = stlutils::select_if(leaf_A, leaf_B, [](auto* a, auto* b) { return a->id < b->id; });
                auto* max_leaf = stlutils::select_if(leaf_A, leaf_B, [](auto* a, auto* b) { return a->id > b->id; });
                auto key = std::pair { min_leaf, max_leaf };
                auto it = room_corridor_map.find(key);
                if (it == room_corridor_map.end())
                {
                  auto* corr = corridors.emplace_back(std::make_unique<Corridor>()).get();
                  corr->bb = { (r0 + r1)/2 - min_corridor_half_width, c0, 2*min_corridor_half_width + 1, c1 - c0 + 1 };
                  corr->orientation = Orientation::Horizontal;
                  size_t surf_area = corr->bb.r_len * corr->bb.c_len;
                  corr->fog_of_war.resize(surf_area, true);
                  corr->light.resize(surf_area, false);
                  room_corridor_map[key] = corr;
                  return true;
                }
              }
              return false;
            };
            
            auto try_make_vertical_corridor = [&](auto* leaf_A, auto* leaf_B)
            {
              // Horizontal approach
              auto bb_A = leaf_A->bb_leaf_room;
              auto bb_B = leaf_B->bb_leaf_room;
              if (bb_A.bottom() > bb_B.top())
                return false;
              
              int r0 = bb_A.bottom();
              int r1 = bb_B.top();
              int c0 = std::max(bb_A.left(), bb_B.left());
              int c1 = std::min(bb_A.right(), bb_B.right());
              if (c0 > c1)
                return false;
              if (bb_A.left() < bb_B.left() && bb_A.right() - bb_B.left() < 2*min_corridor_half_width)
                return false;
              if (bb_B.left() < bb_A.left() && bb_B.right() - bb_A.left() < 2*min_corridor_half_width)
                return false;
              bool collided = false;
              auto collides_with_bb = [r0, r1, c0, c1](const ttl::Rectangle bb) -> bool
              {
                if (r0 <= bb.top() && bb.bottom() <= r1)
                {
                  if (bb.left() <= c0 && c1 <= bb.right())
                    return true;
                  if (c0 <= bb.left() && bb.left() <= c1)
                    return true;
                  if (c0 <= bb.right() && bb.right() <= c1)
                    return true;
                }
                return false;
              };
              for (auto* leaf_C : leaves)
              {
                auto bb_C = leaf_C->bb_leaf_room;
                collided = collides_with_bb(bb_C);
                if (collided)
                  break;
              }
              if (!collided)
              {
                for (const auto& cp : room_corridor_map)
                {
                  auto bb_corr = cp.second->bb;
                  collided = collides_with_bb(bb_corr);
                  if (collided)
                    break;
                }
              }
              if (!collided)
              {
                auto* min_leaf = stlutils::select_if(leaf_A, leaf_B, [](auto* a, auto* b) { return a->id < b->id; });
                auto* max_leaf = stlutils::select_if(leaf_A, leaf_B, [](auto* a, auto* b) { return a->id > b->id; });
                auto key = std::pair { min_leaf, max_leaf };
                auto it = room_corridor_map.find(key);
                if (it == room_corridor_map.end())
                {
                  auto* corr = corridors.emplace_back(std::make_unique<Corridor>()).get();
                  corr->bb = { r0, (c0 + c1)/2 - min_corridor_half_width, r1 - r0 + 1, 2*min_corridor_half_width + 1 };
                  corr->orientation = Orientation::Vertical;
                  size_t surf_area = corr->bb.r_len * corr->bb.c_len;
                  corr->fog_of_war.resize(surf_area, true);
                  corr->light.resize(surf_area, false);
                  room_corridor_map[key] = corr;
                  return true;
                }
              }
              return false;
            };
            
            if (!try_make_horizontal_corridor(leaf_A, leaf_B))
              try_make_vertical_corridor(leaf_A, leaf_B);

          }
        }
      }
    }
    
    void create_doors(int max_num_locked_doors, bool allow_passageways)
    {
      int key_id_ctr = 0;
      int num_locked_doors = 0;
      for (auto& cp : room_corridor_map)
      {
        auto* room_0 = cp.first.first;
        auto* room_1 = cp.first.second;
        auto* door_0 = doors.emplace_back(std::make_unique<Door>()).get();
        auto* door_1 = doors.emplace_back(std::make_unique<Door>()).get();
        
        if (allow_passageways)
        {
          door_0->is_door = rnd::rand_bool();
          door_1->is_door = rnd::rand_bool();
        }
        else
        {
          door_0->is_door = true;
          door_1->is_door = true;
        }
        
        if (num_locked_doors < max_num_locked_doors)
        {
          if (door_0->is_door)
            door_0->is_locked = rnd::rand_bool();
          if (door_1->is_door && (door_0->is_door && !door_0->is_locked))
            door_1->is_locked = rnd::rand_bool();
            
          if (door_0->is_locked)
            num_locked_doors++;
          if (door_1->is_locked)
            num_locked_doors++;
            
          door_0->key_id = key_id_ctr++;
          door_1->key_id = key_id_ctr++;
        }
        
        auto* corr = cp.second;
        door_0->corridor = corr;
        door_1->corridor = corr;
        constexpr auto err_msg = "ERROR in BSPTree::create_doors() : Unable to find a door for room.";
        switch (corr->orientation)
        {
          case Orientation::Horizontal:
          {
            auto r_mid = corr->bb.r + corr->bb.r_len / 2;
            door_0->pos = { r_mid, corr->bb.left() };
            door_1->pos = { r_mid, corr->bb.right() };
            
            if (room_0->bb_leaf_room.right() == corr->bb.left())
            {
              room_0->doors.emplace_back(door_0);
              door_0->room = room_0;
            }
            else if (room_1->bb_leaf_room.right() == corr->bb.left())
            {
              room_1->doors.emplace_back(door_0);
              door_0->room = room_1;
            }
            else
              std::cerr << err_msg << std::endl;
              
            if (room_0->bb_leaf_room.left() == corr->bb.right())
            {
              room_0->doors.emplace_back(door_1);
              door_1->room = room_0;
            }
            else if (room_1->bb_leaf_room.left() == corr->bb.right())
            {
              room_1->doors.emplace_back(door_1);
              door_1->room = room_1;
            }
            else
              std::cerr << err_msg << std::endl;
            break;
          }
          case Orientation::Vertical:
          {
            auto c_mid = corr->bb.c + corr->bb.c_len / 2;
            door_0->pos = { corr->bb.top(), c_mid };
            door_1->pos = { corr->bb.bottom(), c_mid };
            
            if (room_0->bb_leaf_room.bottom() == corr->bb.top())
            {
              room_0->doors.emplace_back(door_0);
              door_0->room = room_0;
            }
            else if (room_1->bb_leaf_room.bottom() == corr->bb.top())
            {
              room_1->doors.emplace_back(door_0);
              door_0->room = room_1;
            }
            else
              std::cerr << err_msg << std::endl;
              
            if (room_0->bb_leaf_room.top() == corr->bb.bottom())
            {
              room_0->doors.emplace_back(door_1);
              door_1->room = room_0;
            }
            else if (room_1->bb_leaf_room.top() == corr->bb.bottom())
            {
              room_1->doors.emplace_back(door_1);
              door_1->room = room_1;
            }
            else
              std::cerr << err_msg << std::endl;
            break;
          }
        }
        corr->doors[0] = door_0;
        corr->doors[1] = door_1;
      }
    }
    
    std::map<std::pair<BSPNode*, BSPNode*>, Corridor*, PtrPairLess<BSPNode>> get_room_corridor_map() const
    {
      return room_corridor_map;
    }
    
    std::vector<Door*> fetch_doors() const
    {
      std::vector<Door*> doors_raw;
      doors_raw.reserve(doors.size());
      for (const auto& d : doors)
        doors_raw.emplace_back(d.get());
      return doors_raw;
    }
    
    template<int NR, int NC>
    void draw_regions(ScreenHandler<NR, NC>& sh,
                      int r0 = 0, int c0 = 0,
                      const styles::Style& border_style = { Color::Black, Color::Yellow }) const
    {
      m_root.draw_regions(sh, r0, c0, border_style);
    }
    
    template<int NR, int NC>
    void draw_rooms(ScreenHandler<NR, NC>& sh,
                    int r0 = 0, int c0 = 0,
                    const styles::Style& room_style = { Color::White, Color::DarkRed }) const
    {
      m_root.draw_rooms(sh, r0, c0, room_style);
    }
    
    template<int NR, int NC>
    void draw_corridors(ScreenHandler<NR, NC>& sh,
                        int r0 = 0, int c0 = 0,
                        const styles::Style& corridor_outline_style = { Color::Green, Color::DarkGreen },
                        const styles::Style& corridor_fill_style = { Color::Black, Color::Green }) const
    {
      for (const auto& corr : room_corridor_map)
      {
        const auto& bb = corr.second->bb;
        drawing::draw_box_outline(sh, r0 + bb.r, c0 + bb.c, bb.r_len, bb.c_len, drawing::OutlineType::Hash, corridor_outline_style);
        drawing::draw_box(sh, r0 + bb.r, c0 + bb.c, bb.r_len, bb.c_len, corridor_fill_style);
      }
    }
    
    void print_tree() const
    {
      m_root.print_tree();
    }
  };
  
}
