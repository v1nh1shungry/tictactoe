#include "pch.h"

#define CMD_GROUP_NAME (_T("PLAYGROUND_COMMANDS"))

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

constexpr double BOX_LENGTH = 30;

class Chessboard : public AcDbEntity {
public:
    enum class Box { Empty, Circle, Cross };
 
public:
    ACRX_DECLARE_MEMBERS(Chessboard);

    Chessboard() {
        std::fill(m_board.begin(), m_board.end(), Box::Empty);
    }

    Box winner() const {
        for (int i = 0; i < 3; ++i) {
            if (m_board[i * 3] == m_board[i * 3 + 1] and m_board[i * 3] == m_board[i * 3 + 2] and m_board[i * 3] != Box::Empty)
                return m_board[i * 3];
        }
        for (int i = 0; i < 3; ++i) {
            if (m_board[i] != Box::Empty and m_board[i] == m_board[i + 3] and m_board[i] == m_board[i + 6])
                return m_board[i];
        }
        if (m_board[0] != Box::Empty and m_board[0] == m_board[4] and m_board[0] == m_board[8])
            return m_board[0];
        if (m_board[2] != Box::Empty and m_board[2] == m_board[4] and m_board[2] == m_board[6])
            return m_board[2];
        return Box::Empty;
    }

    bool full() const {
        return std::all_of(m_board.begin(), m_board.end(), [](Box b) { return b != Box::Empty; });
    }

    void robot(Box sign) {
        unsigned int Smartmove = 0;
        bool found = false;
        Box anti_sign = sign == Box::Circle ? Box::Cross : Box::Circle;

        while (!found && Smartmove < 9)
        {
            if (m_board[Smartmove] == Box::Empty)
            {
                m_board[Smartmove] = sign;
                found = winner() == sign;
                m_board[Smartmove] = Box::Empty;
            }
            if (!found)
            {
                ++Smartmove;
            }
        }

        if (!found)
        {
            Smartmove = 0;
            while (!found && Smartmove < 9)
            {
                if (m_board[Smartmove] == Box::Empty)
                {
                    m_board[Smartmove] = anti_sign;
                    found = winner() == anti_sign;
                    m_board[Smartmove] = Box::Empty;
                }
                if (!found)
                {
                    ++Smartmove;
                }
            }
        }

        if (!found)
        {
            Smartmove = 0;
            unsigned int i = 0;
            const int BEST_MOVES[] = { 0, 4, 8, 2, 6, 1, 3, 5, 7 };
            while (!found && i < 9)
            {
                Smartmove = BEST_MOVES[i];
                if (m_board[Smartmove] == Box::Empty)
                {
                    found = true;
                }
                ++i;
            }
        }
        m_board[Smartmove] = sign;
    }

protected:
    static Adesk::UInt32 kCurrentVersionNumber;

protected:
    Adesk::Boolean subWorldDraw(AcGiWorldDraw* mode) override {
        assertReadEnabled();

        AcGePoint3d points[2];
        points[0] = { 0, 0, 0 };
        points[1] = { 3 * BOX_LENGTH, 0, 0 };
        for (int i = 0; i <= 3; ++i) {
            mode->geometry().polyline(2, points);
            points[0] += {0, BOX_LENGTH, 0};
            points[1] += {0, BOX_LENGTH, 0};
        }
        points[0] = { 0, 0, 0 };
        points[1] = { 0, 3 * BOX_LENGTH, 0 };
        for (int i = 0; i <= 3; ++i) {
            mode->geometry().polyline(2, points);
            points[0] += {BOX_LENGTH, 0, 0};
            points[1] += {BOX_LENGTH, 0, 0};
        }

        for (int i = 0; i < 9; ++i) {
            int r = i / 3, c = i % 3;
            switch (m_board[i]) {
            case Box::Circle:
                mode->geometry().circle(
                    AcGePoint3d{ r * BOX_LENGTH + BOX_LENGTH / 2, c * BOX_LENGTH + BOX_LENGTH / 2, 0 },
                    BOX_LENGTH / 3,
                    AcGeVector3d::kZAxis
                );
                break;
            case Box::Cross:
                points[0] = { r * BOX_LENGTH, c * BOX_LENGTH, 0 };
                points[1] = { (r + 1) * BOX_LENGTH, (c + 1) * BOX_LENGTH, 0 };
                mode->geometry().polyline(2, points);
                points[0] = { (r + 1) * BOX_LENGTH, c * BOX_LENGTH, 0 };
                points[1] = { r * BOX_LENGTH, (c + 1) * BOX_LENGTH, 0 };
                mode->geometry().polyline(2, points);
                break;
            }
        }

        return AcDbEntity::subWorldDraw(mode);
    }

public:
    std::array<Box, 9> m_board{};
};

