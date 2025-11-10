// MazeLoader.cpp
#include "MazeLoader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

static int doorLabelToIndex(const std::string& t) {
    return (t.size()==2 && t[0]=='D' && t[1]>='1' && t[1]<='5') ? (t[1]-'1') : -1;
}

Maze loadMazeCSV(const std::string& path) {
    std::ifstream in(path);
    if(!in) throw std::runtime_error("No pude abrir " + path);

    std::vector<std::vector<std::string>> rows;
    std::string line;
    while (std::getline(in, line)) {
        std::stringstream ss(line);
        std::string tok; std::vector<std::string> row;
        while (std::getline(ss, tok, ',')) {
            while(!tok.empty() && (tok.back()=='\r' || tok.back()==' ')) tok.pop_back();
            while(!tok.empty() && tok.front()==' ') tok.erase(tok.begin());
            row.push_back(tok);
        }
        if (!row.empty()) rows.push_back(row);
    }
    if (rows.empty()) throw std::runtime_error("CSV vacío");

    Maze mz;
    mz.h = (int)rows.size();
    mz.w = (int)rows[0].size();
    mz.cells.resize(mz.w*mz.h);

    bool haveS=false, haveE=false;
    for (int r=0; r<mz.h; ++r) {
        if ((int)rows[r].size()!=mz.w) throw std::runtime_error("Filas con distinto ancho");
        for (int c=0; c<mz.w; ++c) {
            auto& cell = mz.at(r,c);
            const std::string& T = rows[r][c];
            if (T == "#") cell.type = TileType::Wall;
            else if (T == "." || T.empty()) cell.type = TileType::Floor;
            else if (T == "S") { cell.type = TileType::Start; mz.sr=r; mz.sc=c; haveS=true; }
            else if (T == "E") { cell.type = TileType::Exit;  mz.er=r; mz.ec=c; haveE=true; }
            else {
                int di = doorLabelToIndex(T);
                if (di>=0) { cell.type = TileType::Door; cell.doorIndex = di; }
                else cell.type = TileType::Floor;
            }
        }
    }
    if (!haveS || !haveE) throw std::runtime_error("Falta S o E en el CSV");
    return mz;
}
