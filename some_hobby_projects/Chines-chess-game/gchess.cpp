/*
	Chinese chess game implemented in C++11 with a lot of template techniques.
*/
#include <iostream>
#include <array>
#include <stack>
#include <vector>
#include <algorithm>
#include <numeric>
#include <string>
#include <regex>
#include <future>
#include <chrono>
#include <utility>
#include <type_traits>

namespace g_chess {
	enum class Side {
		UP, DOWN, EXTRA
	};

	enum class Type {
		PAWN, CANNON, ROOK, KNIGHT, BISHOP, ADVISOR, GENERAL, EMPTY, OUT
	};

	enum class Piece {
		UP, UC, UR, UN, UB, UA, UG,
		DP, DC, DR, DN, DB, DA, DG,
		EE, EO
	};

	namespace p_util {
		constexpr Side sideMap[] = {
			Side::UP, Side::UP, Side::UP, Side::UP, Side::UP, Side::UP, Side::UP,
			Side::DOWN, Side::DOWN, Side::DOWN, Side::DOWN, Side::DOWN, Side::DOWN, Side::DOWN,
			Side::EXTRA, Side::EXTRA
		};

		// In C++11, we can't use if in constexpr functions, so I have to use this.
		constexpr Side reverseSideMap[] = {
			Side::DOWN, Side::UP, Side::EXTRA
		};

		constexpr Type typeMap[] = {
			Type::PAWN, Type::CANNON, Type::ROOK, Type::KNIGHT, Type::BISHOP, Type::ADVISOR, Type::GENERAL,
			Type::PAWN, Type::CANNON, Type::ROOK, Type::KNIGHT, Type::BISHOP, Type::ADVISOR, Type::GENERAL,
			Type::EMPTY, Type::OUT
		};

		constexpr char charMap[] = {
			'S', 'C', 'R', 'N', 'B', 'A', 'G',
			's', 'c', 'r', 'n', 'b', 'a', 'g',
			'.', '#'
		};

		constexpr uint32_t pieceToInt32(Piece p) noexcept { return static_cast<uint32_t>(p); }

		inline constexpr Side getSide(Piece p) noexcept { return sideMap[pieceToInt32(p)]; }
		inline constexpr Type getType(Piece p) noexcept { return typeMap[pieceToInt32(p)]; }
		inline constexpr char getChar(Piece p) noexcept { return charMap[pieceToInt32(p)]; }
		inline constexpr Side getReverseSide(Side s) noexcept { return reverseSideMap[static_cast<uint32_t>(s)]; }
	};

	struct Pos {
		uint32_t row, col;

		Pos() = default;
		Pos(uint32_t _row, uint32_t _col) : row(_row), col(_col) {}
		bool operator==(const Pos& other) const noexcept { return row == other.row && col == other.col; }
		bool operator!=(const Pos& other) const noexcept { return !(*this == other); }
	};

	struct Move {
		Pos from, to;

		Move() = default;
		Move(const Pos& _from, const Pos& _to) : from(_from), to(_to) {}
		bool operator==(const Move& other) const noexcept { return from == other.from && to == other.to; }
		bool operator!=(const Move& other) const noexcept { return !(*this == other); }
	};

	using Moves = std::vector<Move>;

	struct HistoryNode {
		Pos from, to;
		Piece fromP, toP;

		HistoryNode() = default;
		HistoryNode(const Pos& _from, const Pos& _to, const Piece& _fromP, const Piece& _toP)
			: from(_from), to(_to), fromP(_fromP), toP(_toP)
		{}
	};

	class Board {
	public:
		constexpr static uint32_t COL_NUM = 9;
		constexpr static uint32_t ROW_NUM = 10;

		constexpr static uint32_t SINGLE_COL_PADDING = 2;
		constexpr static uint32_t SINGLE_ROW_PADDING = 2;

		constexpr static uint32_t ACTUAL_COL_NUM = COL_NUM + 2 * SINGLE_COL_PADDING;
		constexpr static uint32_t ACTUAL_ROW_NUM = ROW_NUM + 2 * SINGLE_ROW_PADDING;

		constexpr static uint32_t ROW_BEGIN = SINGLE_ROW_PADDING;
		constexpr static uint32_t COL_BEGIN = SINGLE_COL_PADDING;
		constexpr static uint32_t ROW_END = SINGLE_ROW_PADDING + ROW_NUM;
		constexpr static uint32_t COL_END = SINGLE_COL_PADDING + COL_NUM;

		constexpr static uint32_t LINE_UP_PAWN = ROW_BEGIN + 4;
		constexpr static uint32_t LINE_DOWN_PAWN = ROW_BEGIN + 5;

		constexpr static uint32_t LINE_UP_9_TOP = ROW_BEGIN;
		constexpr static uint32_t LINE_UP_9_BOTTOM = ROW_BEGIN + 2;
		constexpr static uint32_t LINE_UP_9_LEFT = COL_BEGIN + 3;
		constexpr static uint32_t LINE_UP_9_RIGHT = COL_BEGIN + 5;

