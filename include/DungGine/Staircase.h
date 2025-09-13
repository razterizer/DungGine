//
//  Staircase.h
//  DungGine
//
//  Created by Rasmus Anthin on 2025-08-05.
//

namespace dung
{
  
  struct BSPNode;
  
  
  struct Staircase
  {
    RC pos; // world pos
    
    BSPNode* room_floor_A = nullptr;
    BSPNode* room_floor_B = nullptr;
    int floor_A = 0; // =0 : ground level, >0 underground.
    int floor_B = 1;
    
    bool fog_of_war = true;
    bool light = false;
  };

}
