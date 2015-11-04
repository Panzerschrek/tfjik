#pragma once
#include <exception>
#include <string>
#include <vector>

struct LexicalError : std::exception
{
	LexicalError( unsigned int file_pos )
		: file_pos_(file_pos)
		, wtf_("LexicalError")
	{
	}

	const char* what() const noexcept override
	{
		return wtf_.data();
	}

	unsigned int file_pos_;
	std::string wtf_;
};

struct Lexem
{
	enum class Type
	{
		Identifier, // BlaBla87

		BracketLeft, // (
		BracketRight, // )

		SquareBracketLeft, // [
		SquareBracketRight, // ]

		BraceLeft, // {
		BraceRight, // }

		NumericConstant, // 0x78, 9887

		Dot, // .
		Comma, // ,
		Semicolon, // ;

		Colon, // :
		Scope, // ::

		Assignment   , // =
		AssignmentAdd, // +=
		AssignmentSub, // -=
		AssignmentMul, // *=
		AssignmentDiv, // /=
		AssignmentMod, // %=
		AssignmentAnd, // &=
		AssignmentOr , // |=
		AssignmentXor, // ^=
		AssignmentShiftLeft , // <<=
		AssignmentShiftRight, // >>=

		CompareLess, // <
		CommpareGreater, // >
		CompareEqual, // ==
		CompareNotEqual, // !=
		CompareLessOrEqual, // <=
		CompareGreaterOrEqual, // >=

		Increment, // ++
		Decrement, // --
		Plus, // +
		Minus, // -
		Star, // *
		And, // &
		Or, // |
		Xor, // ^
		Tilda, // ~
		Not, // !

		Conjunction, // &&
		Disjunction, // ||
	};

	Type type;
	std::string text;

	unsigned int line;
	unsigned int file_position;
};

typedef std::vector<Lexem> Lexems;

namespace LexemParser
{

Lexems Parse( const std::string& file_data );

} // namespace LexemParser
