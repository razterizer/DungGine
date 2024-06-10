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
    
    ttl::Rectangle bb_leaf_room;
    
    std::array<std::unique_ptr<BSPNode>, 2> children;
    
    bool is_leaf() const { return !children[0] && !children[1]; }
    
    void generate(int min_room_length)
    {
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
        ch_0->generate(min_room_length);
        
        auto& ch_1 = children[1] = std::make_unique<BSPNode>();
        ch_1->orientation = child_orientation;
        f_set_ch_size(ch_1.get(), split_length_1);
        ch_1->generate(min_room_length);
      }
    }
    
    template<int NR, int NC>
    void draw_regions(SpriteHandler<NR, NC>& sh,
                      int r, int c,
                      const styles::Style& border_style) const
    {
      draw_box(sh, r, c, size_rows, size_cols, border_style.fg_color, border_style.bg_color);
      int ch_r = 0;
      int ch_c = 0;
      int ch0_r_len = 0;
      int ch0_c_len = 0;
      if (children[0])
      {
        ch_r = r;
        ch_c = c;
        ch0_r_len = children[0]->size_rows;
        ch0_c_len = children[0]->size_cols;
        children[0]->draw_regions(sh, ch_r, ch_c, border_style);
      }
      if (children[1])
      {
        switch (orientation)
        {
          case Orientation::Vertical:
            ch_r = r;
            ch_c = c + ch0_c_len;
            break;
          case Orientation::Horizontal:
            ch_r = r + ch0_r_len;
            ch_c = c;
            break;
        }
        children[1]->draw_regions(sh, ch_r, ch_c, border_style);
      }
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
    
    void print_tree(int level = 0, const std::string& indent = "") const
    {
      std::cout << indent << "Level: " << level << std::endl;
      std::cout << indent << "Orientation: " << (orientation == Orientation::Vertical ? "V" : "H") << std::endl;
      std::cout << indent << "Size: [" << size_rows << ", " << size_cols << "]" << std::endl;
      
      auto child_indent = indent + str::rep_char(' ', 2);
      
      if (children[0])
      {
        std::cout << indent << "Child 0:" << std::endl;
        children[0]->print_tree(level + 1, child_indent);
      }
      if (children[1])
      {
        std::cout << indent << "Child 1:" << std::endl;
        children[1]->print_tree(level + 1, child_indent);
      }
    }
    
    void pad_rooms(ttl::Rectangle bb, int min_room_length, int min_rnd_wall_padding, int max_rnd_wall_padding)
    {
      if (!children[0] && !children[1])
      {
        std::array<int, 4> padding_nswe { 0, 0, 0, 0 }; // top, bottom, left, right
        int num_tries = 0;
        do
        {
          for (int i = 0; i < 4; ++i)
            padding_nswe[i] = rnd::rand_int(min_rnd_wall_padding, max_rnd_wall_padding);
          bb_leaf_room = bb;
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
        int ch0_r_len = 0;
        int ch0_c_len = 0;
        if (children[0])
        {
          ch0_r_len = children[0]->size_rows;
          ch0_c_len = children[0]->size_cols;
          ttl::Rectangle bb_0 { bb.r, bb.c, ch0_r_len, ch0_c_len };
          children[0]->pad_rooms(bb_0, min_room_length, min_rnd_wall_padding, max_rnd_wall_padding);
        }
        if (children[1])
        {
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
          children[1]->pad_rooms(bb_1, min_room_length, min_rnd_wall_padding, max_rnd_wall_padding);
        }
      }
    }
  };
  
  // //////////////////////////////////////////////////////////////

  class BSPTree final
  {
    BSPNode m_root;
    int m_min_room_length = 4;
    
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
      m_root.generate(m_min_room_length);
    }
    
    void pad_rooms(int min_rnd_wall_padding = 1, int max_rnd_wall_padding = 4)
    {
      ttl::Rectangle bb { 0, 0, m_root.size_rows, m_root.size_cols };
      m_root.pad_rooms(bb, m_min_room_length, min_rnd_wall_padding, max_rnd_wall_padding);
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
    
    void print_tree() const
    {
      m_root.print_tree();
    }
  };
  
}
