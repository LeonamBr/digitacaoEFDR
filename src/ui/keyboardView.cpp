#include "KeyboardView.h"
#include <algorithm>


static SDL_Texture* RenderText(SDL_Renderer* r, TTF_Font* f, const std::string& s, SDL_Color c)
{
    SDL_Surface* surf = TTF_RenderUTF8_Blended(f, s.c_str(), c);
    SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);
    SDL_FreeSurface(surf);
    return tex;
}


void KeyboardView::BuildABNT2Layout()
{
// Layout simplificado (ajuste conforme necessidade)
// Usaremos apenas letras + espaço para o starter
const char* row1 = "q w e r t y u i o p";
const char* row2 = "a s d f g h j k l ç"; // ABNT2 traz ç
const char* row3 = "z x c v b n m";


    auto make = [&](const char* row){
        std::vector<Key> out; std::string acc;
        for (const char* p=row; *p; ++p) {
            if (*p==' ') {
                if (!acc.empty()) { out.push_back({acc}); acc.clear(); }
            } else { acc.push_back(*p); }
        }
        if (!acc.empty()) out.push_back({acc});
        return out;
    };


    m_rows.clear();
    m_rows.push_back(make(row1));
    m_rows.push_back(make(row2));
    m_rows.push_back(make(row3));


    // espaço (barra de espaço) numa linha própria mais larga
    m_rows.push_back({ Key{" "} });


    // mapa de posição
    m_pos.clear();
    for (int r=0; r<(int)m_rows.size(); ++r)
    for (int c=0; c<(int)m_rows[r].size(); ++c)
    m_pos[m_rows[r][c].label] = {r,c};
}

void KeyboardView::MarkHit(const std::string& ch)
{
auto it = m_pos.find(ch);
if (it!=m_pos.end()) m_rows[it->second.x][it->second.y].state = 1;
}

void KeyboardView::MarkMiss(const std::string& ch)
{
auto it = m_pos.find(ch);
if (it!=m_pos.end()) m_rows[it->second.x][it->second.y].state = 2;
}

void KeyboardView::LayoutRectangles(const SDL_Rect& area)
{
    int rows = (int)m_rows.size();
    int gap = 6;
    int rowH = (area.h - gap*(rows+1)) / rows;
    int y = area.y + gap;
    for (int r=0; r<rows; ++r) {
        int cols = (int)m_rows[r].size();
        int extra = (r==rows-1? 3:0); // barra de espaço mais larga
        int totalCols = cols + extra;
        int colW = (area.w - gap*(totalCols+1)) / totalCols;
        int x = area.x + gap;
        for (int c=0; c<cols; ++c) {
            int w = (r==rows-1 && c==0)? colW*4 + gap*3 : colW; // espaço = ~4 teclas
            m_rows[r][c].rect = SDL_Rect{ x, y, w, rowH };
            x += w + gap;
        }
        y += rowH + gap;
    }
}

void KeyboardView::Render(SDL_Renderer* r, TTF_Font* font, const SDL_Rect& area)
{
    LayoutRectangles(area);
    SDL_Color idle{40,40,48,255}, ok{50,180,90,255}, bad{220,70,70,255}, txt{230,230,230,255};

    for (auto& row : m_rows) {
        for (auto& key : row) {
            SDL_Color c = idle;
            if (key.state==1) c = ok; else if (key.state==2) c = bad;
            SDL_SetRenderDrawColor(r, c.r,c.g,c.b,c.a);
            SDL_RenderFillRect(r, &key.rect);
            SDL_SetRenderDrawColor(r, 20,20,24,255);
            SDL_RenderDrawRect(r, &key.rect);


            SDL_Texture* t = RenderText(r, font, key.label, txt);
            int w,h; SDL_QueryTexture(t,nullptr,nullptr,&w,&h);
            SDL_Rect tr{ key.rect.x + (key.rect.w - w)/2, key.rect.y + (key.rect.h - h)/2, w, h };
            SDL_RenderCopy(r, t, nullptr, &tr);
            SDL_DestroyTexture(t);

            // decaimento visual simples: volta para idle depois de desenhar
            if (key.state!=0) key.state = 0;
        }
    }
}