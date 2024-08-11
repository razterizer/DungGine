//
//  Terrain.h
//  DungGine
//
//  Created by Rasmus Anthin on 2024-08-11.
//

#pragma once

namespace dung
{

  enum class Terrain
  {
    Default,
    Void,
    Water,
    Sand,
    Gravel,
    Stone,
    Mountain,
    Lava,
    Cave,
    Swamp,
    Poison,
    Acid,
    Tar,
    Path,
    Mine,
    Grass,
    Shrub,
    Tree,
    Tile,
    Masonry,
    Column,
    Brick,
    Wood,
    Ice,
    Metal,
    Silver,
    Gold,
    Bone,
    Rope,
  };
  
  std::string terrain2str(Terrain terrain)
  {
    switch (terrain)
    {
      case Terrain::Default: return "Default";
      case Terrain::Void: return "Void";
      case Terrain::Water: return "Water";
      case Terrain::Sand: return "Sand";
      case Terrain::Gravel: return "Gravel";
      case Terrain::Stone: return "Stone";
      case Terrain::Mountain: return "Mountain";
      case Terrain::Lava: return "Lava";
      case Terrain::Cave: return "Cave";
      case Terrain::Swamp: return "Swamp";
      case Terrain::Poison: return "Poison";
      case Terrain::Acid: return "Acid";
      case Terrain::Tar: return "Tar";
      case Terrain::Path: return "Path";
      case Terrain::Mine: return "Mine";
      case Terrain::Grass: return "Grass";
      case Terrain::Shrub: return "Shrub";
      case Terrain::Tree: return "Tree";
      case Terrain::Tile: return "Tile";
      case Terrain::Masonry: return "Masonry";
      case Terrain::Column: return "Column";
      case Terrain::Brick: return "Brick";
      case Terrain::Wood: return "Wood";
      case Terrain::Ice: return "Ice";
      case Terrain::Metal: return "Metal";
      case Terrain::Silver: return "Silver";
      case Terrain::Gold: return "Gold";
      case Terrain::Bone: return "Bone";
      case Terrain::Rope: return "Rope";
    }
  }

}
