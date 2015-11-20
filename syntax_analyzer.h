#pragma once
#include <exception>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "lexical_analyzer.h"

struct SyntaxError : std::exception
{
	SyntaxError( unsigned int lexem )
		: lexem_(lexem)
	{}

	unsigned int lexem_;
};

struct CombinedName
{
	// for example
	// ::a::b::c
	// c::d::k
	// khe::khe
	// wtf
	std::vector< std::string > names_part;

	void Print(std::ostream& stream) const;
};

struct BinaryOperationsChain
{
	enum class Operator
	{
		Add,
		Sub,
		Mul,
		Div,
		NoOperator // Special value - for end of binary operators chain
	};

	// Components of binary expression chain

	struct IComponent
	{
		virtual ~IComponent() {}
		virtual void Print(std::ostream& stream) const = 0;
	};
	typedef std::unique_ptr<IComponent> IComponentPtr;

	struct NumericConstant : public IComponent
	{
		virtual void Print(std::ostream& stream) const override;

		std::string value;
	};

	struct Variable : public IComponent
	{
		virtual void Print(std::ostream& stream) const override;

		CombinedName name;
	};

	struct BracketExpression : public IComponent
	{
		virtual void Print(std::ostream& stream) const override;

		std::unique_ptr<BinaryOperationsChain> subexpression;
	};

	struct ComponentWithOperator
	{
		IComponentPtr component;
		Operator op= Operator::NoOperator;
	};

	void Print(std::ostream& stream) const;

	std::vector< ComponentWithOperator > components;
};

struct Enumeration
{
	struct Member
	{
		std::string name;
		BinaryOperationsChain value;
	};

	std::string name;

	bool is_scopped= false;
	std::vector< CombinedName > base_type;

	std::vector<Member> members;
};

typedef std::vector<Enumeration> Enumerations;

namespace SyntaxAnalyzer
{

Enumerations ParseEnumerations( const Lexems& lexems );

} // namespace SyntaxAnalyzer
