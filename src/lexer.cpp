#include <string>
#include <iostream>
#include "lexer.h"

Glass::Token::Token(TokenType type, std::string value){
    this->type = type;
    this->value = value;
}

Glass::Token::Token(TokenType type){
    this->type = type;
}

Glass::Token::~Token(){

}

Glass::Lexer::Lexer(std::string code){
    this->code = code;
}

Glass::Lexer::~Lexer(){
}

/*
De un codi doncs torna tokens!
*/
std::vector<Glass::Token>* Glass::Lexer::GetTokens(){
    return &tokenList;
}

std::string Glass::Lexer::nextWord(int pos, int* nextPos){
    std::string nWord = "";
    while(!isEnd(pos) && !isSymbol(code[pos])){
        nWord += code[pos];
        pos++;
        if(isEmpty(code[pos]) || isSymbol(code[pos])) break;
    }
    *nextPos = pos;
    return nWord;
}

int Glass::Lexer::isNumber(char c){
    if(c >= 48 && c <= 57) return true;
    return false;
}

int Glass::Lexer::isEmpty(char c){
    return c == typeSymbol[Symbols::SPACE] || c == typeSymbol[Symbols::NEWLINE];
}

void Glass::Lexer::flush(int *next){
    int pos = *next;
    while(isEmpty(code[pos])) pos++;
    *next = pos;
}

int Glass::Lexer::isSemicolon(char c){
    return c == typeSymbol[Symbols::SEMICOLON];
}

int Glass::Lexer::isSeparator(char c){
    return 0;
}

int Glass::Lexer::isComment(char c){
    return c == typeSymbol[Symbols::COMMENT];
}

int Glass::Lexer::isEnd(int pos){
    return pos >= code.size();
}

int Glass::Lexer::isPoint(char c){
    return c == typeSymbol[Symbols::POINT];
}

int Glass::Lexer::isQuot(char c){
    return c == typeSymbol[Symbols::QUOT];
}

int Glass::Lexer::isDoubleQuot(char c){
    return c == typeSymbol[Symbols::DOUBLE_QUOT];
}

int Glass::Lexer::isComma(char c){
    return c == typeSymbol[Symbols::COMMA];
}

int Glass::Lexer::isOParentesis(char c){
    return c == typeSymbol[Symbols::PARENTESIS_O];
}

int Glass::Lexer::isCParentesis(char c){
    return c == typeSymbol[Symbols::PARENTESIS_C];
}

int Glass::Lexer::isParentesis(char c){
    return isOParentesis(c) || isCParentesis(c);
}

int Glass::Lexer::isOClaudator(char c){
    return c == typeSymbol[Symbols::CLAUDATOR_O];
}

int Glass::Lexer::isCClaudator(char c){
    return c == typeSymbol[Symbols::CLAUDATOR_C];
}

int Glass::Lexer::isClaudator(char c){
    return isCClaudator(c) || isOClaudator(c);
}

int Glass::Lexer::isOKey(char c){
    return c == typeSymbol[Symbols::KEY_O];
}

int Glass::Lexer::isCKey(char c){
    return c == typeSymbol[Symbols::KEY_C];
}

int Glass::Lexer::isKey(char c){
    return isOKey(c) || isCKey(c);
}

int Glass::Lexer::isTwoPoints(char c){
    return c == typeSymbol[Symbols::TWO_POINTS];
}

int Glass::Lexer::isSymbol(char c){
    for (auto& it : typeSymbol) {
        if (it.second == c) {
            return 1;
        }
    }

    return 0;
}

void Glass::Lexer::addError(){
    tokenList.push_back(Token(Token::ERROR, ""));
}

void Glass::Lexer::getNumber(int *next){
    int pos = *next;

    std::string num(1, code[pos]);

    int isRational = 0;

    for(pos++; isNumber(code[pos]); pos++){
        num += code[pos];
    }
    
    if(code[pos] == typeSymbol[Symbols::POINT]){
        num += ".";
        isRational = 1;
        for(pos += 1; isNumber(code[pos]); pos++){
            num += code[pos];
        }
    }

    *next = pos;
    
    if(isEmpty(code[pos]) || !isNumber(code[pos]) || isSeparator(code[pos]) || isEnd(pos)){
        if(isRational) tokenList.push_back(Token(Token::L_REAL, num));
        else tokenList.push_back(Token(Token::L_INT, num));
    } else {
        // Error
        addError();
    }
}

void Glass::Lexer::getString(int *next){
    int pos = *next;
    std::string s = "";

    char check = code[pos];
    pos++;
    while(code[pos] != check){
        if(code[pos] == '\\'){
            if(!isEnd(pos + 1)){
                if(code[pos + 1] == check){
                    s += check;
                    pos += 2;
                    continue;
                }
            } else {
                // Error
                addError();
                return;
            }
        }
        s += code[pos];
        pos++;
    }

    *next = pos + 1;
    tokenList.push_back(Token(Token::L_STRING, s));
    // Sin error

}

