// Copyright 2018 Global Phasing Ltd.
//
// Chemical component represents a monomer from the Refmac monomer library,
// or from PDB CCD.

#ifndef GEMMI_CHEMCOMP_XYZ_HPP_
#define GEMMI_CHEMCOMP_XYZ_HPP_

#include <array>
#include "cifdoc.hpp"  // for Block, etc
#include "model.hpp" // for Atom, Residue, etc

namespace gemmi {

// Reading chemical component as a coordinate file.
enum class ChemCompModel {
  Xyz,     // _chem_comp_atom.x, etc
  Example, // _chem_comp_atom.model_Cartn_x
  Ideal    // _chem_comp_atom.pdbx_model_Cartn_x_ideal
};

inline Residue read_chem_comp_as_residue(const cif::Block& block,
                                         ChemCompModel kind) {
  std::array<std::string, 3> xyz_tags;
  switch (kind) {
    case ChemCompModel::Xyz:
      xyz_tags = {"x", "y", "z"};
      break;
    case ChemCompModel::Example:
      xyz_tags = {"model_Cartn_x", "model_Cartn_y", "model_Cartn_z"};
      break;
    case ChemCompModel::Ideal:
      xyz_tags = {"pdbx_model_Cartn_x_ideal",
                  "pdbx_model_Cartn_y_ideal",
                  "pdbx_model_Cartn_z_ideal"};
      break;
  }
  Residue res;
  cif::Column col =
    const_cast<cif::Block&>(block).find_values("_chem_comp_atom.comp_id");
  if (col && col.length() > 0)
    res.name = col[0];
  else
    res.name = block.name.substr(starts_with(block.name, "comp_") ? 5 : 0);
  cif::Table table = const_cast<cif::Block&>(block).find("_chem_comp_atom.",
          {"atom_id", "type_symbol", xyz_tags[0], xyz_tags[1], xyz_tags[2]});
  res.atoms.resize(table.length());
  int n = 0;
  for (auto row : table) {
    Atom& atom = res.atoms[n++];
    atom.name = row.str(0);
    atom.element = Element(row.str(1));
    atom.pos = Position(cif::as_number(row[2]),
                        cif::as_number(row[3]),
                        cif::as_number(row[4]));
  }
  return res;
}

inline Model read_chem_comp_as_model(const cif::Block& block,
                                     ChemCompModel kind) {
  std::string name;
  switch (kind) {
    case ChemCompModel::Xyz: name = "xyz"; break;
    case ChemCompModel::Example: name = "example_xyz"; break;
    case ChemCompModel::Ideal: name = "ideal_xyz"; break;
  }
  Model model(name);
  model.chains.emplace_back("");
  model.chains[0].residues.push_back(read_chem_comp_as_residue(block, kind));
  return model;
}

// For CCD input - returns a structure with two single-residue models:
// example (model_Cartn_x) and ideal (pdbx_model_Cartn_x_ideal).
// For Refmac dictionary (monomer library) files returns structure with
// a single model.
inline Structure read_chem_comp_as_structure(const cif::Block& block) {
  Structure st;
  if (block.has_tag("_chem_comp_atom.x"))
    st.models.push_back(read_chem_comp_as_model(block, ChemCompModel::Xyz));
  if (block.has_tag("_chem_comp_atom.model_Cartn_x"))
    st.models.push_back(read_chem_comp_as_model(block, ChemCompModel::Example));
  if (block.has_tag("_chem_comp_atom.pdbx_model_Cartn_x_ideal"))
    st.models.push_back(read_chem_comp_as_model(block, ChemCompModel::Ideal));
  return st;
}

} // namespace gemmi
#endif
// vim:sw=2:ts=2:et
