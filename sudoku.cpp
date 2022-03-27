#include <iostream>
#include <cassert>
#include <memory>
#include <string>
#include <map>

typedef unsigned int uint;

template <uint BASE>
struct Sudoku
{
    static constexpr uint N = BASE * BASE;

    static constexpr uint NUM_TYPES = 4;
    static constexpr uint T_CELL = 0;
    static constexpr uint T_ROW = 1;
    static constexpr uint T_COL = 2;
    static constexpr uint T_SQR = 3;

    struct IdxOpt
    {
        uint idx;
        uint opt;
    };

    struct Maps
    {
        IdxOpt cellToRowOpt[N * N];
        IdxOpt cellToColOpt[N * N];
        IdxOpt cellToBoxOpt[N * N];

        uint rowOptToCell[N][N];
        uint colOptToCell[N][N];
        uint boxOptToCell[N][N];
    };

    static Maps constructMaps()
    {
        Maps maps;

        for (uint cell = 0; cell < N * N; ++cell)
        {
            uint row = cell / N;
            uint col = cell % N;
            uint box = row / BASE * BASE + col / BASE;
            uint boxOpt = row % BASE * BASE + col % BASE;

            maps.cellToRowOpt[cell] = {row, col};
            maps.cellToColOpt[cell] = {col, row};
            maps.cellToBoxOpt[cell] = {box, boxOpt};

            maps.rowOptToCell[row][col] = cell;
            maps.colOptToCell[col][row] = cell;
            maps.boxOptToCell[box][boxOpt] = cell;
        }

        return maps;
    }

    static const Maps maps;
    
    struct Options
    {
        uint numOpts;
        bool isOpt[N];

        Options()
        {
            numOpts = N;
            std::fill(isOpt, isOpt + N, true);
        }

        Options(const Options&) = default;
        Options& operator=(const Options&) = default;
        bool operator==(const Options&) const = default;
        bool operator!=(const Options&) const = default;

        void merge(const Options& other)
        {
            numOpts = 0;
            for (uint opt = 0; opt < N; ++opt)
            {
                isOpt[opt] = isOpt[opt] || other.isOpt[opt];
                numOpts += isOpt[opt];
            }
        }

        void remove(uint opt)
        {
            assert(isOpt[opt]);
            assert(numOpts > 1);

            --numOpts;
            isOpt[opt] = false;
        }

        uint get() const
        {
            assert(numOpts == 1);

            for (uint opt = 0; opt < N; ++opt)
            {
                if (isOpt[opt]) return opt;
            }

            assert(false);

            return N;
        }
    };

    Options cells[N * N];
    Options rows[N][N];
    Options cols[N][N];
    Options boxs[N][N];

    uint numSet;
    bool isSet[N * N];

    bool impossible;

    Sudoku()
    {
        numSet = 0;
        impossible = false;

        for (uint cell = 0; cell < N * N; ++cell)
        {
            isSet[cell] = cells[cell].numOpts == 1;
            numSet += isSet[cell];
        }
    }

    void merge(const Sudoku& other)
    {
        if (other.impossible) return;
        if (impossible) *this = other;

        numSet = 0;

        for (uint i = 0; i < N * N; ++i)
        {
            uint i1 = i / N;
            uint i2 = i % N;

            cells[i].merge(other.cells[i]);
            rows[i1][i2].merge(other.rows[i1][i2]);
            cols[i1][i2].merge(other.cols[i1][i2]);
            boxs[i1][i2].merge(other.boxs[i1][i2]);

            isSet[i] = cells[i].numOpts == 1;
            numSet += isSet[i];
        }
    }

    Sudoku(const Sudoku&) = default;
    Sudoku& operator=(const Sudoku&) = default;
    bool operator==(const Sudoku&) const = default;
    bool operator!=(const Sudoku&) const = default;

