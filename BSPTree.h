//
//  BSPTree.h
//  DungGine
//
//  Created by Rasmus Anthin on 2024-06-08.
//

#pragma once
#include <Termin8or/RC.h>
#include <Termin8or/SpriteHandler.h>
#include <Core/Rand.h>
#include <Core/Math.h>
#include <array>
#include <memory>


namespace bsp
{

  enum class Orientation { Vertical, Horizontal };
  
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
  
  template<int NR, int NC>
  void draw_box(SpriteHandler<NR, NC>& sh,
                int r, int c, int len_r, int len_c,
                Text::Color fg_color, Text::Color bg_color,
                bool filled = false)
  {
    // len_r = 3, len_c = 2
    // ###
    // # #
    // # #
    // ###
    //
    // len_r = 4, len_c = 4 (smallest room size by default)
    // #####
    // #   #
    // #   #
    // #   #
    // #####
    
    auto str_horiz = str::rep_char('#', len_c);
    sh.write_buffer(str_horiz, r, c, fg_color, bg_color);
    for (int i = r; i <= r + len_r; ++i)
    {
      if (filled)
        sh.write_buffer(str_horiz, i, c, fg_color, bg_color);
      else
      {
        sh.write_buffer("#", i, c, fg_color, bg_color);
        sh.write_buffer("#", i, c + len_c, fg_color, bg_color);
      }
    }
    sh.write_buffer(str_horiz, r + len_r, c, fg_color, bg_color);
  }

  struct BSPNode final
  {
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
    
    ttl::Rectangle corridor; // Corridor between the children of this region.
    bool has_corridor = false;
    
    int level = 0; // root = 0;
    
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
    void draw_regions(SpriteHandler<NR, NC>& sh,
                      int r0, int c0,
                      const styles::Style& border_style) const
    {
      draw_box(sh, r0 + bb_region.r, c0 + bb_region.c, bb_region.r_len, bb_region.c_len, border_style.fg_color, border_style.bg_color);
      
      if (children[0])
        children[0]->draw_regions(sh, r0, c0, border_style);
      if (children[1])
        children[1]->draw_regions(sh, r0, c0, border_style);
    }
    
    template<int NR, int NC>
    void draw_rooms(SpriteHandler<NR, NC>& sh,
                    int r0, int c0,
                    const styles::Style& room_style) const
    {
      if (!bb_leaf_room.is_empty())
        draw_box(sh,
                 r0 + bb_leaf_room.r, c0 + bb_leaf_room.c, bb_leaf_room.r_len, bb_leaf_room.c_len,
                 room_style.fg_color, room_style.bg_color);
                 
      if (children[0])
        children[0]->draw_rooms(sh, r0, c0, room_style);
      if (children[1])
        children[1]->draw_rooms(sh, r0, c0, room_style);
    }
    
    void print_tree(const std::string& indent = "") const
    {
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
      }
      else
      {
        if (children[0])
          children[0]->pad_rooms(min_room_length, min_rnd_wall_padding, max_rnd_wall_padding);
        if (children[1])
          children[1]->pad_rooms(min_room_length, min_rnd_wall_padding, max_rnd_wall_padding);
      }
    }
    
    void create_corridors(int min_corridor_half_width)
    {
      //        R
      //     /     \
      //    C0      C1
      //   /  \    /  \
      //  C00 C01 C10 C11 leafs
      //
      //        R
      //     /     \
      //    C0      C1
      //   /  \    /  \
      //  C00=C01 C10=C11 leafs
      //
      //        R
      //     /     \
      //    C0======C1
      //   /  \    /  \
      //  C00=C01 C10=C11 leafs
      //
      if (children[0])
        children[0]->create_corridors(min_corridor_half_width);
      if (children[1])
        children[1]->create_corridors(min_corridor_half_width);
      if (!is_leaf() && !has_corridor)
      {
        auto* ch_0 = children[0].get();
        auto* ch_1 = children[1].get();
        auto center_0 = ch_0->bb_region.center();
        auto center_1 = ch_1->bb_region.center();
        auto c_0 = ch_0->is_leaf() ? (ch_0->bb_leaf_room.right()) : center_0.c;
        auto c_1 = ch_1->is_leaf() ? (ch_1->bb_leaf_room.left()) : center_1.c;
        auto r_0 = ch_0->is_leaf() ? (ch_0->bb_leaf_room.bottom()) : center_0.r;
        auto r_1 = ch_1->is_leaf() ? (ch_1->bb_leaf_room.top()) : center_1.r;
        int r_avg = (center_0.r + center_1.r)/2;
        int c_avg = (center_0.c + center_1.c)/2;
        switch (orientation)
        {
          case Orientation::Vertical:
            corridor.r = r_avg - min_corridor_half_width;
            corridor.c = c_0;
            corridor.r_len = 2*min_corridor_half_width;
            corridor.c_len = c_1 - c_0;
            break;
          case Orientation::Horizontal:
            corridor.r = r_0;
            corridor.c = c_avg - min_corridor_half_width;
            corridor.r_len = r_1 - r_0;
            corridor.c_len = 2*min_corridor_half_width;
            break;
        }
        has_corridor = true;
      }
    }
    