		constexpr static uint32_t LINE_DOWN_9_TOP = ROW_END - 2;
		constexpr static uint32_t LINE_DOWN_9_BOTTOM = ROW_END;
		constexpr static uint32_t LINE_DOWN_9_LEFT = COL_BEGIN + 3;
		constexpr static uint32_t LINE_DOWN_9_RIGHT = COL_BEGIN + 5;
	private:
		std::array<std::array<Piece, ACTUAL_COL_NUM>, ACTUAL_ROW_NUM> data;
		std::stack<HistoryNode> history;
	private:
		void set(const Pos& pos, Piece p) {
			data[pos.row][pos.col] = p;
		}
	public:
		Board() :
			history{},
			data{
				Piece::EO,Piece::EO, Piece::EO, Piece::EO, Piece::EO, Piece::EO, Piece::EO, Piece::EO, Piece::EO, Piece::EO, Piece::EO, Piece::EO,Piece::EO,
				Piece::EO,Piece::EO, Piece::EO, Piece::EO, Piece::EO, Piece::EO, Piece::EO, Piece::EO, Piece::EO, Piece::EO, Piece::EO, Piece::EO,Piece::EO,
				Piece::EO,Piece::EO, Piece::UR, Piece::UN, Piece::UB, Piece::UA, Piece::UG, Piece::UA, Piece::UB, Piece::UN, Piece::UR, Piece::EO,Piece::EO,
				Piece::EO,Piece::EO, Piece::EE, Piece::EE, Piece::EE, Piece::EE, Piece::EE, Piece::EE, Piece::EE, Piece::EE, Piece::EE, Piece::EO,Piece::EO,
				Piece::EO,Piece::EO, Piece::EE, Piece::UC, Piece::EE, Piece::EE, Piece::EE, Piece::EE, Piece::EE, Piece::UC, Piece::EE, Piece::EO,Piece::EO,
				Piece::EO,Piece::EO, Piece::UP, Piece::EE, Piece::UP, Piece::EE, Piece::UP, Piece::EE, Piece::UP, Piece::EE, Piece::UP, Piece::EO,Piece::EO,
				Piece::EO,Piece::EO, Piece::EE, Piece::EE, Piece::EE, Piece::EE, Piece::EE, Piece::EE, Piece::EE, Piece::EE, Piece::EE, Piece::EO,Piece::EO,
				Piece::EO,Piece::EO, Piece::EE, Piece::EE, Piece::EE, Piece::EE, Piece::EE, Piece::EE, Piece::EE, Piece::EE, Piece::EE, Piece::EO,Piece::EO,
				Piece::EO,Piece::EO, Piece::DP, Piece::EE, Piece::DP, Piece::EE, Piece::DP, Piece::EE, Piece::DP, Piece::EE, Piece::DP, Piece::EO,Piece::EO,
				Piece::EO,Piece::EO, Piece::EE, Piece::DC, Piece::EE, Piece::EE, Piece::EE, Piece::EE, Piece::EE, Piece::DC, Piece::EE, Piece::EO,Piece::EO,
				Piece::EO,Piece::EO, Piece::EE, Piece::EE, Piece::EE, Piece::EE, Piece::EE, Piece::EE, Piece::EE, Piece::EE, Piece::EE, Piece::EO,Piece::EO,
				Piece::EO,Piece::EO, Piece::DR, Piece::DN, Piece::DB, Piece::DA, Piece::DG, Piece::DA, Piece::DB, Piece::DN, Piece::DR, Piece::EO,Piece::EO,
				Piece::EO,Piece::EO, Piece::EO, Piece::EO, Piece::EO, Piece::EO, Piece::EO, Piece::EO, Piece::EO, Piece::EO, Piece::EO, Piece::EO,Piece::EO,
				Piece::EO,Piece::EO, Piece::EO, Piece::EO, Piece::EO, Piece::EO, Piece::EO, Piece::EO, Piece::EO, Piece::EO, Piece::EO, Piece::EO,Piece::EO,
			}
		{}

		Piece get(uint32_t row, uint32_t col) const {
			return data[row][col];
		}

		Piece get(const Pos& pos) const {
			return get(pos.row, pos.col);
		}

		void move(const Move& m) {
			history.emplace(m.from, m.to, get(m.from), get(m.to));

			set(m.to, get(m.from));
			set(m.from, Piece::EE);
		}

		void undo() {
			if (history.empty()) {
				return;
			}

			const auto& historyNode = history.top();

			set(historyNode.from, historyNode.fromP);
			set(historyNode.to, historyNode.toP);
			history.pop();
		}
	};

	void printBoard(const Board& bd) {
		Piece p;
		uint32_t n = Board::ROW_NUM - 1;

		std::cout << "\n";
		for (uint32_t r = Board::ROW_BEGIN; r < Board::ROW_END; ++r) {
			if (r == (Board::ROW_BEGIN + Board::ROW_END) / 2) {
				std::cout << "  | =================\n";
				std::cout << "  | =================\n";
			}

			std::cout << n-- << " | ";
			for (uint32_t c = Board::COL_BEGIN; c < Board::COL_END; ++c) {
				p = bd.get(r, c);
				std::cout << p_util::getChar(p) << " ";
			}

			std::cout << "\n";
		}

		std::cout << "  +------------------\n";
		std::cout << "\n    a b c d e f g h i\n\n";
	}

	namespace gen_moves {
		template<Type T>
		void addMoveOf(Side side, const Board& bd, Moves& moves, const Pos& from, const Pos& to) {}

		template<Type T>
		void addMoveOf(Side side, const Board& bd, Moves& moves, const Pos& from, int32_t rowVariation, int32_t colVariation) {}

		template<Type T>
		void addMoveOf(Side side, const Board& bd, Moves& moves, const Pos& from, const Pos& to, const Pos& middle) {}

		template<>
		void addMoveOf<Type::PAWN>(Side side, const Board& bd, Moves& moves, const Pos& from, const Pos& to) {
			Piece p = bd.get(to);

			if (p != Piece::EO && p_util::getSide(p) != side) {
				moves.emplace_back(from, to);
			}
		}

		template<>
		void addMoveOf<Type::CANNON>(Side side, const Board& bd, Moves& moves, const Pos& from, int32_t rowVariation, int32_t colVariation) {
			uint32_t newRow = from.row + rowVariation;
			uint32_t newCol = from.col + colVariation;
			Piece p = bd.get(newRow, newCol);

			while (p == Piece::EE) {
				moves.emplace_back(from, Pos{ newRow, newCol });

				newRow += rowVariation;
				newCol += colVariation;
				p = bd.get(newRow, newCol);
			}

			if (p != Piece::EO) {
				newRow += rowVariation;
				newCol += colVariation;
				p = bd.get(newRow, newCol);

				while (p != Piece::EO) {
					Side ps = p_util::getSide(p);

					if (ps == p_util::getReverseSide(side)) {
						moves.emplace_back(from, Pos{ newRow, newCol });
						return;
					}
					else if (ps == side) {
						return;
					}

					newRow += rowVariation;
					newCol += colVariation;
					p = bd.get(newRow, newCol);
				}
			}
		}

