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
  
  bool is_dry(Terrain terrain)
  {
    switch (terrain)
    {
      case Terrain::Water:
      case Terrain::Lava:
      case Terrain::Swamp:
      case Terrain::Poison:
      case Terrain::Acid:
      case Terrain::Tar:
        return false;
      default:
        return true;
    }
  }
  
  bool is_wet(Terrain terrain)
  {
    switch (terrain)
    {
      case Terrain::Water:
      case Terrain::Poison:
      case Terrain::Acid:
        return true;
      default:
        return false;
    }
  }
  
  bool allow_move_to(Terrain terrain)
  {
    switch (terrain)
    {
      case Terrain::Mountain: return false;
      case Terrain::Tree: return false;
      case Terrain::Column: return false;
      case Terrain::Masonry: return false;
      default: return true;
    }
  }
  
  // 1 : Most viscous.
  // 0 : Least viscous.
  std::optional<float> get_wet_viscosity(Terrain terrain)
  {
    switch (terrain)
    {
      case Terrain::Water: return 0.05f;
      case Terrain::Lava: return 0.9f;
      case Terrain::Swamp: return 0.7f;
      case Terrain::Poison: return 0.2f;
      case Terrain::Acid: return 0.1f;
      case Terrain::Tar: return 0.8f;
      default:
        return {};
    }
  }
  
  // 1 : highest resistance : hardest to walk over/in.
  // 0 : lowest resistance : easiest to walk over/in.
  // Some values are a bit arbitrary.
  std::optional<float> get_dry_resistance(Terrain terrain)
  {
    switch (terrain)
    {
      case Terrain::Default:  return 0.f;
      case Terrain::Void:     return 0.f;
      case Terrain::Stone:    return 0.f;
      case Terrain::Cave:     return 0.f;
      case Terrain::Mine:     return 0.f;
      case Terrain::Tile:     return 0.f;
      case Terrain::Ice:      return 0.f;
      case Terrain::Rope:     return 0.1f;
      case Terrain::Grass:    return 0.15f;
      case Terrain::Wood:     return 0.15f;
      case Terrain::Gravel:   return 0.25f;
      case Terrain::Path:     return 0.25f;
      case Terrain::Bone:     return 0.3f;
      case Terrain::Sand:     return 0.55f;
      case Terrain::Metal:    return 0.65f;
      case Terrain::Silver:   return 0.65f;
      case Terrain::Gold:     return 0.65f;
      case Terrain::Shrub:    return 0.7f;
      case Terrain::Brick:    return 0.75f;
      case Terrain::Tree:     return 1.f;
      case Terrain::Column:   return 1.f;
      case Terrain::Masonry:  return 1.f;
      case Terrain::Mountain: return 1.f;
      default: return {};
    }
  }
  
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
      default: return "n/a";
    }
  }

}