    void remove(uint cell, uint val)
    {
        assert(cell < N * N);
        assert(val < N);

        if (impossible) return;
        assert(cells[cell].isOpt[val]);

        uint row = maps.cellToRowOpt[cell].idx;
        uint col = maps.cellToColOpt[cell].idx;
        uint box = maps.cellToBoxOpt[cell].idx;

        uint rowOpt = maps.cellToRowOpt[cell].opt;
        uint colOpt = maps.cellToColOpt[cell].opt;
        uint boxOpt = maps.cellToBoxOpt[cell].opt;

        assert(cells[cell].isOpt[val]);
        assert(rows[row][val].isOpt[rowOpt]);
        assert(cols[col][val].isOpt[colOpt]);
        assert(boxs[box][val].isOpt[boxOpt]);

        if (cells[cell].numOpts == 1) impossible = true;
        if (rows[row][val].numOpts == 1) impossible = true;
        if (cols[col][val].numOpts == 1) impossible = true;
        if (boxs[box][val].numOpts == 1) impossible = true;
        if (impossible) return;

        cells[cell].remove(val);
        rows[row][val].remove(rowOpt);
        cols[col][val].remove(colOpt);
        boxs[box][val].remove(boxOpt);

        if (cells[cell].numOpts == 1) set(cell, cells[cell].get());
        if (rows[row][val].numOpts == 1) set(maps.rowOptToCell[row][rows[row][val].get()], val);
        if (cols[col][val].numOpts == 1) set(maps.colOptToCell[col][cols[col][val].get()], val);
        if (boxs[box][val].numOpts == 1) set(maps.boxOptToCell[box][boxs[box][val].get()], val);
    }

    void set(uint cell, uint val)
    {
        assert(cell < N * N);
        assert(val < N);

        if (impossible) return;
        assert(cells[cell].isOpt[val]);
        if (isSet[cell]) return;

        ++numSet;
        isSet[cell] = true;

        uint row = maps.cellToRowOpt[cell].idx;
        uint col = maps.cellToColOpt[cell].idx;
        uint box = maps.cellToBoxOpt[cell].idx;

        uint rowOpt = maps.cellToRowOpt[cell].opt;
        uint colOpt = maps.cellToColOpt[cell].opt;
        uint boxOpt = maps.cellToBoxOpt[cell].opt;

        for (uint opt = 0; opt < N; ++opt)
        {
            if (opt != val && cells[cell].isOpt[opt]) remove(cell, opt);
            if (opt != rowOpt && rows[row][val].isOpt[opt]) remove(maps.rowOptToCell[row][opt], val);
            if (opt != colOpt && cols[col][val].isOpt[opt]) remove(maps.colOptToCell[col][opt], val);
            if (opt != boxOpt && boxs[box][val].isOpt[opt]) remove(maps.boxOptToCell[box][opt], val);
        }
    }

    Options& typeIdxToOpts(uint typeIdx)
    {
        uint idx = typeIdx % (N * N);
        uint type = typeIdx / (N * N);
        uint idx1 = idx / N;
        uint idx2 = idx % N;

        switch (type)
        {
        case T_CELL:
            return cells[idx];

        case T_ROW:
            return rows[idx1][idx2];

        case T_COL:
            return cols[idx1][idx2];

        case T_SQR:
            return boxs[idx1][idx2];
        
        default:
            assert(false);
        }
    }

    IdxOpt typeIdxOptToCellVal(uint typeIdx, uint opt)
    {
        uint idx = typeIdx % (N * N);
        uint type = typeIdx / (N * N);
        uint idx1 = idx / N;
        uint idx2 = idx % N;

        switch (type)
        {
        case T_CELL:
            return {idx, opt};

        case T_ROW:
            return {maps.rowOptToCell[idx1][opt], idx2};

        case T_COL:
            return {maps.colOptToCell[idx1][opt], idx2};

        case T_SQR:
            return {maps.boxOptToCell[idx1][opt], idx2};

        default:
            assert(false);
        }
    }

