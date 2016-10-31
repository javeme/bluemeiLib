#include "StdAfx.h"
#include "TemplateParser.h"

namespace blib{

/////////////////////////////////////////////////////////////////////
//Tokenizer
Tokenizer::Tokenizer( const String& s )
{
	current = null;
	next = null;
	reset(s);
}

Tokenizer::~Tokenizer()
{
	destroy();
}

void Tokenizer::reset( const String& s )
{
	this->destroy();

	this->content = s;
	this->current = new Token(-1);
	this->next = this->readNext(new Token(0));
}

void Tokenizer::destroy()
{
	delete current;
	delete next;
	current = null;
	next = null;
}

String Tokenizer::readIdent(unsigned int pos)
{
	StringBuilder ident;
	ident.append(content[pos]);
	if(!CodeUtil::isAlpha(ident.charAt(0)))
		throwpe(ParseTemplateException("invalid ident"));

	for(unsigned i = pos + 1; i < content.length(); ++i)
	{
		char ch = content[i];
		if(CodeUtil::isAlphaOrNumber(ch) || ch == '_')
			ident.append(ch);
		else
			break;
	}
	return ident.toString();
}

unsigned int Tokenizer::readString( unsigned int pos, String& str )
{
	StringBuilder buf;
	char pre = 0;
	unsigned int count = 0;
	for(unsigned i = pos; i < content.length(); ++i)
	{
		char ch = content[i];
		if(ch == '$'){
			if(pre != '\\')
				break;
			else
				buf.pop();
		}
		buf.append(ch);
		pre = ch;
		count += 1;
	}
	str = buf.toString();
	return count;
}

Token* Tokenizer::readNext( Token* current )
{
	Token& next = *current;
	unsigned int pos = next.pos;

	char sym = 0;
	if(pos >= this->content.length())
		sym = 0;
	else
		sym = this->readChar(pos);

	switch(sym)
	{
	case 0:
		next.token = Token::END;
		break;
	case '\n':
		next.nextLine();
		return this->readNext(&next);
	case '$':
		next.nextPos();
		next.token = Token::IDENT;
		next.str = this->readIdent(next.pos);
		next.nextPos(next.str.length());
		break;
	default:
		next.token = Token::CONST_STR;
		next.nextPos(this->readString(next.pos, next.str));
	}

	return &next;
}

Token Tokenizer::nextToken()
{
	Token ret = *this->next;

	Token* next = this->current;
	this->current = this->next;
	*next = *this->next;
	this->next = this->readNext(next);

	return ret;
}



/////////////////////////////////////////////////////////////////////
//TemplateParser
TemplateExpr* TemplateParser::expr()
{
	Token token = this->peekToken();
	Token::Symbol tok = token.token;
	if(tok == Token::CONST_STR)
		return this->joinConstString();
	else if(tok == Token::IDENT) // hello $name($age)
		return this->joinVar();
	else
		throwpe(ParseTemplateException("unknow token " + token.toString()));
}

TemplateExpr* TemplateParser::joiner( TemplateExpr* left )
{
	TemplateExpr* right = this->expr();
	return new JoinerExpr(left, right);
}

TemplateExpr* TemplateParser::stmt()
{
	Token token = this->peekToken();
	Token::Symbol tok = token.token;
	TemplateExpr* node = new EmptyExpr();
	while(this->peekToken().token != Token::END)
		node = this->joiner(node);

	return node;
}

TemplateExpr* TemplateParser::tmpl()
{
	TemplateExpr* root = this->stmt();
	Token token = this->peekToken();
	Token::Symbol tok = token.token;
	if(tok != Token::END)
		throwpe(ParseTemplateException("unexpected end: " + token.toString()));
	return root;
}

}