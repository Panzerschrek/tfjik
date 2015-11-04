#include "lexical_analyzer.h"
#include "syntax_analyzer.h"

namespace SyntaxAnalyzer
{

static CombinedName ParseCombinedName( const Lexems& lexems, Lexems::const_iterator& it )
{
	CombinedName result;
	if( it->type == Lexem::Type::Identifier )
	{
		result.names_part.push_back( it->text );
		it++;
	}
	else result.names_part.push_back( "" );

	while( it < lexems.end() )
	{
		if( it->type != Lexem::Type::Scope ) break;
		it++;
		if( it >= lexems.end() || it->type != Lexem::Type::Identifier ) throw SyntaxError(it - lexems.begin());
		result.names_part.push_back( it->text );
		it++;
	}

	return result;
}

Enumeration ParseEnumeration( const Lexems& lexems )
{
	Enumeration result;

	Lexems::const_iterator it= lexems.begin();

	if( it != lexems.end() &&
		it->type == Lexem::Type::Identifier &&
		it->text == "enum" )
	{
		it++;
	}
	else throw SyntaxError(it - lexems.begin());

	if( it != lexems.end() && it->type == Lexem::Type::Identifier )
	{
		if( it->text == "struct" || it->text == "class" )
		{
			result.is_scopped= true;
			it++;
		}
	}

	if( it != lexems.end() && it->type == Lexem::Type::Identifier )
	{
		result.name= it->text;
		it++;
	}

	if( it != lexems.end() && it->type == Lexem::Type::Colon )
	{
		it++;
		while( it != lexems.end() && ( it->type == Lexem::Type::Identifier || it->type == Lexem::Type::Scope ) )
			result.base_type.push_back( ParseCombinedName( lexems, it ) );
	}

	if( it != lexems.end() && it->type == Lexem::Type::BraceLeft )
		it++;
	else throw SyntaxError(it - lexems.begin());

	while( it != lexems.end() )
	{
		if( it->type == Lexem::Type::BraceRight )
		{
			it++;
			break;
		}
		else if( it->type != Lexem::Type::Identifier ) throw SyntaxError(it - lexems.begin());

		result.members.emplace_back();
		Enumeration::Member& enumeration_member= result.members.back();

		enumeration_member.name= it->text;
		it++;

		if( it == lexems.end() ) throw SyntaxError(it - lexems.begin());
		if( it->type == Lexem::Type::Assignment )
		{
			it++;
			if( it != lexems.end() && it->type == Lexem::Type::NumericConstant )
			{
				enumeration_member.value= it->text;
				it++;
			}
			else throw SyntaxError(it - lexems.begin());
		}
		if( it->type == Lexem::Type::Comma )
		{
			it++;
			continue;
		}
		else if( it->type == Lexem::Type::BraceRight ) continue;
		else throw SyntaxError(it - lexems.begin());
	}

	if( it != lexems.end() && it->type == Lexem::Type::Semicolon ){}
	else throw SyntaxError(it - lexems.begin());

	return result;
}

} // namespace SyntaxAnalyzer