    bool caseAnalysis(uint maxWidth)
    {
        if (maxWidth == 1 || impossible || numSet == N * N) return false;

        bool changed = false;

        for (uint typeIdx = 0; typeIdx < NUM_TYPES * N * N; ++typeIdx)
        {
            Options& opts = typeIdxToOpts(typeIdx);

            if (opts.numOpts == 1 || opts.numOpts > maxWidth) continue;

            Sudoku merged;
            merged.impossible = true;

            for (uint opt = 0; opt < N; ++opt)
            {
                if (!opts.isOpt[opt]) continue;

                IdxOpt cellVal = typeIdxOptToCellVal(typeIdx, opt);

                Sudoku next(*this);
                next.set(cellVal.idx, cellVal.opt);

                if (!next.impossible && next.numSet == N * N)
                {
                    *this = next;
                    return true;
                }

                merged.merge(next);
            }

            if (merged != *this)
            {
                *this = merged;
                changed = true;
            }
        }

        return changed;
    }

    void bruteforce()
    {
        if (impossible || numSet == N * N) return;

        uint bestTypeIdx = NUM_TYPES * N * N;
        uint minOpts = N + 1;

        for (uint typeIdx = 0; typeIdx < NUM_TYPES * N * N; ++typeIdx)
        {
            Options& opts = typeIdxToOpts(typeIdx);

            if (opts.numOpts > 1 && opts.numOpts < minOpts)
            {
                bestTypeIdx = typeIdx;
                minOpts = opts.numOpts;
            }
        }

        assert(bestTypeIdx < NUM_TYPES * N * N);

        Options& bestOpts = typeIdxToOpts(bestTypeIdx);

        std::unique_ptr<Sudoku> nextPtr = std::make_unique<Sudoku>();
        Sudoku& next = *nextPtr;

        for (uint opt = 0; opt < N; ++opt)
        {
            if (!bestOpts.isOpt[opt]) continue;

            IdxOpt cellVal = typeIdxOptToCellVal(bestTypeIdx, opt);

            std::cout << '+';

            next = *this;
            next.set(cellVal.idx, cellVal.opt);
            next.solve();

            std::cout << '-';

            if (!next.impossible && next.numSet == N * N)
            {
                *this = next;
                return;
            }
        }

        impossible = true;
    }

    void solve(uint maxWidth = 3)
    {
        assert(maxWidth >= 1);

        while (caseAnalysis(maxWidth))
        {
            std::cout << ".";
        }
        std::cout << "!";

        bruteforce();
    }

    void maybeSet(uint cell, uint val)
    {
        if (!cells[cell].isOpt[val]) impossible = true;
        set(cell, val);
    }

    void print(std::ostream& out) const
    {
        if (impossible)
        {
            out << "Impossible\n\n";
            return;
        }

        for (uint cell = 0; cell < N * N; ++cell)
        {
            uint row = cell / N;
            uint col = cell % N;

            if (col > 0) out << " ";
            if (col > 0 && col % BASE == 0) out << " ";

            if (col == 0 && row > 0) out << "\n";
            if (col == 0 && row > 0 && row % BASE == 0) out << "\n";

            if (cells[cell].numOpts == 1) out << cells[cell].get() + 1;
            else out << ".";
        }

        out << "\n\n";
    }

    static uint readVal(std::istream& in, std::map<char, uint>& charMap, bool fromChars)
    {
        if (!fromChars)
        {
            std::string s;
            in >> s;
            if (s == ".") return 0;
            return std::stoi(s);
        }
        else
        {
            char c;
            do
            {
                in.get(c);
            }
            while (c == '\n' || c == '\r');
            if (c == ' ' || c == '.') return 0;
            charMap.insert({c, charMap.size() + 1});
            std::cerr << c << " : " << charMap[c] << "\n";
            return charMap[c];
        }
    }

    void input(std::istream& in, bool fromChars = false)
    {
        *this = Sudoku();
        std::map<char, uint> charMap;
        for (uint cell = 0; cell < N * N; ++cell)
        {
            uint val = readVal(in, charMap, fromChars);
            if (val == 0) continue;
            maybeSet(cell, val - 1);
        }
    }
};

template <uint N>
const typename Sudoku<N>::Maps Sudoku<N>::maps = constructMaps();

int main()
{
    Sudoku<3> orig;
    orig.input(std::cin, false);

    orig.print(std::cout);

    orig.solve(3);
    std::cout << "\n\n";
    
    orig.print(std::cout);

    return 0;
}
