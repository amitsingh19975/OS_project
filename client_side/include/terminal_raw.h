#ifndef TERMINAL_RAW_H
#define TERMINAL_RAW_H
#include <iostream>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#define CTRL_KEY(k) ((k) & 0x1f)
std::string CLEARSCREEN  = "\x1b[2J";
// std::string CLEARSCREEN  = "\x1b[K";

enum editorKey{
    ARROW_LEFT      = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    DEL_KEY,
    HOME_KEY,
    END_KEY,
    PAGE_UP,
    PAGE_DOWN     
};

class Terminal{
    public:
        inline static void init();
        inline static void disable();
        inline static void die(std::string str);
        inline static int keyEvent();
        inline static void clearScreen(size_t x = 0, size_t y = 0);
        inline static std::pair<int,int> getWindowSize();
        inline static std::pair<int,int> getCursorPosition();
        inline static void getCursorPosition(int& x, int& y);
        inline static void setCursor(int x, int y);

    private:
        inline static termios origonalState;
};

int Terminal::keyEvent(){
    int nRead;
    char c;
    while((nRead = read(0,&(c),1)) != 1){
        if(nRead == -1 && errno != EAGAIN) Terminal::die("Unable to Read!");
    }

    if (c == '\x1b') {
    char seq[3];
    if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
    if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';
    if (seq[0] == '[') {
      if (seq[1] >= '0' && seq[1] <= '9') {
        if (read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';
        if (seq[2] == '~') {
          switch (seq[1]) {
            case '1': return HOME_KEY;
            case '3': return DEL_KEY;
            case '4': return END_KEY;
            case '5': return PAGE_UP;
            case '6': return PAGE_DOWN;
            case '7': return HOME_KEY;
            case '8': return END_KEY;
          }
        }
      } else {
        switch (seq[1]) {
          case 'A': return ARROW_UP;
          case 'B': return ARROW_DOWN;
          case 'C': return ARROW_RIGHT;
          case 'D': return ARROW_LEFT;
          case 'H': return HOME_KEY;
          case 'F': return END_KEY;
        }
      }
    } else if (seq[0] == 'O') {
      switch (seq[1]) {
        case 'H': return HOME_KEY;
        case 'F': return END_KEY;
      }
    }
    return '\x1b';
  } else {
    return c;
  }

}

void Terminal::disable(){
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &(Terminal::origonalState)) == -1) Terminal::die("tcsetattr");
}

void Terminal::init(){
    if(tcgetattr(STDIN_FILENO, &Terminal::origonalState) == -1) Terminal::die("tcgetattr");
    atexit(disable);
    termios raw = Terminal::origonalState;

    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) Terminal::die("tcgetattr");

}

void Terminal::die(std::string s) {
    Terminal::clearScreen();
    perror(s.c_str());
    exit(1);
}

void Terminal::clearScreen(size_t x , size_t y){
    std::string str = CLEARSCREEN;
    
    if(x || y) {
        std::string temp = std::string("[") + std::to_string(x) + std::string(";") + std::to_string(y) + std::string("H");
        str += CLEARSCREEN + std::string("\x1b") + temp;
    }else{
        str.append("\x1b[H");
    }
    write(STDOUT_FILENO, str.c_str(), str.size());
}

std::pair<int,int> Terminal::getWindowSize(){
    winsize w;
    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1){
        Terminal::die("Unable To Get Size Of Window!");
        return {-1,-1};
    }else{
        return {w.ws_row, w.ws_col};
    }
}

std::pair<int,int> Terminal::getCursorPosition(){
    char buf[32];
    uint32_t i = 0;
    int x{0},y{0};
    if(write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return {-1,-1};

    while(i < sizeof(buf) - 1){
        if (read(STDIN_FILENO, &buf[i], 1) != 1) {
            break;
        } if(buf[i] == 'R') {
            break;
        }
        i++;
    }
    buf[i] = '\0';

    if(buf[0] != '\x1b' || buf[1] != '[') return {x,y};
    if(sscanf(&buf[2], "%d;%d", &(y), &(x)) != 2) return {x,y};
    return {x,y};
}

void Terminal::setCursor(int x, int y){
    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", y, x);
    write(STDOUT_FILENO,buf,strlen(buf));
}
void Terminal::getCursorPosition(int &x, int &y){
    auto [X,Y] = Terminal::getCursorPosition();
    x = X;
    y = Y;
}

#endif
