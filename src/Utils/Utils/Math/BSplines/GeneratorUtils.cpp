/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory for Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

#include "GeneratorUtils.h"

namespace Scine {
namespace Utils {

namespace BSplines {

namespace GeneratorUtils {

Eigen::VectorXd generateParametersByEquallySpacedMethod(const Eigen::MatrixXd& dataPoints) {
  auto m = dataPoints.rows() - 1;
  Eigen::VectorXd uBar(m + 1);

  uBar(0) = 0;
  for (int g = 1; g < m; ++g) {
    uBar(g) = static_cast<double>(g) / m;
  }
  uBar(m) = 1;

  return uBar;
}

Eigen::VectorXd generateParametersByCentripetalMethod(const Eigen::MatrixXd& dataPoints) {
  auto m = dataPoints.rows() - 1;
  Eigen::VectorXd uBar(m + 1);

  double sum = 0;
  for (int g = 1; g <= m; ++g) {
    Eigen::VectorXd diff = dataPoints.row(g) - dataPoints.row(g - 1);
    sum += sqrt(diff.norm());
  }

  if (sum <= 1e-10) {
    // If all data points are identical, nan would be returned otherwise
    return generateParametersByEquallySpacedMethod(dataPoints);
  }

  uBar(0) = 0;
  for (int g = 1; g < m; ++g) {
    uBar(g) = uBar(g - 1);
    Eigen::VectorXd diff = dataPoints.row(g) - dataPoints.row(g - 1);
    uBar[g] += sqrt(diff.norm()) / sum;
  }
  uBar(m) = 1;

  return uBar;
}

Eigen::VectorXd generateParametersByChordLengthMethod(const Eigen::MatrixXd& dataPoints) {
  auto m = dataPoints.rows() - 1;
  Eigen::VectorXd uBar(m + 1);

  double sum = 0;

  for (int g = 1; g <= m; ++g) {
    Eigen::VectorXd diff = dataPoints.row(g) - dataPoints.row(g - 1);
    sum += diff.norm();
  }

  uBar(0) = 0;
  for (int g = 1; g < m; ++g) {
    uBar(g) = uBar(g - 1);
    Eigen::VectorXd diff = dataPoints.row(g) - dataPoints.row(g - 1);
    uBar[g] += diff.norm() / sum;
  }
  uBar(m) = 1;
  return uBar;
}

Eigen::VectorXd generateKnotVectorByDeBoorsMethod(int splineDegree, int numberPolynomialSegments, const Eigen::VectorXd& uBar) {
  /* For the least squares fitting, parameters must be generated by the chord-length method. Otherwise the resulting
   * system of linear equations would be singular.
   * */
  Eigen::VectorXd U_(numberPolynomialSegments + splineDegree + 2);
  U_.setZero();
  double d, alpha;

  /*! set the first p+1 knots with zeros*/
  for (int h = 0; h <= splineDegree; ++h) {
    U_(h) = 0;
  }

  /*! calculate the internal knots by de Boor algorithm */ // TODO is this the same algorithm as in BSpline.cpp?
  for (int j = 1; j <= numberPolynomialSegments - splineDegree; ++j) {
    auto m = uBar.size() - 1;
    d = double(m + 1) / double(numberPolynomialSegments - splineDegree + 1);
    auto i = static_cast<int>(j * d);
    alpha = j * d - i;
    U_(splineDegree + j) = (1 - alpha) * uBar(i - 1) + alpha * uBar(i);
  }

  /*! set the last p+1 knots with ones*/
  for (int h = numberPolynomialSegments + 1; h <= numberPolynomialSegments + splineDegree + 1; ++h) {
    U_(h) = 1;
  }
  return U_;
}

Eigen::VectorXd generateKnotVectorByUniformMethod(int splineDegree, int numberPolynomialSegments) {
  Eigen::VectorXd U_(numberPolynomialSegments + splineDegree + 2);
  U_.setZero();

  /*! set the first p+1 knots with zeros*/
  for (int h = 0; h <= splineDegree; ++h) {
    U_(h) = 0;
  }

  /*! calculate the internal knots*/
  for (int j = 1; j <= numberPolynomialSegments - splineDegree; ++j) {
    U_(j + splineDegree) = double(j) / double((numberPolynomialSegments - splineDegree + 1));
  }

  /*! set the last p+1 knots with ones*/
  for (int h = numberPolynomialSegments + 1; h <= numberPolynomialSegments + splineDegree + 1; ++h) {
    U_(h) = 1;
  }
  return U_;
}

Eigen::VectorXd generateKnotVectorByKnotAveraging(int splineDegree, int numberPolynomialSegments, const Eigen::VectorXd& uBar) {
  Eigen::VectorXd U_(numberPolynomialSegments + splineDegree + 2);
  U_.setZero();

  /*! set the first p+1 knots with zeros*/
  for (int h = 0; h <= splineDegree; ++h) {
    U_(h) = 0;
  }

  /*! calculate the internal knots by knot averaging */
  for (int j = 1; j <= numberPolynomialSegments - splineDegree; ++j) {
    double parameterSum = 0.0;
    for (int i = j; i <= j + splineDegree - 1; ++i) {
      parameterSum += uBar[i];
    }
    U_(j + splineDegree) = parameterSum / double(splineDegree);
  }

  /*! set the last p+1 knots with ones*/
  for (int h = numberPolynomialSegments + 1; h <= numberPolynomialSegments + splineDegree + 1; ++h) {
    U_(h) = 1;
  }
  return U_;
}

} // namespace GeneratorUtils

} // namespace BSplines
} // namespace Utils
} // namespace Scine