		template<>
		void addMoveOf<Type::ROOK>(Side side, const Board& bd, Moves& moves, const Pos& from, int32_t rowVariation, int32_t colVariation) {
			uint32_t newRow = from.row + rowVariation;
			uint32_t newCol = from.col + colVariation;
			Piece p = bd.get(newRow, newCol);

			while (p == Piece::EE) {
				moves.emplace_back(from, Pos{ newRow, newCol });

				newRow += rowVariation;
				newCol += colVariation;
				p = bd.get(newRow, newCol);
			}

			if (p_util::getSide(p) == p_util::getReverseSide(side)) {
				moves.emplace_back(from, Pos{ newRow, newCol });
			}
		}

		template<>
		void addMoveOf<Type::KNIGHT>(Side side, const Board& bd, Moves& moves, const Pos& from, const Pos& to, const Pos& middle) {
			Piece middleP = bd.get(middle);
			Piece toP = bd.get(to);

			if (middleP == Piece::EE && toP != Piece::EO && p_util::getSide(toP) != side) {
				moves.emplace_back(from, to);
			}
		}

		template<>
		void addMoveOf<Type::BISHOP>(Side side, const Board& bd, Moves& moves, const Pos& from, const Pos& to, const Pos& middle) {
			addMoveOf<Type::KNIGHT>(side, bd, moves, from, to, middle);
		}

		template<>
		void addMoveOf<Type::ADVISOR>(Side side, const Board& bd, Moves& moves, const Pos& from, const Pos& to) {
			addMoveOf<Type::PAWN>(side, bd, moves, from, to);
		}

		template<>
		void addMoveOf<Type::GENERAL>(Side side, const Board& bd, Moves& moves, const Pos& from, const Pos& to) {
			addMoveOf<Type::PAWN>(side, bd, moves, from, to);
		}

		template<Piece P, typename ... Args>
		inline void addMove(Args&& ... args) {
			addMoveOf<p_util::getType(P)>(p_util::getSide(P), std::forward<Args>(args)...);
		}

		template<Piece P>
		void genMovesOf(const Board& bd, const Pos& pos, Moves& moves) {}

		template<>
		void genMovesOf<Piece::UP>(const Board& bd, const Pos& pos, Moves& moves) {
			if (pos.row <= Board::LINE_UP_PAWN) {
				addMove<Piece::UP>(bd, moves, pos, Pos{ pos.row + 1, pos.col });
			}
			else {
				addMove<Piece::UP>(bd, moves, pos, Pos{ pos.row + 1, pos.col });
				addMove<Piece::UP>(bd, moves, pos, Pos{ pos.row, pos.col - 1 });
				addMove<Piece::UP>(bd, moves, pos, Pos{ pos.row, pos.col + 1 });
			}
		}

		template<>
		void genMovesOf<Piece::DP>(const Board& bd, const Pos& pos, Moves& moves) {
			if (pos.row >= Board::LINE_DOWN_PAWN) {
				addMove<Piece::DP>(bd, moves, pos, Pos{ pos.row - 1, pos.col });
			}
			else {
				addMove<Piece::DP>(bd, moves, pos, Pos{ pos.row - 1, pos.col });
				addMove<Piece::DP>(bd, moves, pos, Pos{ pos.row, pos.col - 1 });
				addMove<Piece::DP>(bd, moves, pos, Pos{ pos.row, pos.col + 1 });
			}
		}

		template<>
		void genMovesOf<Piece::UC>(const Board& bd, const Pos& pos, Moves& moves) {
			addMove<Piece::UC>(bd, moves, pos, +1, 0);
			addMove<Piece::UC>(bd, moves, pos, -1, 0);
			addMove<Piece::UC>(bd, moves, pos, 0, +1);
			addMove<Piece::UC>(bd, moves, pos, 0, -1);
		}

		template<>
		void genMovesOf<Piece::DC>(const Board& bd, const Pos& pos, Moves& moves) {
			addMove<Piece::DC>(bd, moves, pos, +1, 0);
			addMove<Piece::DC>(bd, moves, pos, -1, 0);
			addMove<Piece::DC>(bd, moves, pos, 0, +1);
			addMove<Piece::DC>(bd, moves, pos, 0, -1);
		}

		template<>
		void genMovesOf<Piece::UR>(const Board& bd, const Pos& pos, Moves& moves) {
			addMove<Piece::UR>(bd, moves, pos, +1, 0);
			addMove<Piece::UR>(bd, moves, pos, -1, 0);
			addMove<Piece::UR>(bd, moves, pos, 0, +1);
			addMove<Piece::UR>(bd, moves, pos, 0, -1);
		}

		template<>
		void genMovesOf<Piece::DR>(const Board& bd, const Pos& pos, Moves& moves) {
			addMove<Piece::DR>(bd, moves, pos, +1, 0);
			addMove<Piece::DR>(bd, moves, pos, -1, 0);
			addMove<Piece::DR>(bd, moves, pos, 0, +1);
			addMove<Piece::DR>(bd, moves, pos, 0, -1);
		}

