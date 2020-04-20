/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory for Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#include "Utils/IO/ChemicalFileFormats/MOLStreamHandler.h"
#include "Utils/Bonds/BondOrderCollection.h"
#include "Utils/Constants.h"
#include "Utils/Geometry/AtomCollection.h"
#include "Utils/Geometry/ElementInfo.h"
#include "Utils/Technical/ScopedLocale.h"
#include "boost/optional.hpp"
#include <iomanip>
#include <numeric>

namespace Scine {

namespace Utils {

// Helper to insert newlines
inline std::ostream& nl(std::ostream& os) {
  os << "\n";
  return os;
}

std::pair<Utils::AtomCollection, Utils::BondOrderCollection> MOLStreamHandler::read(std::istream& is,
                                                                                    const std::string& format) const {
  if (format != "mol") {
    throw FormattedStreamHandler::FormatUnsupportedException();
  }

  auto data = read(is);

  return data;
}

void MOLStreamHandler::write(std::ostream& os, const std::string& format, const Utils::AtomCollection& atoms) const {
  if (format != "mol") {
    throw FormattedStreamHandler::FormatUnsupportedException();
  }

  write(os, atoms, boost::none, "V2000");
}

void MOLStreamHandler::write(std::ostream& os, const std::string& format, const AtomCollection& atoms,
                             const BondOrderCollection& bondOrders) const {
  if (format != "mol") {
    throw FormattedStreamHandler::FormatUnsupportedException();
  }

  write(os, atoms, bondOrders, "V2000");
}

std::vector<MOLStreamHandler::FormatSupportPair> MOLStreamHandler::formats() const {
  return {{{"mol", SupportType::ReadWrite}}};
}

bool MOLStreamHandler::formatSupported(const std::string& format, SupportType /* operation */
                                       ) const {
  if (format == "mol") {
    return true;
  }

  return false;
}

std::string MOLStreamHandler::name() const {
  return MOLStreamHandler::model;
}

const std::vector<std::string> MOLStreamHandler::versionStrings{"V2000", "V3000"};

void MOLStreamHandler::write(std::ostream& os, const AtomCollection& atoms,
                             const boost::optional<BondOrderCollection>& bondOrdersOption, const std::string& formatVersion) {
  /* Imbue the output stream with the C locale to ensure commas and periods are
   * consistent
   */
  os.imbue(std::locale("C"));
  const unsigned N = atoms.size();

  std::vector<unsigned> valences(N, 0u);
  // Count the number of bonds in the molecule if there is bond information present
  if (bondOrdersOption) {
    auto& bondOrders = bondOrdersOption.value();
    for (unsigned i = 0; i < N - 1; ++i) {
      for (unsigned j = i + 1; j < N; ++j) {
        double order = bondOrders.getOrder(i, j);
        // Discretized bond representation can only handle orders 1, 2 and 3
        if (0.5 <= order && order < 3.5) {
          ++valences[i];
          ++valences[j];
        }
      }
    }
  }

  /* Sum up the valences to obtain the total number of bonds.  Since we counted
   * valences for each atom, each bond is represented twice in valences, so we
   * halve the sum.
   */
  const unsigned B = std::accumulate(std::begin(valences), std::end(valences), 0u, std::plus<>()) / 2;

  os << std::setprecision(0);

  // Header: molecule name
  os << "Unnamed Molecule" << nl;

  // Header: Information about the program
  auto now = time(nullptr);
  auto localNow = *localtime(&now);

  os << std::setw(2) << "##"                   // First and last initial of user
     << std::setw(8) << "SCINE"                // PPPPPPPP (8, Prog name)
     << std::put_time(&localNow, "%m%d%y%H%M") // MMDDYYHHmm
     << "3D"                                   // dd (dimensionality)
     // Missing:
     // SS (integer scaling factor)
     // ss (float scaling factor, 10 digits long: bbbb.aaaaa)
     // EE (energy, 12 digits long: sbbbbb.aaaaa)
     // RRRRRR (registry number)
     << nl;

  // Header: Comments
  os << nl;

  // Counts line
  os << std::setw(3) << N             // aaa
     << std::setw(3) << B             // bbb
     << std::setw(3) << 0u            // lll (number of atom lists)
     << std::setw(3) << 0u            // fff (obsolete)
     << std::setw(3) << 0u            // ccc (chiral or not?)
     << std::setw(3) << 0u            // sss (num s-text entries, irrelevant here)
     << std::setw(3) << 0u            // xxx (obsolete)
     << std::setw(3) << 0u            // rrr (obsolete)
     << std::setw(3) << 0u            // ppp (obsolete)
     << std::setw(3) << 0u            // iii (obsolete)
     << std::setw(3) << 999u          // mmm (num add. prop.s, unsupported, default 999)
     << std::setw(6) << formatVersion // vvvvvv (Version string)
     << nl;

  // Atom block: one line per atom
  for (unsigned i = 0; i < N; ++i) {
    os << std::setprecision(4) << std::fixed << std::setw(10)
       << atoms.getPosition(i)(0) * Constants::angstrom_per_bohr                                  // x position
       << std::setw(10) << atoms.getPosition(i)(1) * Constants::angstrom_per_bohr                 // y position
       << std::setw(10) << atoms.getPosition(i)(2) * Constants::angstrom_per_bohr                 // z position
       << " " << std::setprecision(0) << std::setw(3) << ElementInfo::symbol(atoms.getElement(i)) // aaa (atom symbol)
       << std::setw(2) << 0u             // dd (isotope mass difference)
       << std::setw(3) << 0u             // ccc (local charge)
       << std::setw(3) << 0u             // sss (atom stereo parity, ignored)
       << std::setw(3) << 0u             // hhh (hydrogen count, for query, ignored)
       << std::setw(3) << 0u             // bbb (stereo care box??, ignored)
       << std::setw(3) << valences.at(i) // vvv (valence)
       << std::setw(3) << 0u             // HHH (H0 designator, ISIS/Desktop, ignored)
       << std::setw(3) << 0u             // rrr (unused)
       << std::setw(3) << 0u             // iii (unused)
       << std::setw(3) << 0u             // mmm (atom-atom mapping number, for reactions, ignored)
       << std::setw(3) << 0u             // nnn (inversion/retention flag, for reactions, ignored)
       << std::setw(3) << 0u             // eee (exact change flag, for reactions, ignored)
       << nl;
  }

  // Bond block: one line per bond
  if (bondOrdersOption) {
    auto& bondOrders = bondOrdersOption.value();
    for (unsigned i = 0; i < N - 1; ++i) {
      for (unsigned j = i + 1; j < N; ++j) {
        // Discretize bond orders to nearest integer
        unsigned bondOrder = std::round(bondOrders.getOrder(i, j));

        if (0 < bondOrder && bondOrder < 4) {
          os << std::setw(3) << (1 + i)   // 111 (index of 1st atom)
             << std::setw(3) << (1 + j)   // 222 (index of 2nd atom)
             << std::setw(3) << bondOrder // ttt (bond type)
             << std::setw(3) << 0u        // sss (bond stereo, ignored for now)
             << std::setw(3) << 0u        // xxx (unused)
             << std::setw(3) << 0u        // rrr (bond topology, ignored)
             << std::setw(3) << 0u        // ccc (reacting center status, ignored)
             << std::endl;
        }
      }
    }
  }

  // final line
  os << "M END";
}

std::pair<AtomCollection, BondOrderCollection> MOLStreamHandler::read(std::istream& is) {
  /* Make sure string to number conversions are performed in the c locale,
   * otherwise commas and periods might be interpreted differently
   */
  auto scopedLocale = ScopedLocale::cLocale();

  // Reset errno to avoid contamination from a previous program
  int priorErrno = errno;
  errno = 0;

  auto removeAllSpaces = [](std::string a) -> std::string {
    a.erase(std::remove(a.begin(), a.end(), ' '), a.end());

    return a;
  };

  auto skipLine = [](std::istream& is) {
    // Skip up to max characters until you encounter the locale-specific newline
    is.ignore(std::numeric_limits<std::streamsize>::max(), is.widen('\n'));
  };

  unsigned atomBlockSize = 0, bondBlockSize = 0;
  std::string line;

  // Header: molecule name
  skipLine(is);

  // Header: program name and other unnecessary details
  skipLine(is);

  // Header: comments
  skipLine(is);

  /* Now is possibly the first instance where we could have a counts line.
   * A counts line is formatted as:
   *
   * aaabbblllfffcccsssxxxrrrpppiiimmmvvvvv
   *
   * Eleven 3-character sequences identifying
   * - aaa: number of atoms
   * - bbb: number of bonds
   * - [...] lots of unneeded or obsolete things
   *
   * One 5-character sequence
   * - vvvvv: Version string (V2000 or V3000)
   */
  while (std::getline(is, line)) {
    // A valid counts line must have at least 38 (11x3 + 5) characters
    if (line.size() < 38) {
      continue;
    }

    // The aaa and bbb substrings must be valid integers
    try {
      std::size_t convertedChars = 0;
      atomBlockSize = (unsigned int)std::stoul(line.substr(0, 3), &convertedChars);
      if (convertedChars != 3) {
        atomBlockSize = 0;
        continue;
      }

      bondBlockSize = (unsigned int)std::stoul(line.substr(3, 3), &convertedChars);
      if (convertedChars != 3) {
        bondBlockSize = 0;
        continue;
      }
    }
    catch (...) {
      continue;
    }

    /* If no exceptions were thrown, the conversions worked and we have a valid
     * counts line. We don't want to fetch a new line though, so we break here.
     */
    break;
  }

  if (is.eof()) {
    throw FormatMismatchException();
  }

  std::string versionString = removeAllSpaces(line.substr(33));

  if (versionString == "V3000") {
    throw std::logic_error("V3000 MOL Format not implemented!");
  }

  AtomCollection atoms(atomBlockSize);

  // Atom block
  if (versionString == "V2000") {
    for (unsigned i = 0; i < atomBlockSize; ++i) {
      std::getline(is, line);

      // Check line length
      if (line.size() < 34) {
        throw FormattedStreamHandler::FormatMismatchException();
      }

      double x, y, z;
      try {
        x = std::stod(line.substr(0, 10));
        y = std::stod(line.substr(10, 10));
        z = std::stod(line.substr(20, 10));
      }
      catch (...) {
        throw FormattedStreamHandler::FormatMismatchException();
      }

      std::string elementStr = removeAllSpaces(line.substr(31, 3));
      // Make sure capitalization matches our variant
      std::transform(std::begin(elementStr), std::begin(elementStr) + 1, std::begin(elementStr), ::toupper);
      // Make other letters lowercase
      std::transform(std::begin(elementStr) + 1, std::end(elementStr), std::begin(elementStr) + 1, ::tolower);

      ElementType f;

      try {
        f = ElementInfo::elementTypeForSymbol(elementStr);
      }
      catch (...) {
        throw FormattedStreamHandler::FormatMismatchException();
      }

      atoms.setPosition(i, Position(x, y, z) * Constants::bohr_per_angstrom);
      atoms.setElement(i, f);
    }
  }

  BondOrderCollection bondOrders;

  // Bond block
  if (bondBlockSize > 0) {
    bondOrders.resize(atomBlockSize);
    // Read the bonds
    for (unsigned i = 0; i < bondBlockSize; ++i) {
      std::getline(is, line);

      unsigned a, b, molBondSpecifier;
      if (line.size() < 9) {
        throw FormattedStreamHandler::FormatMismatchException();
      }

      try {
        std::size_t convertedChars = 0;
        // MOLFile indices are 1-based, thus subtract one
        a = (unsigned int)std::stoul(line.substr(0, 3), &convertedChars) - 1;
        if (convertedChars != 3) {
          throw std::exception();
        }
        b = (unsigned int)std::stoul(line.substr(3, 3), &convertedChars) - 1;
        if (convertedChars != 3) {
          throw std::exception();
        }
        molBondSpecifier = (unsigned int)std::stoul(line.substr(6, 3), &convertedChars);
        if (convertedChars != 3) {
          throw std::exception();
        }
      }
      catch (...) {
        throw FormattedStreamHandler::FormatMismatchException();
      }

      if (0 < molBondSpecifier && molBondSpecifier < 4) {
        bondOrders.setOrder(a, b, molBondSpecifier);
      }
    }
  }

  /* Reset errno with the prior value (so that previous error information is
   * not lost)
   */
  errno = priorErrno;

  return std::make_pair(std::move(atoms), std::move(bondOrders));
}

} // namespace Utils

} // namespace Scine