    template<int NR, int NC>
    void draw_corridors(SpriteHandler<NR, NC>& sh,
                        int r0, int c0,
                        const styles::Style& corridor_style) const
    {
      if (children[0])
        children[0]->draw_corridors(sh, r0, c0, corridor_style);
      if (children[1])
        children[1]->draw_corridors(sh, r0, c0, corridor_style);
      if (!is_leaf() && has_corridor)
      {
        draw_box(sh, r0 + corridor.r, c0 + corridor.c, corridor.r_len, corridor.c_len, corridor_style.fg_color, corridor_style.bg_color);
      }
    }
    
    void collect_leafs(std::vector<BSPNode*>& leafs)
    {
      if (is_leaf())
        leafs.emplace_back(this);
      if (children[0])
        children[0]->collect_leafs(leafs);
      if (children[1])
        children[1]->collect_leafs(leafs);
    }
  };
  
  // //////////////////////////////////////////////////////////////

  class BSPTree final
  {
    BSPNode m_root;
    int m_min_room_length = 4;
    std::map<std::pair<BSPNode*, BSPNode*>, ttl::Rectangle> corridors;
    
  public:
    BSPTree() = default;
    BSPTree(int min_room_length)
      : m_min_room_length(min_room_length)
    {}
    
    void generate(int world_size_rows, int world_size_cols,
                  Orientation first_split_orientation)
    {
      m_root.orientation = first_split_orientation;
      m_root.size_rows = world_size_rows;
      m_root.size_cols = world_size_cols;
      ttl::Rectangle bb { 0, 0, m_root.size_rows, m_root.size_cols };
      m_root.generate(bb, 0, m_min_room_length);
    }
    
    void pad_rooms(int min_rnd_wall_padding = 1, int max_rnd_wall_padding = 4)
    {
      m_root.pad_rooms(m_min_room_length, min_rnd_wall_padding, max_rnd_wall_padding);
    }
    
    void create_corridors(int min_corridor_half_width = 1)
    {
      m_root.create_corridors(min_corridor_half_width);
    }
    
    void create_corridors2(int min_corridor_half_width = 1)
    {
      std::vector<BSPNode*> leafs;
      m_root.collect_leafs(leafs);
      
      for (auto* leaf_A : leafs)
      {
        for (auto* leaf_B : leafs)
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
              bool collided = false;
              for (auto* leaf_C : leafs)
              {
                auto bb_C = leaf_C->bb_leaf_room;
                if (c0 <= bb_C.left() && bb_C.right() <= c1)
                {
                  if (bb_C.top() <= r0 && r1 <= bb_C.bottom())
                    collided = true;
                  else if (r0 <= bb_C.top() && bb_C.top() <= r1)
                    collided = true;
                  else if (r0 <= bb_C.bottom() && bb_C.bottom() <= r1)
                    collided = true;
                }
                if (collided)
                  break;
              }
              if (!collided)
              {
                auto key = std::pair { std::min(leaf_A, leaf_B), std::max(leaf_A, leaf_B) };
                auto it = corridors.find(key);
                if (it == corridors.end())
                {
                  corridors[key] = { (r0 + r1)/2 - min_corridor_half_width, c0, 2*min_corridor_half_width, c1 - c0 };
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
              bool collided = false;
              for (auto* leaf_C : leafs)
              {
                auto bb_C = leaf_C->bb_leaf_room;
                if (r0 <= bb_C.top() && bb_C.bottom() <= r1)
                {
                  if (bb_C.left() <= c0 && c1 <= bb_C.right())
                    collided = true;
                  else if (c0 <= bb_C.left() && bb_C.left() <= c1)
                    collided = true;
                  else if (c0 <= bb_C.right() && bb_C.right() <= c1)
                    collided = true;
                }
                if (collided)
                  break;
              }
              if (!collided)
              {
                auto key = std::pair { std::min(leaf_A, leaf_B), std::max(leaf_A, leaf_B) };
                auto it = corridors.find(key);
                if (it == corridors.end())
                {
                  corridors[key] = { r0, (c0 + c1)/2 - min_corridor_half_width, r1 - r0, 2*min_corridor_half_width };
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
    
    template<int NR, int NC>
    void draw_regions(SpriteHandler<NR, NC>& sh,
                      int r0 = 0, int c0 = 0,
                      const styles::Style& border_style = { Text::Color::Black, Text::Color::Yellow }) const
    {
      m_root.draw_regions(sh, r0, c0, border_style);
    }
    
    template<int NR, int NC>
    void draw_rooms(SpriteHandler<NR, NC>& sh,
                    int r0 = 0, int c0 = 0,
                    const styles::Style& room_style = { Text::Color::White, Text::Color::DarkRed }) const
    {
      m_root.draw_rooms(sh, r0, c0, room_style);
    }
    
    template<int NR, int NC>
    void draw_corridors(SpriteHandler<NR, NC>& sh,
                        int r0 = 0, int c0 = 0,
                        const styles::Style& corridor_style = { Text::Color::Green, Text::Color::DarkGreen }) const
    {
      m_root.draw_corridors(sh, r0, c0, corridor_style);
    }
    
    template<int NR, int NC>
    void draw_corridors2(SpriteHandler<NR, NC>& sh,
                         int r0 = 0, int c0 = 0,
                         const styles::Style& corridor_style = { Text::Color::Green, Text::Color::DarkGreen }) const
    {
      for (const auto& corr : corridors)
      {
        const auto& corridor = corr.second;
        draw_box(sh, r0 + corridor.r, c0 + corridor.c, corridor.r_len, corridor.c_len, corridor_style.fg_color, corridor_style.bg_color);
      }
    }
    
    void print_tree() const
    {
      m_root.print_tree();
    }
  };
  
}