		template<>
		void genMovesOf<Piece::UN>(const Board& bd, const Pos& pos, Moves& moves) {
			addMove<Piece::UN>(bd, moves, pos, Pos{ pos.row + 2, pos.col + 1 }, Pos{ pos.row + 1, pos.col });
			addMove<Piece::UN>(bd, moves, pos, Pos{ pos.row + 2, pos.col - 1 }, Pos{ pos.row + 1, pos.col });
			addMove<Piece::UN>(bd, moves, pos, Pos{ pos.row - 2, pos.col + 1 }, Pos{ pos.row - 1, pos.col });
			addMove<Piece::UN>(bd, moves, pos, Pos{ pos.row - 2, pos.col - 1 }, Pos{ pos.row - 1, pos.col });
			addMove<Piece::UN>(bd, moves, pos, Pos{ pos.row + 1, pos.col + 2 }, Pos{ pos.row, pos.col + 1 });
			addMove<Piece::UN>(bd, moves, pos, Pos{ pos.row + 1, pos.col - 2 }, Pos{ pos.row, pos.col - 1 });
			addMove<Piece::UN>(bd, moves, pos, Pos{ pos.row - 1, pos.col + 2 }, Pos{ pos.row, pos.col + 1 });
			addMove<Piece::UN>(bd, moves, pos, Pos{ pos.row - 1, pos.col - 2 }, Pos{ pos.row, pos.col - 1 });
		}

		template<>
		void genMovesOf<Piece::DN>(const Board& bd, const Pos& pos, Moves& moves) {
			addMove<Piece::DN>(bd, moves, pos, Pos{ pos.row + 2, pos.col + 1 }, Pos{ pos.row + 1, pos.col });
			addMove<Piece::DN>(bd, moves, pos, Pos{ pos.row + 2, pos.col - 1 }, Pos{ pos.row + 1, pos.col });
			addMove<Piece::DN>(bd, moves, pos, Pos{ pos.row - 2, pos.col + 1 }, Pos{ pos.row - 1, pos.col });
			addMove<Piece::DN>(bd, moves, pos, Pos{ pos.row - 2, pos.col - 1 }, Pos{ pos.row - 1, pos.col });
			addMove<Piece::DN>(bd, moves, pos, Pos{ pos.row + 1, pos.col + 2 }, Pos{ pos.row, pos.col + 1 });
			addMove<Piece::DN>(bd, moves, pos, Pos{ pos.row + 1, pos.col - 2 }, Pos{ pos.row, pos.col - 1 });
			addMove<Piece::DN>(bd, moves, pos, Pos{ pos.row - 1, pos.col + 2 }, Pos{ pos.row, pos.col + 1 });
			addMove<Piece::DN>(bd, moves, pos, Pos{ pos.row - 1, pos.col - 2 }, Pos{ pos.row, pos.col - 1 });
		}

		template<>
		void genMovesOf<Piece::UB>(const Board& bd, const Pos& pos, Moves& moves) {
			if (pos.row + 2 <= Board::LINE_UP_PAWN) {
				addMove<Piece::UB>(bd, moves, pos, Pos{ pos.row + 2, pos.col + 2 }, Pos{ pos.row + 1, pos.col + 1 });
				addMove<Piece::UB>(bd, moves, pos, Pos{ pos.row + 2, pos.col - 2 }, Pos{ pos.row + 1, pos.col - 1 });
			}

			addMove<Piece::UB>(bd, moves, pos, Pos{ pos.row - 2, pos.col + 2 }, Pos{ pos.row - 1, pos.col + 1 });
			addMove<Piece::UB>(bd, moves, pos, Pos{ pos.row - 2, pos.col - 2 }, Pos{ pos.row - 1, pos.col - 1 });
		}

		template<>
		void genMovesOf<Piece::DB>(const Board& bd, const Pos& pos, Moves& moves) {
			addMove<Piece::DB>(bd, moves, pos, Pos{ pos.row + 2, pos.col + 2 }, Pos{ pos.row + 1, pos.col + 1 });
			addMove<Piece::DB>(bd, moves, pos, Pos{ pos.row + 2, pos.col - 2 }, Pos{ pos.row + 1, pos.col - 1 });

			if (pos.row - 2 >= Board::LINE_DOWN_PAWN) {
				addMove<Piece::DB>(bd, moves, pos, Pos{ pos.row - 2, pos.col + 2 }, Pos{ pos.row - 1, pos.col + 1 });
				addMove<Piece::DB>(bd, moves, pos, Pos{ pos.row - 2, pos.col - 2 }, Pos{ pos.row - 1, pos.col - 1 });
			}
		}

		template<>
		void genMovesOf<Piece::UA>(const Board& bd, const Pos& pos, Moves& moves) {
			if (pos.row + 1 <= Board::LINE_UP_9_BOTTOM && pos.col + 1 <= Board::LINE_UP_9_RIGHT) {
				addMove<Piece::UA>(bd, moves, pos, Pos{ pos.row + 1, pos.col + 1 });
			}

			if (pos.row + 1 <= Board::LINE_UP_9_BOTTOM && pos.col - 1 >= Board::LINE_UP_9_LEFT) {
				addMove<Piece::UA>(bd, moves, pos, Pos{ pos.row + 1, pos.col - 1 });
			}

			if (pos.row - 1 >= Board::LINE_UP_9_TOP && pos.col + 1 <= Board::LINE_UP_9_RIGHT) {
				addMove<Piece::UA>(bd, moves, pos, Pos{ pos.row - 1, pos.col + 1 });
			}

			if (pos.row - 1 >= Board::LINE_UP_9_TOP && pos.col - 1 >= Board::LINE_UP_9_LEFT) {
				addMove<Piece::UA>(bd, moves, pos, Pos{ pos.row - 1, pos.col - 1 });
			}
		}

		template<>
		void genMovesOf<Piece::DA>(const Board& bd, const Pos& pos, Moves& moves) {
			if (pos.row + 1 <= Board::LINE_DOWN_9_BOTTOM && pos.col + 1 <= Board::LINE_DOWN_9_RIGHT) {
				addMove<Piece::DA>(bd, moves, pos, Pos{ pos.row + 1, pos.col + 1 });
			}

			if (pos.row + 1 <= Board::LINE_DOWN_9_BOTTOM && pos.col - 1 >= Board::LINE_DOWN_9_LEFT) {
				addMove<Piece::DA>(bd, moves, pos, Pos{ pos.row + 1, pos.col - 1 });
			}

			if (pos.row - 1 >= Board::LINE_DOWN_9_TOP && pos.col + 1 <= Board::LINE_DOWN_9_RIGHT) {
				addMove<Piece::DA>(bd, moves, pos, Pos{ pos.row - 1, pos.col + 1 });
			}

			if (pos.row - 1 >= Board::LINE_DOWN_9_TOP && pos.col - 1 >= Board::LINE_DOWN_9_LEFT) {
				addMove<Piece::DA>(bd, moves, pos, Pos{ pos.row - 1, pos.col - 1 });
			}
		}

