/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory for Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

#include "Utils/CalculatorBasics/Results.h"
#include "PropertyList.h"
#include "Utils/Bonds/BondOrderCollection.h"
#include <boost/optional.hpp>

namespace Scine {
namespace Utils {

Results::Results() = default;
Results::Results(Results&& /*rhs*/) noexcept = default;
Results::Results(const Results& rhs) = default;

Results& Results::operator=(const Results& rhs) {
  static_assert(std::is_move_assignable<Results>::value, "Must be move assignable!");
  *this = Results(rhs);
  return *this;
}

PropertyList Results::allContainedProperties() const {
  PropertyList propertyList;
  // Loop over all possible properties and check whether they are available
  for (unsigned i = 0; i < allProperties.size(); ++i) {
    if (resultsMap_.find(allProperties.at(i)) != resultsMap_.end()) {
      propertyList.addProperty(allProperties.at(i));
    }
  }
  return propertyList;
}

Results::~Results() = default;

Results& Results::operator=(Results&& /*rhs*/) noexcept = default;

} // namespace Utils
} // namespace Scine
