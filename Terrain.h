//
//  Terrain.h
//  DungGine
//
//  Created by Rasmus Anthin on 2024-08-11.
//

#pragma once

#define TERRAIN_TO_STR(sym) case Terrain::sym: return #sym;
#define STR_TO_TERRAIN(sym) if (str == #sym) return Terrain::sym;

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
      TERRAIN_TO_STR(Default);
      TERRAIN_TO_STR(Void);
      TERRAIN_TO_STR(Water);
      TERRAIN_TO_STR(Sand);
      TERRAIN_TO_STR(Gravel);
      TERRAIN_TO_STR(Stone);
      TERRAIN_TO_STR(Mountain);
      TERRAIN_TO_STR(Lava);
      TERRAIN_TO_STR(Cave);
      TERRAIN_TO_STR(Swamp);
      TERRAIN_TO_STR(Poison);
      TERRAIN_TO_STR(Acid);
      TERRAIN_TO_STR(Tar);
      TERRAIN_TO_STR(Path);
      TERRAIN_TO_STR(Mine);
      TERRAIN_TO_STR(Grass);
      TERRAIN_TO_STR(Shrub);
      TERRAIN_TO_STR(Tree);
      TERRAIN_TO_STR(Tile);
      TERRAIN_TO_STR(Masonry);
      TERRAIN_TO_STR(Column);
      TERRAIN_TO_STR(Brick);
      TERRAIN_TO_STR(Wood);
      TERRAIN_TO_STR(Ice);
      TERRAIN_TO_STR(Metal);
      TERRAIN_TO_STR(Silver);
      TERRAIN_TO_STR(Gold);
      TERRAIN_TO_STR(Bone);
      TERRAIN_TO_STR(Rope);
      default: return "n/a";
    }
  }
  
  std::optional<Terrain> str2terrain(const std::string& str)
  {
    STR_TO_TERRAIN(Default);
    STR_TO_TERRAIN(Void);
    STR_TO_TERRAIN(Water);
    STR_TO_TERRAIN(Sand);
    STR_TO_TERRAIN(Gravel);
    STR_TO_TERRAIN(Stone);
    STR_TO_TERRAIN(Mountain);
    STR_TO_TERRAIN(Lava);
    STR_TO_TERRAIN(Cave);
    STR_TO_TERRAIN(Swamp);
    STR_TO_TERRAIN(Poison);
    STR_TO_TERRAIN(Acid);
    STR_TO_TERRAIN(Tar);
    STR_TO_TERRAIN(Path);
    STR_TO_TERRAIN(Mine);
    STR_TO_TERRAIN(Grass);
    STR_TO_TERRAIN(Shrub);
    STR_TO_TERRAIN(Tree);
    STR_TO_TERRAIN(Tile);
    STR_TO_TERRAIN(Masonry);
    STR_TO_TERRAIN(Column);
    STR_TO_TERRAIN(Brick);
    STR_TO_TERRAIN(Wood);
    STR_TO_TERRAIN(Ice);
    STR_TO_TERRAIN(Metal);
    STR_TO_TERRAIN(Silver);
    STR_TO_TERRAIN(Gold);
    STR_TO_TERRAIN(Bone);
    STR_TO_TERRAIN(Rope);
    return std::nullopt;
  }

}