		template<>
		void genMovesOf<Piece::UG>(const Board& bd, const Pos& pos, Moves& moves) {
			if (pos.row + 1 <= Board::LINE_UP_9_BOTTOM) {
				addMove<Piece::UG>(bd, moves, pos, Pos{ pos.row + 1, pos.col });
			}

			if (pos.row - 1 >= Board::LINE_UP_9_TOP) {
				addMove<Piece::UG>(bd, moves, pos, Pos{ pos.row - 1, pos.col });
			}

			if (pos.col + 1 <= Board::LINE_UP_9_RIGHT) {
				addMove<Piece::UG>(bd, moves, pos, Pos{ pos.row, pos.col + 1 });
			}

			if (pos.col - 1 >= Board::LINE_UP_9_LEFT) {
				addMove<Piece::UG>(bd, moves, pos, Pos{ pos.row, pos.col - 1 });
			}

			auto currentPos = Pos{ pos.row + 1, pos.col };
			auto p = bd.get(currentPos);
			
			while (p == Piece::EE) {
				++currentPos.row;
				p = bd.get(currentPos);
			}

			if (p == Piece::DG) {
				moves.emplace_back(pos, currentPos);
			}
		}

		template<>
		void genMovesOf<Piece::DG>(const Board& bd, const Pos& pos, Moves& moves) {
			if (pos.row + 1 <= Board::LINE_DOWN_9_BOTTOM) {
				addMove<Piece::DG>(bd, moves, pos, Pos{ pos.row + 1, pos.col });
			}

			if (pos.row - 1 >= Board::LINE_DOWN_9_TOP) {
				addMove<Piece::DG>(bd, moves, pos, Pos{ pos.row - 1, pos.col });
			}

			if (pos.col + 1 <= Board::LINE_DOWN_9_RIGHT) {
				addMove<Piece::DG>(bd, moves, pos, Pos{ pos.row, pos.col + 1 });
			}

			if (pos.col - 1 >= Board::LINE_DOWN_9_LEFT) {
				addMove<Piece::DG>(bd, moves, pos, Pos{ pos.row, pos.col - 1 });
			}

			auto currentPos = Pos{ pos.row - 1, pos.col };
			auto p = bd.get(currentPos);

			while (p == Piece::EE) {
				--currentPos.row;
				p = bd.get(currentPos);
			}

			if (p == Piece::UG) {
				moves.emplace_back(pos, currentPos);
			}
		}

		using GenMovesMethod = void(*)(const Board& bd, const Pos& pos, Moves& moves);

		constexpr static GenMovesMethod methods[] = {
			&genMovesOf<Piece::UP>,
			&genMovesOf<Piece::UC>,
			&genMovesOf<Piece::UR>,
			&genMovesOf<Piece::UN>,
			&genMovesOf<Piece::UB>,
			&genMovesOf<Piece::UA>,
			&genMovesOf<Piece::UG>,
			&genMovesOf<Piece::DP>,
			&genMovesOf<Piece::DC>,
			&genMovesOf<Piece::DR>,
			&genMovesOf<Piece::DN>,
			&genMovesOf<Piece::DB>,
			&genMovesOf<Piece::DA>,
			&genMovesOf<Piece::DG>,
		};
	};

	inline void genMoves(const Board& bd, const Pos& pos, Moves& moves) {
		Piece p = bd.get(pos);
		gen_moves::methods[p_util::pieceToInt32(p)](bd, pos, moves);
	}

	template<Side side, typename = typename std::enable_if<side == Side::UP || side == Side::DOWN, bool>::type>
	void genMoves(const Board& bd, Moves& moves) {
		Piece p{};

		for (uint32_t r = Board::ROW_BEGIN; r < Board::ROW_END; ++r) {
			for (uint32_t c = Board::COL_BEGIN; c < Board::COL_END; ++c) {
				p = bd.get(r, c);
				if (p_util::getSide(p) == side) {
					gen_moves::methods[p_util::pieceToInt32(p)](bd, Pos{ r, c }, moves);
				}
			}
		}
	}

	bool isValidMove(const Board& bd, const Move& m) {
		Moves moves;
		genMoves(bd, m.from, moves);

		return std::find(moves.cbegin(), moves.cend(), m) != moves.cend();
	}

	namespace value {
		constexpr int32_t pieceValueMap[] = {
			-20, -50, -100, -50, -10, -10, -10000,
			+20, +50, +100, +50, +10, +10, +10000,
			  0,   0
		};