#ifdef ARXPROJECT1_MODULE
ACDB_REGISTER_OBJECT_ENTRY_AUTO(SampleCustEnt)
#endif

ACRX_DXF_DEFINE_MEMBERS(
    Chessboard, AcDbEntity, 
    AcDb::kDHL_CURRENT, AcDb::kMReleaseCurrent, 
    AcDbProxyEntity::kNoOperation, Chessboard,
    ARXPROJECT1APP
    |Product Desc: TicTacToe chessboard
    |Company: SCUT
    |WEB Address: https://github.com/v1nh1shungry
)

template <class T>
constexpr bool between(T value, T min, T max) {
    return min <= value and value <= max;
}

class TicTacToe : public AcEdJig {
private:
    Chessboard* m_entity = new Chessboard;
    int m_row = -1;
    int m_col = -1;
    int m_prev_row = -1;
    int m_prev_col = -1;
    enum class Turn { Circle, Cross } m_turn = Turn::Circle;

public:
    DragStatus sampler() override {
        DragStatus status;
        AcGePoint3d point;
        status = acquirePoint(point);
        if (status == AcEdJig::kNormal) {
            if (not between(point.x, 0.0, 3 * BOX_LENGTH) or not between(point.y, 0.0, 3 * BOX_LENGTH))
                return AcEdJig::kNoChange;
            int r = static_cast<int>(point.x / BOX_LENGTH);
            int c = static_cast<int>(point.y / BOX_LENGTH);
            if (m_entity->m_board[r * 3 + c] != Chessboard::Box::Empty)
                return AcEdJig::kNoChange;
            m_prev_row = m_row;
            m_prev_col = m_col;
            m_row = r;
            m_col = c;
        }
        return status;
    }

    Adesk::Boolean update() override {
        if (between(m_prev_row, 0, 2) and between(m_prev_col, 0, 2))
            m_entity->m_board[m_prev_row * 3 + m_prev_col] = Chessboard::Box::Empty;
        if (between(m_row, 0, 2) and between(m_col, 0, 2))
            m_entity->m_board[m_row * 3 + m_col] = m_turn == Turn::Circle ? Chessboard::Box::Circle : Chessboard::Box::Cross;
        return Adesk::kTrue;
    }

    virtual AcDbEntity* entity() const { return m_entity; }

    void play() {
        bool full = false;
        Chessboard::Box winner = Chessboard::Box::Empty;
        while ((winner = m_entity->winner()) == Chessboard::Box::Empty and not (full = m_entity->full())) {
            if (m_turn == Turn::Circle) {
                drag();
                m_prev_row = m_prev_col = m_row = m_col = -1;
            }
            else {
                m_entity->robot(m_turn == Turn::Circle ? Chessboard::Box::Circle : Chessboard::Box::Cross);
            }
            m_turn = m_turn == Turn::Circle ? Turn::Cross : Turn::Circle;
        }
        switch (winner) {
        case Chessboard::Box::Circle:
            acutPrintf(_T("Circle win"));
            break;
        case Chessboard::Box::Cross:
            acutPrintf(_T("Cross win"));
            break;
        default:
            acutPrintf(_T("Tie"));
            break;
        }
        append();
    }
};

void homework() {
    auto* game = new TicTacToe;
    game->play();
    delete game;
}

extern "C" AcRx::AppRetCode zcrxEntryPoint(AcRx::AppMsgCode msg, void* appId) {
    switch (msg) {
    case AcRx::kInitAppMsg:
        acrxDynamicLinker->unlockApplication(appId);
        acrxDynamicLinker->registerAppMDIAware(appId);
        Chessboard::rxInit();
        acrxBuildClassHierarchy();
        acedRegCmds->addCommand(CMD_GROUP_NAME, _T("TicTacToe"), _T("TicTacToe"), ACRX_CMD_MODAL, homework);
        break;
    case AcRx::kUnloadAppMsg:
        deleteAcRxClass(Chessboard::desc());
        acedRegCmds->removeGroup(CMD_GROUP_NAME);
        break;
    default:
        break;
    }
    return AcRx::kRetOK;
}

#ifdef _WIN64
#pragma comment(linker, "/export:zcrxEntryPoint,PRIVATE")
#pragma comment(linker, "/export:zcrxGetApiVersion,PRIVATE")
#else // WIN32
#pragma comment(linker, "/export:_zcrxEntryPoint,PRIVATE")
#pragma comment(linker, "/export:_zcrxGetApiVersion,PRIVATE")
#endif