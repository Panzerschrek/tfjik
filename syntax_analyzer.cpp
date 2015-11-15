#include "lexical_analyzer.h"
#include "syntax_analyzer.h"

void CombinedName::Print(std::ostream& stream) const
{
	stream<< names_part.front();
	for( size_t i= 1; i < names_part.size(); i++ )
		stream<< "::"<< names_part[i];
}

void BinaryOperationsChain::NumericConstant::Print(std::ostream &stream) const
{
	stream<< value;
}

void BinaryOperationsChain::Variable::Print(std::ostream &stream) const
{
	name.Print(stream);
}

void BinaryOperationsChain::BracketExpression::Print(std::ostream &stream) const
{
	stream<< "( ";
	subexpression->Print(stream);
	stream<< " )";
}

void BinaryOperationsChain::Print(std::ostream& stream) const
{
	for( const ComponentWithOperator& component : components )
	{
		component.component->Print(stream);
		stream<< " ";
		const char* op_str;
		switch (component.op)
		{
			case Operator::Add: op_str= "+"; break;
			case Operator::Sub: op_str= "-"; break;
			case Operator::Mul: op_str= "*"; break;
			case Operator::Div: op_str= "/"; break;
			default: op_str = ""; break;
		};
		stream<< op_str<< " ";
	}
}

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

BinaryOperationsChain::Operator ParseBinaryOperator( Lexems::const_iterator& it )
{
	switch( it->type )
	{
		case Lexem::Type::Plus: it++; return BinaryOperationsChain::Operator::Add;
		case Lexem::Type::Minus: it++; return BinaryOperationsChain::Operator::Sub;
		case Lexem::Type::Star: it++; return BinaryOperationsChain::Operator::Mul;
		case Lexem::Type::Slash: it++; return BinaryOperationsChain::Operator::Div;
		default: break;
	};

	return BinaryOperationsChain::Operator::NoOperator;
}

BinaryOperationsChain ParseBinaryOperatorsChain( const Lexems& lexems, Lexems::const_iterator& it )
{
	BinaryOperationsChain result;

	while( it < lexems.end() )
	{
		BinaryOperationsChain::ComponentWithOperator comp;

		switch (it->type)
		{
		case Lexem::Type::Identifier:
		case Lexem::Type::Scope:
			{
				std::unique_ptr<BinaryOperationsChain::Variable> var( new BinaryOperationsChain::Variable );
				var->name= ParseCombinedName( lexems, it );

				comp.component= std::move(var);
			}
			break;

		case Lexem::Type::BracketLeft:
			{
				it++;
				if( it == lexems.end()) throw LexicalError(lexems.back().file_position);

				std::unique_ptr<BinaryOperationsChain::BracketExpression> subexpr( new BinaryOperationsChain::BracketExpression );
				subexpr->subexpression.reset( new BinaryOperationsChain(ParseBinaryOperatorsChain( lexems, it )) );
				comp.component= std::move(subexpr);

				if( it->type != Lexem::Type::BracketRight ) throw LexicalError( it->file_position);
				it++;
			}
			break;

		case Lexem::Type::NumericConstant:
			{
				std::unique_ptr<BinaryOperationsChain::NumericConstant> num( new BinaryOperationsChain::NumericConstant );
				num->value= it->text;
				it++;

				comp.component= std::move(num);
			}
			break;

		default:
			goto end;
		};

		BinaryOperationsChain::Operator op= ParseBinaryOperator( it );
		comp.op= op;
		result.components.push_back(std::move(comp));

		if( op == BinaryOperationsChain::Operator::NoOperator ) break;
	}
	end:;

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
			/*if( it != lexems.end() && it->type == Lexem::Type::NumericConstant )
			{
				BinaryOperationsChain chain;
				chain.components.emplace_back();
				enumeration_member.value= std::move(chain);
				it++;
			}
			else throw SyntaxError(it - lexems.begin());*/
			enumeration_member.value= ParseBinaryOperatorsChain( lexems, it );
			if (it == lexems.end() ) throw SyntaxError(it - lexems.begin());
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
