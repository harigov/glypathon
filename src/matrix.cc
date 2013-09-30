#include "matrix.h"

#include <sstream>

Matrix::Matrix(size_t r, size_t c)
  : rows_(r), cols_(c)
{
  // Array storing matrix in row major order
  data_ = new float[rows_ * cols_];
  for (size_t i = 0; i < (rows_ * cols_); ++i)
  {
    data_[i] = 0;
  }
}

Matrix::~Matrix()
{
  delete[] data_;
}

const Matrix& Matrix::Transpose()
{
  Matrix* transpose = new Matrix(cols_, rows_);
  for (size_t r = 0; r < rows_; ++r)
  {
    for (size_t c = 0; c < cols_; ++c)
    {
      transpose->data_[c * rows_ + r] = data_[r * cols_ + c];
    }
  }
  return *transpose;
}

std::string Matrix::ToString()
{
  std::stringstream output;
  for (size_t r = 0; r < rows_; ++r)
  {
    for (size_t c = 0; c < cols_; ++c)
    {
      output << data_[r * cols_ + c] << " ";
    }
    output << std::endl;
  }
  return output.str();
}


