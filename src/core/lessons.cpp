#include "core/Lessons.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cctype>

namespace Lessons {

static std::vector<Lesson> g_cache;
static bool g_loaded = false;

static std::string read_file_utf8(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return {};
    std::ostringstream oss;
    oss << f.rdbuf();
    return oss.str();
}

// Lê uma string JSON a partir de pos (apontando para a aspa inicial).
// Suporta escapes \" \\ \n \r \t. \uXXXX é repassado “como veio”.
static bool parse_json_string(const std::string& s, size_t& pos, std::string& out) {
    if (pos >= s.size() || s[pos] != '"') return false;
    ++pos; // pula a aspa
    std::string r;
    while (pos < s.size()) {
        char c = s[pos++];
        if (c == '"') { out = r; return true; }
        if (c == '\\') {
            if (pos >= s.size()) return false;
            char e = s[pos++];
            switch (e) {
                case '"': r.push_back('"'); break;
                case '\\': r.push_back('\\'); break;
                case '/': r.push_back('/'); break;
                case 'b': r.push_back('\b'); break;
                case 'f': r.push_back('\f'); break;
                case 'n': r.push_back('\n'); break;
                case 'r': r.push_back('\r'); break;
                case 't': r.push_back('\t'); break;
                case 'u':
                    // Mantém literal \uXXXX (não convertemos aqui)
                    r.push_back('\\'); r.push_back('u');
                    for (int i=0;i<4 && pos<s.size();++i) r.push_back(s[pos++]);
                    break;
                default:  r.push_back(e); break;
            }
        } else {
            r.push_back(c);
        }
    }
    return false; // string não terminou
}

static void skip_ws(const std::string& s, size_t& i) {
    while (i < s.size() && std::isspace((unsigned char)s[i])) ++i;
}

// Lê um campo string:  "field" : "value"
// Retorna true se conseguiu extrair.
static bool extract_string_field(const std::string& obj, const char* field, std::string& value) {
    size_t i = 0;
    while (true) {
        size_t keypos = obj.find('"', i);
        if (keypos == std::string::npos) return false;
        size_t save = keypos;
        std::string key;
        if (!parse_json_string(obj, keypos, key)) { i = save + 1; continue; }

        skip_ws(obj, keypos);
        if (keypos >= obj.size() || obj[keypos] != ':') { i = save + 1; continue; }
        ++keypos;
        skip_ws(obj, keypos);

        if (key == field) {
            if (keypos < obj.size() && obj[keypos] == '"') {
                return parse_json_string(obj, keypos, value);
            }
            // Não-string — consideramos inválido para esse campo
            return false;
        }

        // pula o valor (bruto) desse outro campo para seguir procurando
        // (bem superficial: tenta avançar por string, {obj}, [array] ou valor primitivo)
        if (keypos >= obj.size()) { i = save + 1; continue; }
        if (obj[keypos] == '"') {
            std::string dummy; parse_json_string(obj, keypos, dummy);
        } else if (obj[keypos] == '{') {
            int depth = 1; ++keypos;
            while (keypos < obj.size() && depth > 0) {
                if (obj[keypos] == '"') { std::string dummy; parse_json_string(obj, keypos, dummy); }
                else if (obj[keypos] == '{') { ++depth; ++keypos; }
                else if (obj[keypos] == '}') { --depth; ++keypos; }
                else ++keypos;
            }
        } else if (obj[keypos] == '[') {
            int depth = 1; ++keypos;
            while (keypos < obj.size() && depth > 0) {
                if (obj[keypos] == '"') { std::string dummy; parse_json_string(obj, keypos, dummy); }
                else if (obj[keypos] == '[') { ++depth; ++keypos; }
                else if (obj[keypos] == ']') { --depth; ++keypos; }
                else ++keypos;
            }
        } else {
            // valor simples: avança até vírgula/fecha_chave
            while (keypos < obj.size() && obj[keypos] != ',' && obj[keypos] != '}') ++keypos;
        }
        i = keypos;
    }
}

static std::vector<Lesson> fallback_lessons() {
    return {
        { "Home row (a s d f j k l ç)", "Meu nome é leonam dos Santos braga" },
        { "Vogais", "a e i o u aeiou" },
        { "Maiúsculas", "A Aa aA B bB" },
        { "Palavras simples", "ola ola ola joao joao" },
        { "Pontuação", ".,;:/? !" }
    };
}

static void load_from_json_if_needed() {
    if (g_loaded) return;
    g_loaded = true;

    const std::string path = "resources/lessons.json";
    const std::string js = read_file_utf8(path);
    if (js.empty()) {
        g_cache = fallback_lessons();
        return;
    }

    // Procura pelo array "lessons": [...]
    size_t p = js.find("\"lessons\"");
    if (p == std::string::npos) p = js.find("\"Lessons\"");
    if (p == std::string::npos) {
        g_cache = fallback_lessons();
        return;
    }

    size_t bracket = js.find('[', p);
    if (bracket == std::string::npos) { g_cache = fallback_lessons(); return; }

    // Varre objetos { ... } dentro do array
    size_t i = bracket+1;
    while (i < js.size()) {
        // pula WS e vírgulas
        while (i < js.size() && (std::isspace((unsigned char)js[i]) || js[i] == ',')) ++i;
        if (i >= js.size() || js[i] == ']') break;
        if (js[i] != '{') { ++i; continue; }

        int depth = 1;
        size_t obj_start = i++;
        while (i < js.size() && depth > 0) {
            if (js[i] == '"') { std::string dummy; parse_json_string(js, i, dummy); }
            else if (js[i] == '{') { ++depth; ++i; }
            else if (js[i] == '}') { --depth; ++i; }
            else ++i;
        }
        size_t obj_end = i;
        if (obj_end <= obj_start) break;

        std::string obj = js.substr(obj_start, obj_end - obj_start);
        std::string title, text, alt;

        if (!extract_string_field(obj, "title", title))
            extract_string_field(obj, "Title", title);

        // aceita vários nomes para o conteúdo
        if (!extract_string_field(obj, "text", text)) {
            if (!extract_string_field(obj, "target", text) &&
                !extract_string_field(obj, "sequence", text) &&
                !extract_string_field(obj, "content", text))
            {
                text.clear();
            }
        }

        if (!text.empty()) {
            if (title.empty()) title = "Sem titulo";
            g_cache.push_back({title, text});
        }
    }

    if (g_cache.empty()) {
        g_cache = fallback_lessons();
    }
}

const std::vector<Lesson>& All() {
    load_from_json_if_needed();
    return g_cache;
}

void Reload() {
    g_loaded = false;
    g_cache.clear();
    load_from_json_if_needed();
}

} // namespace Lessons
