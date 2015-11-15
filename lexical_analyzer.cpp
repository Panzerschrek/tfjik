#include <map>

#include "lexical_analyzer.h"

static const std::map<std::string, Lexem::Type> c_operators_lexems=
{
	{ "(", Lexem::Type::BracketLeft },
	{ ")", Lexem::Type::BracketRight },
	{ "[", Lexem::Type::SquareBracketLeft },
	{ "]", Lexem::Type::SquareBracketRight },
	{ "{", Lexem::Type::BraceLeft },
	{ "}", Lexem::Type::BraceRight },

	{ ".", Lexem::Type::Dot },
	{ ",", Lexem::Type::Comma },
	{ ";", Lexem::Type::Semicolon },
	{ ":", Lexem::Type::Colon },
	{ "::", Lexem::Type::Scope },

	{ "=", Lexem::Type::Assignment },
	{ "+=", Lexem::Type::AssignmentAdd },
	{ "-=", Lexem::Type::AssignmentSub },
	{ "*=", Lexem::Type::AssignmentMul },
	{ "/=", Lexem::Type::AssignmentDiv },
	{ "%=", Lexem::Type::AssignmentMod },
	{ "&=", Lexem::Type::AssignmentAnd },
	{ "|=", Lexem::Type::AssignmentOr  },
	{ "^=", Lexem::Type::AssignmentXor },
	{ "<<=", Lexem::Type::AssignmentShiftLeft  },
	{ ">>=", Lexem::Type::AssignmentShiftRight },

	{ "<", Lexem::Type::CompareLess },
	{ ">", Lexem::Type::CommpareGreater },
	{ "==", Lexem::Type::CompareEqual },
	{ "!=", Lexem::Type::CompareNotEqual },
	{ "<=" , Lexem::Type::CompareLessOrEqual },
	{ ">=", Lexem::Type::CompareGreaterOrEqual },

	{ "++", Lexem::Type::Increment },
	{ "--", Lexem::Type::Decrement },
	{ "+", Lexem::Type::Plus },
	{ "-", Lexem::Type::Minus },
	{ "*", Lexem::Type::Star },
	{ "/", Lexem::Type::Slash },
	{ "&", Lexem::Type::And },
	{ "|", Lexem::Type::Or },
	{ "^", Lexem::Type::Xor },
	{ "~", Lexem::Type::Tilda },
	{ "!", Lexem::Type::Not },

	{ "&&", Lexem::Type::Conjunction },
	{ "||", Lexem::Type::Disjunction },
};

static const int c_max_predifined_lexem_length= 4;

namespace LexemParser
{

inline bool IsHexDigit(char c)
{
	if( isdigit(c) ) return true;

	char upper= toupper(c);
	return upper >= 'A' && upper <= 'F';
}

static Lexem ParseNumericConstant( const std::string& file_data, std::string::const_iterator& it )
{
	Lexem result;

	result.file_position= it - file_data.begin();

	// Hex
	if( *it == '0' && (it+1) < file_data.end() && *(it+1) == 'x' )
	{
		if( it+2 == file_data.end() || !IsHexDigit(*it) )
			throw LexicalError( it - file_data.begin() );

		result.text+= "0x";
		it+= 2;

		while( it != file_data.end() && IsHexDigit(*it) )
		{
			result.text+= *it;
			it++;
		}
	}
	else
		while( it != file_data.end() && isdigit(*it) )
		{
			result.text+= *it;
			it++;
		}

	if( it != file_data.end() && isalpha(*it) )
		throw LexicalError( it - file_data.begin() );

	result.type= Lexem::Type::NumericConstant;
	return result;
}

static Lexem ParseIdentifier( const std::string& file_data, std::string::const_iterator& it )
{
	Lexem result;

	result.file_position= it - file_data.begin();

	while( it != file_data.end() && ( isalnum( *it ) || *it == '_' ) )
	{
		result.text+= *it;
		it++;
	}

	result.type= Lexem::Type::Identifier;
	return result;
}

Lexems Parse( const std::string& file_data )
{
	Lexems result;

	unsigned int line= 1;
	std::string::const_iterator it= file_data.begin();

	while( it != file_data.end() )
	{
		const char c= *it;

		Lexem lexem;

		if( c == '/' )
		{
			if( it+1 < file_data.end() )
			{
				it++;
				if( *it == '/' )
				{
					it++;
					while( it < file_data.end() && *it != '\n' ) it++;
					line++;
					continue;
				}
				else if( *it == '*' )
				{
					it++;
					while( it < file_data.end() - 1 )
					{
						if( *it == '\n' ) line++;
						else if( *it == '*' && *(it+1) == '/' )
						{
							it+= 2;
							break;
						}
						it++;
					}
					continue;
				}
				else
				{
					lexem.type= Lexem::Type::Slash;
					lexem.text= "/";
				}

			}
		}
		else if( c == '\n' )
		{
			line++;
			it++;
			continue;
		}
		else if( isdigit(c) )
			lexem= ParseNumericConstant( file_data, it );
		else if( isalpha(c) )
			lexem= ParseIdentifier( file_data, it );
		else
		{
			bool found= false;

			for( int i= c_max_predifined_lexem_length; i >= 1; i-- )
			{
				if( it + i <= file_data.end() )
				{
					auto operator_lexem_it= c_operators_lexems.find( std::string( &*it, i ) );
					if( operator_lexem_it != c_operators_lexems.end() )
					{
						lexem.text= operator_lexem_it->first;
						lexem.type= operator_lexem_it->second;
						lexem.file_position= it - file_data.begin();
						it+= i;
						found= true;
						break;
					}
				}
			}
			if( !found )
			{
				// TODO - check for trash
				it++;
				continue;
			}
		}

		lexem.line= line;
		result.push_back(lexem);

	} // while not end

	return result;
}

} // namespace LexemParser