		constexpr int32_t posValueMap[][Board::ROW_NUM][Board::COL_NUM] = {
			// UP
			{
				{   0,   0,   0,   0,   0,   0,   0,   0,   0 },
				{   0,   0,   0,   0,   0,   0,   0,   0,   0 },
				{   0,   0,   0,   0,   0,   0,   0,   0,   0 },
				{   2,   0,   2,   0,  -6,   0,   2,   0,   2 },
				{  -3,   0,  -4,   0,  -7,   0,  -4,   0,  -3 },
				{ -10, -18, -22, -35, -40, -35, -22, -18, -10 },
				{ -20, -27, -30, -40, -42, -40, -30, -27, -20 },
				{ -20, -30, -45, -55, -55, -55, -45, -30, -20 },
				{ -20, -30, -50, -65, -70, -65, -50, -30, -20 },
				{   0,   0,   0,  -2,  -4,  -2,   0,   0,   0 }
			},
			// UC
			{
				{  0,  0, -1, -3, -3, -3, -1,  0,  0 },
				{  0, -1, -2, -2, -2, -2, -2, -1,  0 },
				{ -1,  0, -4, -3, -5, -3, -4,  0, -1 },
				{  0,  0,  0,  0,  0,  0,  0,  0,  0 },
				{  1,  0, -3,  0, -4,  0, -3,  0,  1 },
				{  0,  0,  0,  0, -4,  0,  0,  0,  0 },
				{  0, -3, -3, -2, -4, -2, -3, -3,  0 },
				{ -1, -1,  0,  5,  4,  5,  0, -1, -1 },
				{ -2, -2,  0,  4,  7,  4,  0, -2, -2 },
				{ -4, -4,  0,  5,  6,  5,  0, -4, -4 }
			},
			// UR
			{
				{  6,  -6,  -4, -12,  0,  -12,  -4,  -6,  6 },
				{ -5,  -8,  -6, -12,  0,  -12,  -6,  -8, -5 },
				{  2,  -8,  -4, -12, -12, -12,  -4,  -8,  2 },
				{ -4,  -9,  -4, -12, -14, -12,  -4,  -9, -4 },
				{ -8, -12, -12, -14, -15, -14, -12, -12, -8 },
				{ -8, -11, -11, -14, -15, -14, -11, -11, -8 },
				{ -6, -13, -13, -16, -16, -16, -13, -13, -6 },
				{ -6,  -8,  -7, -14, -16, -14,  -7,  -8, -6 },
				{ -6, -12,  -9, -16, -33, -16,  -9, -12, -6 },
				{ -6,  -8,  -7, -13, -14, -13,  -7,  -8, -6 }
			},
			// UN
			{
				{  0,   3,  -2,   0,  -2,   0,  -2,   3,  0 },
				{  3,  -2,  -4,  -5,  10,  -5,  -4,  -2,  3 },
				{ -5,  -4, -6,   -7,  -4,  -7,  -6,  -4, -5 },
				{ -4,  -6, -10,  -7, -10,  -7, -10,  -6, -4 },
				{ -2, -10, -13, -14, -15, -14, -13, -10, -2 },
				{ -2, -12, -11, -15, -16, -15, -11, -12, -2 },
				{ -5, -20, -12, -19, -12, -19, -12, -20, -5 },
				{ -4, -10, -11, -15, -11, -15, -11, -10, -4 },
				{ -2,  -8, -15,  -9,  -6,  -9, -15,  -8, -2 },
				{ -2,  -2,  -2,  -8,  -2,  -8,  -2,  -2, -2 }
			},
			// UB
			{
				{ 0, 0, 0, 0,  0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0,  0, 0, 0, 0, 0 },
				{ 2, 0, 0, 0, -3, 0, 0, 0, 2 },
				{ 0, 0, 0, 0,  0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0,  0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0,  0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0,  0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0,  0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0,  0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0,  0, 0, 0, 0, 0 }
			},
			// UA
			{
				{ 0, 0, 0, 0,  0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, -3, 0, 0, 0, 0 },
				{ 0, 0, 0, 0,  0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0,  0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0,  0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0,  0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0,  0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0,  0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0,  0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0,  0, 0, 0, 0, 0 }
			},
			// UG
			{
				{ 0, 0, 0, -1, -5, -1, 0, 0, 0 },
				{ 0, 0, 0,  8,  8,  8, 0, 0, 0 },
				{ 0, 0, 0,  9,  9,  9, 0, 0, 0 },
				{ 0, 0, 0,  0,  0,  0, 0, 0, 0 },
				{ 0, 0, 0,  0,  0,  0, 0, 0, 0 },
				{ 0, 0, 0,  0,  0,  0, 0, 0, 0 },
				{ 0, 0, 0,  0,  0,  0, 0, 0, 0 },
				{ 0, 0, 0,  0,  0,  0, 0, 0, 0 },
				{ 0, 0, 0,  0,  0,  0, 0, 0, 0 },
				{ 0, 0, 0,  0,  0,  0, 0, 0, 0 }
			},
			// DP
			{
				{  0,  0,  0,  2,  4,  2,  0,  0,  0 },
				{ 20, 30, 50, 65, 70, 65, 50, 30, 20 },
				{ 20, 30, 45, 55, 55, 55, 45, 30, 20 },
				{ 20, 27, 30, 40, 42, 40, 30, 27, 20 },
				{ 10, 18, 22, 35, 40, 35, 22, 18, 10 },
				{  3,  0,  4,  0,  7,  0,  4,  0,  3 },
				{ -2,  0, -2,  0,  6,  0, -2,  0, -2 },
				{  0,  0,  0,  0,  0,  0,  0,  0,  0 },
				{  0,  0,  0,  0,  0,  0,  0,  0,  0 },
				{  0,  0,  0,  0,  0,  0,  0,  0,  0 }
			},
			// DC
			{
				{  4, 4, 0, -5, -6, -5, 0, 4,  4 },
				{  2, 2, 0, -4, -7, -4, 0, 2,  2 },
				{  1, 1, 0, -5, -4, -5, 0, 1,  1 },
				{  0, 3, 3,  2,  4,  2, 3, 3,  0 },
				{  0, 0, 0,  0,  4,  0, 0, 0,  0 },
				{ -1, 0, 3,  0,  4,  0, 3, 0, -1 },
				{  0, 0, 0,  0,  0,  0, 0, 0,  0 },
				{  1, 0, 4,  3,  5,  3, 4, 0,  1 },
				{  0, 1, 2,  2,  2,  2, 2, 1,  0 },
				{  0, 0, 1,  3,  3,  3, 1, 0,  0 },
			},
			// DR
			{
				{  6,  8,  7, 13, 14, 13,  7,  8,  6 },
				{  6, 12,  9, 16, 33, 16,  9, 12,  6 },
				{  6,  8,  7, 14, 16, 14,  7,  8,  6 },
				{  6, 13, 13, 16, 16, 16, 13, 13,  6 },
				{  8, 11, 11, 14, 15, 14, 11, 11,  8 },
				{  8, 12, 12, 14, 15, 14, 12, 12,  8 },
				{  4,  9,  4, 12, 14, 12,  4,  9,  4 },
				{ -2,  8,  4, 12, 12, 12,  4,  8, -2 },
				{  5,  8,  6, 12,  0, 12,  6,  8,  5 },
				{ -6,  6,  4, 12,  0, 12,  4,  6, -6 },
			},
			// DN
			{
				{  2,  2,  2,  8,   2,  8,  2,  2,  2 },
				{  2,  8, 15,  9,   6,  9, 15,  8,  2 },
				{  4, 10, 11, 15,  11, 15, 11, 10,  4 },
				{  5, 20, 12, 19,  12, 19, 12, 20,  5 },
				{  2, 12, 11, 15,  16, 15, 11, 12,  2 },
				{  2, 10, 13, 14,  15, 14, 13, 10,  2 },
				{  4,  6, 10,  7,  10,  7, 10,  6,  4 },
				{  5,  4,  6,  7,   4,  7,  6,  4,  5 },
				{ -3,  2,  4,  5, -10,  5,  4,  2, -3 },
				{  0, -3,  2,  0,   2,  0,  2, -3,  0 },
			},
			// DB
			{
				{  0, 0, 0, 0, 0, 0, 0, 0,  0 },
				{  0, 0, 0, 0, 0, 0, 0, 0,  0 },
				{  0, 0, 0, 0, 0, 0, 0, 0,  0 },
				{  0, 0, 0, 0, 0, 0, 0, 0,  0 },
				{  0, 0, 0, 0, 0, 0, 0, 0,  0 },
				{  0, 0, 0, 0, 0, 0, 0, 0,  0 },
				{  0, 0, 0, 0, 0, 0, 0, 0,  0 },
				{ -2, 0, 0, 0, 3, 0, 0, 0, -2 },
				{  0, 0, 0, 0, 0, 0, 0, 0,  0 },
				{  0, 0, 0, 0, 0, 0, 0, 0,  0 }
			},
			// DA
			{
				{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 3, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
			},
			// DG
			{
				{ 0, 0, 0,  0,  0,  0, 0, 0, 0 },
				{ 0, 0, 0,  0,  0,  0, 0, 0, 0 },
				{ 0, 0, 0,  0,  0,  0, 0, 0, 0 },
				{ 0, 0, 0,  0,  0,  0, 0, 0, 0 },
				{ 0, 0, 0,  0,  0,  0, 0, 0, 0 },
				{ 0, 0, 0,  0,  0,  0, 0, 0, 0 },
				{ 0, 0, 0,  0,  0,  0, 0, 0, 0 },
				{ 0, 0, 0, -9, -9, -9, 0, 0, 0 },
				{ 0, 0, 0, -8, -8, -8, 0, 0, 0 },
				{ 0, 0, 0,  1,  5,  1, 0, 0, 0 }
			},
			// EE
			{
				{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
			}
		};
	};

	inline int32_t getPieceValue(Piece p) {
		return value::pieceValueMap[p_util::pieceToInt32(p)];
	}

	inline int32_t getPiecePosValue(Piece p, const Pos& pos) {
		return value::posValueMap[p_util::pieceToInt32(p)][pos.row - Board::SINGLE_ROW_PADDING][pos.col - Board::SINGLE_COL_PADDING];
	}

	int32_t calcBoardScore(const Board& bd) {
		int32_t totalScore = 0;
		Piece p{};

		for (uint32_t r = Board::ROW_BEGIN; r < Board::ROW_END; ++r) {
			for (uint32_t c = Board::COL_BEGIN; c < Board::COL_END; ++c) {
				p = bd.get(r, c);

				totalScore += getPieceValue(p);
				totalScore += getPiecePosValue(p, Pos{ r, c });
			}
		}

		return totalScore;
	}

	constexpr int32_t MAX_VALUE = std::numeric_limits<int32_t>::max();
	constexpr int32_t MIN_VALUE = std::numeric_limits<int32_t>::min();

	template<Side side>
	int32_t minMax(Board& bd, uint32_t searchDepth, int32_t alpha, int32_t beta, bool& stopFlag);

	template<>
	int32_t minMax<Side::UP>(Board& bd, uint32_t searchDepth, int32_t alpha, int32_t beta, bool& stopFlag);

	template<>
	int32_t minMax<Side::DOWN>(Board& bd, uint32_t searchDepth, int32_t alpha, int32_t beta, bool& stopFlag);

	template<>
	int32_t minMax<Side::UP>(Board& bd, uint32_t searchDepth, int32_t alpha, int32_t beta, bool& stopFlag) {
		if (searchDepth == 0) {
			return calcBoardScore(bd);
		}

		Moves moves;
		genMoves<Side::UP>(bd, moves);
		int32_t minValue = MAX_VALUE;

		for (const auto& m : moves) {
			if (stopFlag) {
				return minValue;
			}

			bd.move(m);
			minValue = std::min(minValue, minMax<Side::DOWN>(bd, searchDepth - 1, alpha, beta, stopFlag));
			bd.undo();

			beta = std::min(beta, minValue);
			if (alpha >= beta) {
				return minValue;
			}
		}

		return minValue;
	}

	template<>
	int32_t minMax<Side::DOWN>(Board& bd, uint32_t searchDepth, int32_t alpha, int32_t beta, bool& stopFlag) {
		if (searchDepth == 0) {
			return calcBoardScore(bd);
		}

		Moves moves;
		genMoves<Side::DOWN>(bd, moves);
		int32_t maxValue = MIN_VALUE;

		for (const auto& m : moves) {
			if (stopFlag) {
				return maxValue;
			}

			bd.move(m);
			maxValue = std::max(maxValue, minMax<Side::UP>(bd, searchDepth - 1, alpha, beta, stopFlag));
			bd.undo();

			alpha = std::max(alpha, maxValue);
			if (alpha >= beta) {
				return maxValue;
			}
		}

		return maxValue;
	}

	template<Side S>
	Move genBestMoveFor(Board& bd, uint32_t searchDepth);

	template<>
	Move genBestMoveFor<Side::UP>(Board& bd, uint32_t searchDepth) {
		Move bestMove{};
		Moves moves;
		genMoves<Side::UP>(bd, moves);

		bool stopFlag = false;
		auto task = std::async(std::launch::async, [&moves, &bd, &bestMove, &stopFlag, searchDepth]() {
			int32_t minValue = MAX_VALUE;
			int32_t value{};

			for (const auto& m : moves) {
				if (stopFlag) {
					break;
				}

				bd.move(m);
				value = minMax<Side::DOWN>(bd, searchDepth, MIN_VALUE, MAX_VALUE, stopFlag);
				bd.undo();

				if (minValue >= value) {
					minValue = value;
					bestMove = m;
				}
			}
			});
		
		auto status = task.wait_for(std::chrono::seconds(5));
		if (status == std::future_status::timeout) {
			stopFlag = true;
		}

		task.get();
		return bestMove;
	}

	template<>
	Move genBestMoveFor<Side::DOWN>(Board& bd, uint32_t searchDepth) {
		Move bestMove{};
		int32_t maxValue = MIN_VALUE;
		int32_t value{};
		bool stopFlag = false;

		Moves moves;
		genMoves<Side::DOWN>(bd, moves);

		for (const auto& m : moves) {
			bd.move(m);
			value = minMax<Side::UP>(bd, searchDepth, MIN_VALUE, MAX_VALUE, stopFlag);
			bd.undo();

			if (maxValue <= value) {
				maxValue = value;
				bestMove = m;
			}
		}

		return bestMove;
	}
};

bool isInputValid(const std::string& input) {
	std::regex reg{ R"([a-i][0-9][a-i][0-9])" };
	return std::regex_match(input, reg);
}

g_chess::Move inputToMove(const std::string& input) {
	g_chess::Move m{};
	m.from.row = 9 - static_cast<uint32_t>(input[1] - '0') + g_chess::Board::SINGLE_ROW_PADDING;
	m.from.col = static_cast<uint32_t>(input[0] - 'a') + g_chess::Board::SINGLE_COL_PADDING;
	m.to.row = 9 - static_cast<uint32_t>(input[3] - '0') + g_chess::Board::SINGLE_ROW_PADDING;
	m.to.col = static_cast<uint32_t>(input[2] - 'a') + g_chess::Board::SINGLE_COL_PADDING;

	return m;
}

std::string moveToStr(const g_chess::Move& m) {
	std::string str;
	str += (m.from.col - g_chess::Board::SINGLE_COL_PADDING + 'a');
	str += (9 - (m.from.row - g_chess::Board::SINGLE_ROW_PADDING) + '0');
	str += (m.to.col - g_chess::Board::SINGLE_COL_PADDING + 'a');
	str += (9 - (m.to.row - g_chess::Board::SINGLE_ROW_PADDING) + '0');

	return str;
}

g_chess::Side checkWinner(const g_chess::Board& bd) {
	using namespace g_chess;

	bool upAlive = false;
	bool downAlive = false;
	Piece p{};

	for (uint32_t r = Board::LINE_UP_9_TOP; r <= Board::LINE_UP_9_BOTTOM; ++r) {
		for (uint32_t c = Board::LINE_UP_9_LEFT; c <= Board::LINE_UP_9_RIGHT; ++c) {
			p = bd.get(r, c);
			if (p == Piece::UG) {
				upAlive = true;
			}
		}
	}

	for (uint32_t r = Board::LINE_DOWN_9_TOP; r <= Board::LINE_DOWN_9_BOTTOM; ++r) {
		for (uint32_t c = Board::LINE_DOWN_9_LEFT; c <= Board::LINE_DOWN_9_RIGHT; ++c) {
			p = bd.get(r, c);
			if (p == Piece::DG) {
				downAlive = true;
			}
		}
	}

	if (upAlive && downAlive) {
		return Side::EXTRA;
	}
	else if (upAlive) {
		return Side::UP;
	}
	else {
		return Side::DOWN;
	}
}

int main() {
	using namespace g_chess;
	Board bd;
	std::string input;
	Move aiBestMove{};

	while (true) {
		printBoard(bd);

		std::cout << "Your move: ";
		std::getline(std::cin, input);

		if (input == "undo") {
			bd.undo();
			bd.undo();
			continue;
		}
		else {
			while (!isInputValid(input)) {
				std::cout << "Please give an valid move, like b2e2: ";
				std::getline(std::cin, input);
			}
		}

		auto m = inputToMove(input);
		while (!isValidMove(bd, m)) {
			std::cout << "invalid move, please enter right move: ";
			std::getline(std::cin, input);
			m = inputToMove(input);
			if (input == "undo") {
				bd.undo();
				bd.undo();
				continue;
			}
			else {
				while (!isInputValid(input)) {
					std::cout << "Please give an valid move, like b2e2: ";
					std::getline(std::cin, input);
				}
			}
		}

		bd.move(m);
		printBoard(bd);

		if (checkWinner(bd) == Side::DOWN) {
			std::cout << "You win!\n";
			break;
		}

		std::cout << "AI thinking...\n";
		aiBestMove = genBestMoveFor<Side::UP>(bd, 5);
		char c = p_util::getChar(bd.get(aiBestMove.from));
		std::cout << "AI moves: " << moveToStr(aiBestMove) << " -> " << c << "\n";
		bd.move(aiBestMove);

		if (checkWinner(bd) == Side::UP) {
			printBoard(bd);
			std::cout << "You lose!\n";
			break;
		}
	}

	std::getline(std::cin, input);
	return 0;
}
