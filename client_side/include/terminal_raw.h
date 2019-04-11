#ifndef TERMINAL_RAW_H
#define TERMINAL_RAW_H
#include <iostream>
#include <cstdlib>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#define CTRL_KEY(k) ((k) & 0x1f)

namespace terminal{
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


    struct Terminal{

        auto die(std::string_view) const noexcept;
        auto keyEvent() const noexcept -> int;
        auto init() noexcept;
        auto setCursor(int x, int y)  const noexcept;
        auto getCursorPosition(int &x, int &y) const noexcept;
        auto clearScreen(size_t x = 0, size_t y = 0) const noexcept;
        auto getWindowSize() const noexcept;
        auto getCursorPosition() const noexcept;
        auto wScreen(std::string_view, int x = 0, int y = 0);

        inline static termios origonalState;
    };


    void disable() noexcept{
        if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &(Terminal::origonalState)) == -1){
            std::cerr<<"tcsetattr";
            exit(1);
        }
    }


    auto Terminal::clearScreen(size_t x , size_t y)  const noexcept{
        std::string str = CLEARSCREEN;
        
        if(x || y) {
            std::string temp = std::string("[") + std::to_string(x) + std::string(";") + std::to_string(y) + std::string("H");
            str += CLEARSCREEN + std::string("\x1b") + temp;
        }else{
            str.append("\x1b[H");
        }
        write(STDOUT_FILENO, str.c_str(), str.size());
    }
    
    auto Terminal::die(std::string_view s) const noexcept{
        clearScreen();
        std::cerr<<s;
        exit(1);
    }


    auto Terminal::keyEvent() const noexcept -> int{
        int nRead = -1;
        char c = 0;
        while((nRead = read(0,&(c),1)) != 1){
            if(nRead == -1 && errno != EAGAIN) die("Unable to Read!");
        }

        if (c == '\x1b') {
        char seq[3]={0};
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

    auto  Terminal::init() noexcept{
        if(tcgetattr(STDIN_FILENO, &Terminal::origonalState) == -1) die("tcgetattr");
        std::atexit(disable);
        termios raw = Terminal::origonalState;

        raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
        raw.c_oflag &= ~(OPOST);
        raw.c_cflag |= (CS8);
        raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = 1;

        if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcgetattr");

    }

    auto Terminal::getWindowSize() const noexcept{
        winsize w;
        if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1){
            die("Unable To Get Size Of Window!");
            return std::make_pair(ushort(-1),ushort(-1));
        }else{
            return std::make_pair(w.ws_row, w.ws_col);
        }
    }

    auto Terminal::getCursorPosition() const noexcept{
        char buf[32];
        uint32_t i = 0;
        int x{0},y{0};
        if(write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return std::make_pair(-1,-1);

        while(i < sizeof(buf) - 1){
            if (read(STDIN_FILENO, &buf[i], 1) != 1) {
                break;
            } if(buf[i] == 'R') {
                break;
            }
            i++;
        }
        buf[i] = '\0';

        if(buf[0] != '\x1b' || buf[1] != '[') return std::make_pair(x,y);
        if(sscanf(&buf[2], "%d;%d", &(y), &(x)) != 2) return std::make_pair(x,y);
        return std::make_pair(x,y);
    }

    auto Terminal::setCursor(int x, int y)  const noexcept{
        char buf[32];
        snprintf(buf, sizeof(buf), "\x1b[%d;%dH", y, x);
        write(STDOUT_FILENO,buf,strlen(buf));
    }
    auto  Terminal::getCursorPosition(int &x, int &y)  const noexcept{
        auto [X,Y] = this->getCursorPosition();
        x = X;
        y = Y;
    }

    auto Terminal::wScreen(std::string_view str, int x, int y){
        std::string text(str);
        if(x != y != 0){
            setCursor(x,y);
            text = "\x1b["+ std::to_string(y) +";"+ std::to_string(x) + "H" + text;
        }
        write(1,text.c_str(),text.size());
    }
}

#endif
