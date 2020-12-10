// MIT License

#pragma once

#include <vector>

namespace frcchar {
/**
 * Calculates multiple regression on the data set and returns the coefficients
 * of the regression, as well as the adjusted coefficient of determination.
 *
 * @param data  The data to perform the regression on.
 * @param variables  The number of independent variables (i.e. x values).
 *
 * @return The coefficients of the regression with the adjusted r-squared
 * appended to the vector.
 */
std::vector<double> OLS(const std::vector<double>& data, size_t variables);
}  // namespace frcchar
