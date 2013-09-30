#pragma once

#include <iostream>
#include <string>

class Matrix
{
  public:
    Matrix(size_t rows, size_t cols);
    ~Matrix();

    // Rows and column indices are zero-index based.
    float Get(size_t r, size_t c)
    {
      return data_[r * cols_ + c];
    };
    void Set(size_t r, size_t c, float value)
    {
      data_[r * cols_ + c] = value;
    };

    const Matrix& Transpose();

    // Inspired by C#.
    std::string ToString();

  private:
    size_t rows_, cols_;
    // Array storing matrix in row major order
    float* data_;

    // Hiding any copy construction behavior.
    Matrix(const Matrix& matrix);
    Matrix& operator=(const Matrix& matrix);
};

