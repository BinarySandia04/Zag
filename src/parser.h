#pragma once

#include <vector>

#include "lexer.h"
#include "nodes.h"

namespace That {

    class Parser {
        public:
            Parser(std::vector<That::Token> tokens);

            Nodes::Node GenerateAST();
        private:
            std::vector<That::Token> tokens;
            
            void GetExpression(Nodes::Expression** parent, int from, int to);
    };
}