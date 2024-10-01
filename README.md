# Arcade Machine with Tetris

## Description

This project is an arcade machine built using the TTGO T-Display S3, featuring a boot menu that allows users to select and launch different games, including Tetris. The interface is displayed on a TFT screen, and user input is handled through physical buttons.

## Features

- Boot menu to navigate through multiple games.
- Tetris game with a classic gameplay experience.
- Simple and intuitive user interface.
- Customizable button mapping.

## Components Used

- **TTGO T-Display S3**: A microcontroller with a built-in TFT display.
- **Buttons**: Used for user input (navigation and game controls).
- **TFT_eSPI library**: For handling the display.
- **Tetris game logic**: Custom game implementation.

## Project Structure

```
/BootMenu
    ├── BootMenu.ino       # Main boot menu code
    ├── Tetris.h           # Header file for Tetris game
    ├── Tetris.ino         # Tetris game implementation
```

## Getting Started

### Requirements

- Arduino IDE
- TTGO T-Display S3 board support
- TFT_eSPI library

### Usage

- Use the navigation buttons to select games from the boot menu.
- Press the A button to launch the selected game.
- In Tetris, use the navigation buttons to control the game pieces.

### Contributing

Feel free to submit issues, fork the repository, or create pull requests to contribute to the project!

### License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Thanks to the open-source community for libraries and resources.
- Thanks to VolosR @ [TTGOTetris](https://github.com/VolosR/TTGOTetris) for the TTGO code for Tetris.
- Special thanks to the creators of Tetris for the original game concept.
