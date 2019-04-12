#ifndef COLOR_H
#define COLOR_H

#include <iostream>
#include <initializer_list>

namespace chat_utility{
    
    enum class FG : uint16_t{
        BLACK           = 30,
        RED,
        GREEN,
        YELLOW,
        BLUE,
        MAGENTA,
        CYAN,
        WHITE,
        BRIGHT_BLACK    = 90,
        BRIGHT_RED,
        BRIGHT_GREEN,
        BRIGHT_YELLOW,
        BRIGHT_BLUE,
        BRIGHT_MAGENTA,
        BRIGHT_CYAN,
        BRIGHT_WHITE,
        NONE = 39,
    };
    enum class BG : uint16_t{
        BLACK           = 40,
        RED,
        GREEN,
        YELLOW,
        BLUE,
        MAGENTA,
        CYAN,
        WHITE,
        BRIGHT_BLACK    = 100,
        BRIGHT_RED,
        BRIGHT_GREEN,
        BRIGHT_YELLOW,
        BRIGHT_BLUE,
        BRIGHT_MAGENTA,
        BRIGHT_CYAN,
        BRIGHT_WHITE,
        NONE = 49,
    };

    enum class TF : u_char{
        BOLD = 1,
        FAINT,
        ITALIC,
        UNDERLINE,
        NONE
    };

    template < typename TextRGB, typename BackgroundRGB>
    struct FormatRGB;

    template < u_short r, u_short g, u_short b >
    struct RGB{
        constexpr RGB() = default;

        template < typename TextRGB, typename BackgroundRGB>
        friend struct FormatRGB;

        auto to_string() const noexcept{
            return std::string(std::to_string(m_r) + 
                ";" + std::to_string(m_g) + ";" + std::to_string(m_b));
        }

    private:
        u_short m_r{r};
        u_short m_g{g};
        u_short m_b{b};
    };

    template < typename TextRGB, typename BackgroundRGB = RGB<500,500,500> >
    struct FormatRGB{
        constexpr FormatRGB() = default;
        auto to_string() const noexcept{
            RGB<500,500,500> temp;
            if (m_bg.m_r == temp.m_r && m_bg.m_g == temp.m_g && m_bg.m_b == temp.m_b){
                return "38;2;" + m_fg.to_string();
            }else{
                return "38;2;" + m_fg.to_string() + ";48;2;" 
                    + m_bg.to_string();
            }
        }
    private:
        TextRGB         m_fg;
        BackgroundRGB   m_bg;
    };
    
    template < FG TextColour, BG BackgroundColour = BG::NONE> 
    struct Bit_3_4{
        constexpr Bit_3_4() = default;

        constexpr Bit_3_4(FG text_color, BG background): m_fg(text_color),m_bg(background){}
        constexpr Bit_3_4(FG text_color): m_fg(text_color){}
        constexpr Bit_3_4(BG background): m_bg(background){}

        auto to_string() const noexcept{
            if (m_bg == BG::NONE){
                return std::to_string(static_cast<uint16_t>(m_fg));
            }else if(m_fg == FG::NONE){
                return std::to_string(static_cast<uint16_t>(m_bg));
            }else{
                return std::to_string(static_cast<uint16_t>(m_fg)) + ";" 
                    + std::to_string(static_cast<uint16_t>(m_bg));
            }
        }
    private:
        FG m_fg{TextColour};
        BG m_bg{BackgroundColour};
    };

    template < u_short TextColour, u_short BackgroundColour = 500> 
    struct Bit_8{
        constexpr Bit_8(u_char f_bit, u_char b_bit):m_f_bit(f_bit), m_b_bit(b_bit){}
        constexpr Bit_8(u_char b_bit):m_b_bit(b_bit){}
        auto to_string() const noexcept{
            if (m_b_bit == 500){
                return "38;5" + std::to_string(m_f_bit);
            }else if(m_f_bit == 0){
                return "48;5;" + std::to_string(m_b_bit);
            }else{
                return "38;5;" + std::string(std::to_string(m_f_bit) + ";48;5;" 
                    + std::to_string(m_b_bit));
            }
        }
    private:
        u_short m_f_bit{0};
        u_short m_b_bit{0};
    };



    template < typename Colour, TF ...TextFormat>
    auto format(std::string_view text, Colour c){
        std::initializer_list<TF> li {TextFormat...};
        std::string temp= "\x1b[";
        size_t i = 0;
        for(auto const& l : li){
            temp += std::to_string(static_cast<u_char>(l)) + ";";
        }
        return temp + c.to_string() + "m" + std::string(text) + temp + "\x1b[0m";
    }

    template < typename Colour, TF ...TextFormat>
    auto format(std::string_view text){
        return format<Colour,TextFormat...>(text,Colour{});
    }

}

#endif // COLOR_H