void Glass::Lexer::skipComment(int *next){
    if(isComment(code[*next])){
        int pos = *next;
        while(code[pos] != typeSymbol[Symbols::NEWLINE] && !isEnd(pos)){
            pos++;
        }
        *next = pos + 1;
    }
}

int Glass::Lexer::checkLiterals(int *next){
    if(isEnd(*next)) return 1;

    char c = code[*next];
    if(isNumber(c) || isPoint(c)){
        getNumber(next);
        return 0;
    }
    else if(isQuot(c) || isDoubleQuot(c)){
        getString(next);
        return 0;
    }

    flush(next);
    return 1;
}

int Glass::Lexer::checkOperations(int *next){
    if(isEnd(*next)) return 1;
    if(isEmpty(code[*next])) flush(next);

    int pos = *next;
    // Bueno pues a comprovar
    std::string test(1, code[*next]);
    int i;
    for(i = 1; i < 2; i++){
        if(isEnd(pos+i)){
            break;
        }
        test += code[pos+i];
    }

    bool done = false;
    for(i--; i >= 0; i--){
        if(typeOperation.count(test)){
            tokenList.push_back(Token(typeOperation[test]));
            pos+=i+1;
            *next = pos;
            done = true;
            break;
        }
        test = test.substr(0, i);
    }

    flush(next);
    if(done) return 0;
    return 1;
}

int Glass::Lexer::checkKeywords(int *next){

    if(isEnd(*next)) return 1;
    if(isEmpty(code[*next])) return 1;
    if(isSymbol(code[*next])) return 1;
    if(isNumber(code[*next])) return 1;
    
    int pos = *next;
    std::string word;

    int nextPos = pos;
    word = nextWord(pos, &nextPos);
    if(typeKeyword.count(word)){
        switch (typeKeyword[word]){
            case FUNC:
                /* code */
                tokenList.push_back(Token(Token::FUNCTION_DECLARATION));
                break;
            case MODULE:
                tokenList.push_back(Token(Token::MODULE_DECLARATION));
                break;
            case IMPORT:
                tokenList.push_back(Token(Token::IMPORT_DECLARATION));
                break;
            case IF:
                tokenList.push_back(Token(Token::K_IF));
                break;
            case ELSE:
                tokenList.push_back(Token(Token::K_ELSE));
                break;
            case WHILE:
                tokenList.push_back(Token(Token::K_WHILE));
                break;
            case RETURN:
                tokenList.push_back(Token(Token::K_RETURN));
                break;
            case BREAK:
                tokenList.push_back(Token(Token::K_BREAK));
                break;
            case CONTINUE:
                tokenList.push_back(Token(Token::K_CONTINUE));
                break;
            case REAL:
                tokenList.push_back(Token(Token::T_REAL));
                break;
            case INT:
                tokenList.push_back(Token(Token::T_INT));
                break;
            case STRING:
                tokenList.push_back(Token(Token::T_STRING));
                break;
            case BOOLEAN:
                tokenList.push_back(Token(Token::T_BOOLEAN));
                break;
            case TRUE:
                tokenList.push_back(Token(Token::L_TRUE));
                break;
            case FALSE:
                tokenList.push_back(Token(Token::L_FALSE));
                break;
            case _NULL:
                tokenList.push_back(Token(Token::L_NULL));
                break;
            default:
                /* Ha de ser una reference */

                break;
        }
    } else {
        tokenList.push_back(Token(Token::IDENTIFIER, word));    
    }

    *next = nextPos;

    flush(next);
    return 0;
    
}

int Glass::Lexer::checkSymbols(int *next){
    if(isEnd(*next)) return 0;
    switch(code[*next]){
        case ',':
            tokenList.push_back(Token(Token::COMMA));
            break;
        case ';':
            tokenList.push_back(Token(Token::SEMICOLON));
            break;
        case ':':
            tokenList.push_back(Token(Token::TWO_POINTS));
            break;
        case '(':
            tokenList.push_back(Token(Token::PARENTHESIS_OPEN));
            break;
        case ')':
            tokenList.push_back(Token(Token::PARENTHESIS_CLOSE));
            break;
        case '[':
            tokenList.push_back(Token(Token::SQUARE_BRACKET_OPEN));
            break;
        case ']':
            tokenList.push_back(Token(Token::SQUARE_BRACKET_CLOSE));
            break;
        case '{':
            tokenList.push_back(Token(Token::CURLY_BRACKET_OPEN));
            break;
        case '}':
            tokenList.push_back(Token(Token::CURLY_BRACKET_CLOSE));
            break;
        case '$':
            tokenList.push_back(Token(Token::DOLLAR));
            break;
        case '!':
            tokenList.push_back(Token(Token::S_FLIP));
            break;
        case '|':
            tokenList.push_back(Token(Token::S_DIVIDES));
            break;
        default:
            return 1;
    }
    *next += 1;
    flush(next);
    return 0;
}

int Glass::Lexer::GenerateTokens(){
    for(int i = 0; i < code.size(); ){
        skipComment(&i);

        if(checkLiterals(&i))
        if(checkSymbols(&i))
        if(checkOperations(&i))
        checkKeywords(&i);

        // std::cout << code[i] << std::endl;
    }

    return 0;
}