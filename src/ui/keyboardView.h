#ifndef KEYBOARD_VIEW_H
#define KEYBOARD_VIEW_H


#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include <unordered_map>
#include <vector>


class KeyboardView {
    public:
        void BuildABNT2Layout();
        void MarkHit(const std::string& ch);
        void MarkMiss(const std::string& ch);

        void Render(SDL_Renderer* r, TTF_Font* font, const SDL_Rect& area);

    private:
        struct Key { std::string label; SDL_Rect rect{}; int state=0; /*0=idle,1=ok,2=bad*/ };
        std::vector<std::vector<Key>> m_rows; // linhas do teclado
        std::unordered_map<std::string, SDL_Point> m_pos; // char -> (row,col)

        void LayoutRectangles(const SDL_Rect& area);
};


#endif