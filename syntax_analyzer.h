#pragma once
#include <exception>
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
};

struct Enumeration
{
	struct Member
	{
		std::string name;
		std::string value;
	};

	std::string name;

	bool is_scopped= false;
	std::vector< CombinedName > base_type;

	std::vector<Member> members;
};

namespace SyntaxAnalyzer
{

Enumeration ParseEnumeration( const Lexems& lexems );

} // namespace SyntaxAnalyzer
