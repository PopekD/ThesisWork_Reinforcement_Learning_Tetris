# Bachelor's Thesis Implementation

[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

Implementation for my bachelor's thesis - [https://urn.fi/URN:NBN:fi:amk-2024061022571](https://urn.fi/URN:NBN:fi:amk-2024061022571)

This repository contains the implementation of a Deep Mind's Deep Q-Network with a twist of using prioritized replay memory over the original replay memory. The implementation includes a custom Tetris environment and a Deep Q-Network using Libtorch.

## Overview

The work done here is an implementation of a Deep Q-Network with a twist of making prioritized replay memory over an original replay memory. The project involves creating a custom Tetris environment and a Deep Q-Network using Libtorch. All the details are explained in the paper linked above.

## Dependencies

The following external libraries were used for this project:

- [SFML](https://www.sfml-dev.org/) - Simple and Fast Multimedia Library
- [Libtorch](https://pytorch.org/cppdocs/) - C++ library for PyTorch
- [OpenCV](https://opencv.org/) - Open Source Computer Vision Library

The project was developed using C++20.

## Usage

To run the project, follow these steps:

1. Clone the repository
2. Install the required dependencies (SFML, Libtorch, OpenCV).
5. Build the project using your preferred C++ build system.
6. Run the compiled executable.

To enable free play mode, uncomment the commented lines in the `main.cpp` file, comment the uncommented ones, and change the value to `1` in the `Player.h` file.

## Contributing

Contributions are welcome! If you find any issues or have suggestions for improvements, please open an issue or submit a pull request.

## License

This project is licensed under the [MIT License](LICENSE).
