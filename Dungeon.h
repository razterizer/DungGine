//
//  Dungeon.h
//  DungGine
//
//  Created by Rasmus Anthin on 2025-08-05.
//

#pragma once
#include "BSPTree.h"
#include "Staircase.h"
#include "Comparison.h"
#include <Termin8or/AABB.h>

namespace dung
{

  struct DungeonFloorParams
  {
    int min_room_length = 4;
    RC world_size { 100, 100 };
    Orientation first_split_orientation = dung::Orientation::Vertical;
    int room_padding_min = 1;
    int room_padding_max = 4;
    int min_corridor_half_width = 1;
    int max_num_locked_doors = 50;
    bool allow_passageways = true;
  };

  struct Dungeon
  {
    std::vector<std::unique_ptr<BSPTree>> bsp_forest; // levels of bsp-trees.
    
    std::vector<std::unique_ptr<Staircase>> staircases; // staircases between levels (bsp-trees).
    
    std::map<const BSPTree*, std::vector<BSPNode*>, PtrLess<BSPTree>> bsp_tree_rooms;
    
    RC world_size { 0, 0 };
    
    int init_floor = 0;
    const int begin_at_floor = -1;
    int m_num_floors = 0;
    
    bool first_floor_is_surface_level = true;
    
  public:
    // starting_floor = -1 : start from the bottom-most floor.
    Dungeon(bool first_level_is_over_ground, int starting_floor = -1)
      : begin_at_floor(starting_floor)
      , first_floor_is_surface_level(first_level_is_over_ground)
    {}
    
    void generate(const std::vector<DungeonFloorParams>& floor_params)
    {
      for (const auto& params : floor_params)
      {
        auto* bsp_tree = bsp_forest.emplace_back(std::make_unique<BSPTree>(params.min_room_length)).get();
        bsp_tree->generate(params.world_size.r, params.world_size.c, params.first_split_orientation);
        bsp_tree->pad_rooms(params.room_padding_min, params.room_padding_max);
        bsp_tree->create_corridors(params.min_corridor_half_width);
        bsp_tree->create_doors(params.max_num_locked_doors, params.allow_passageways);
      }
      for (const auto& bsp_tree : bsp_forest)
        if (bsp_tree == nullptr)
          std::cerr << "ERROR in Dungeon() : The vector of bsp-trees must not contain nullptrs!\n";
      for (auto& bsp_tree : bsp_forest)
      {
        bsp_tree_rooms[bsp_tree.get()] = bsp_tree->fetch_leaves();
        // #NOTE: Assumes all floors start at the same coordinate (i.e. (0, 0)).
        const auto& floor_size = bsp_tree->get_world_size();
        math::maximize(world_size.r, floor_size.r);
        math::maximize(world_size.c, floor_size.c);
      }
      m_num_floors = stlutils::sizeI(bsp_forest);
      if (begin_at_floor == -1)
        init_floor = m_num_floors - 1;
      else
        init_floor = math::clamp(begin_at_floor, 0, m_num_floors - 1);
    }
    
    void reset()
    {
      global_bsp_tree_id = 0;
      global_bsp_node_id = 0;
      global_corridor_id = 0;
      //for (auto& bsp_tree : bsp_forest)
      //  bsp_tree->reset();
      bsp_forest.clear();
      staircases.clear();
      bsp_tree_rooms.clear();
    }
    
    const RC& get_world_size() const
    {
      return world_size;
    }
    
    const std::vector<BSPTree*> get_trees() const
    {
      std::vector<BSPTree*> trees;
      trees.reserve(bsp_forest.size());
      for (auto& t : bsp_forest)
        trees.emplace_back(t.get());
      return trees;
    }
    
    int get_init_floor() const
    {
      return init_floor;
    }
    
    BSPTree* get_tree(int floor) const
    {
      if (!stlutils::in_range(bsp_forest, floor))
        return nullptr;
      return bsp_forest[floor].get();
    }
    
    const std::vector<BSPNode*>* get_rooms(BSPTree* bsp_tree) const
    {
      auto it = bsp_tree_rooms.find(bsp_tree);
      if (it != bsp_tree_rooms.end())
        return &(it->second);
      return nullptr;
    }
    
    // prob_in_room : inverse probability. A value of 10 means probability 1/10 or about once every ten times.
    void create_staircases(int prob_in_room = 10)
    {
      AABB<int> aabb_A, aabb_B;
      for (int f_idx = 1; f_idx < m_num_floors; ++f_idx)
      {
        auto* floor_A = bsp_forest[f_idx - 1].get(); // upper floor
        auto* floor_B = bsp_forest[f_idx].get(); // lower floor
        
        const auto& rooms_A = bsp_tree_rooms[floor_A];
        const auto& rooms_B = bsp_tree_rooms[floor_B];
        
        for (auto* rA : rooms_A)
        {
          aabb_A = rA->bb_leaf_room.extrude(-1); // We only want to intersect the interior of the rooms.
          for (auto* rB : rooms_B)
          {
            if (rB->staircase == nullptr && rnd::one_in(prob_in_room))
            {
              aabb_B = rB->bb_leaf_room.extrude(-1); // We only want to intersect the interior of the rooms.
              if (aabb_A.overlaps(aabb_B))
              {
                auto bb_isect = aabb_A.set_intersect(aabb_B).to_rectangle();
                if (bb_isect.is_empty())
                  continue;
                auto* stairs = staircases.emplace_back(std::make_unique<Staircase>()).get();
                stairs->pos = {
                  rnd::rand_int(bb_isect.top(), bb_isect.bottom()),
                  rnd::rand_int(bb_isect.left(), bb_isect.right())
                };
                stairs->room_floor_A = rA;
                stairs->room_floor_B = rB;
                stairs->floor_A = f_idx - 1;
                stairs->floor_B = f_idx;
                rA->staircase = stairs;
                rB->staircase = stairs;
                break; // We don't want more than one staircase in room A.
              }
            }
          }
        }
      }
    }
    
    int num_floors() const
    {
      return m_num_floors;
    }
    
    bool is_first_floor_is_surface_level() const
    {
      return first_floor_is_surface_level;
    }
    
    std::vector<Staircase*> fetch_staircases(int floor) const
    {
      std::vector<Staircase*> staircases_raw;
      staircases_raw.reserve(staircases.size());
      for (const auto& s : staircases)
        if (s->floor_A == floor || s->floor_B == floor)
          staircases_raw.emplace_back(s.get());
      return staircases_raw;
    }
    
    void serialize(std::vector<std::string>& lines) const
    {
      for (auto* bsp_tree : get_trees())
      {
        lines.emplace_back("bsp_tree");
        bsp_tree->serialize(lines);
      }
    }
    
    std::vector<std::string>::iterator deserialize(std::vector<std::string>::iterator it_line_begin,
                                                   std::vector<std::string>::iterator it_line_end)
    {
      auto it_line = it_line_begin;
      for (auto* t : get_trees())
        if (*it_line == "bsp_tree")
          it_line = t->deserialize(it_line + 1, it_line_end) + 1;
      return it_line - 1;
    }
  };

}
