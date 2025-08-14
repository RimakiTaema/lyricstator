#pragma once
#include "common/Types.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

namespace Lyricstator {

// Token types for lexical analysis
enum class TokenType {
    // Literals
    STRING,
    NUMBER,
    BOOLEAN,
    
    // Identifiers
    IDENTIFIER,
    
    // Keywords
    DISPLAY,
    TIMING,
    ANIMATE,
    COLOR,
    POSITION,
    FADE_IN,
    FADE_OUT,
    HIGHLIGHT,
    WAIT,
    REPEAT,
    IF,
    ELSE,
    WHILE,
    
    // Operators
    ASSIGN,
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    EQUALS,
    NOT_EQUALS,
    LESS_THAN,
    GREATER_THAN,
    
    // Punctuation
    SEMICOLON,
    COMMA,
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACE,
    RIGHT_BRACE,
    LEFT_BRACKET,
    RIGHT_BRACKET,
    
    // Special
    NEWLINE,
    END_OF_FILE,
    INVALID
};

// Token structure
struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
    
    Token(TokenType t = TokenType::INVALID, const std::string& v = "", int l = 0, int c = 0)
        : type(t), value(v), line(l), column(c) {}
};

// AST Node types
enum class ASTNodeType {
    PROGRAM,
    COMMAND,
    EXPRESSION,
    LITERAL,
    IDENTIFIER,
    PARAMETER_LIST,
    PARAMETER,
    CONDITION,
    LOOP,
    BLOCK
};

// Abstract Syntax Tree Node
class ASTNode {
public:
    ASTNodeType type;
    std::string value;
    std::vector<std::unique_ptr<ASTNode>> children;
    std::unordered_map<std::string, std::string> attributes;
    
    ASTNode(ASTNodeType t, const std::string& v = "") : type(t), value(v) {}
    virtual ~ASTNode() = default;
    
    void AddChild(std::unique_ptr<ASTNode> child) {
        children.push_back(std::move(child));
    }
    
    void SetAttribute(const std::string& key, const std::string& value) {
        attributes[key] = value;
    }
    
    std::string GetAttribute(const std::string& key) const {
        auto it = attributes.find(key);
        return (it != attributes.end()) ? it->second : "";
    }
};

// Lexical analyzer
class Lexer {
public:
    Lexer(const std::string& source);
    
    std::vector<Token> Tokenize();
    Token GetNextToken();
    bool HasMoreTokens() const;
    
private:
    std::string source_;
    size_t position_;
    int line_;
    int column_;
    
    char CurrentChar() const;
    char PeekChar(int offset = 1) const;
    void Advance();
    void SkipWhitespace();
    void SkipComment();
    
    Token ReadString();
    Token ReadNumber();
    Token ReadIdentifier();
    
    bool IsAlpha(char c) const;
    bool IsDigit(char c) const;
    bool IsAlphaNumeric(char c) const;
    
    TokenType GetKeywordType(const std::string& identifier) const;
};

// Syntax analyzer and parser
class LystrParser {
public:
    LystrParser();
    ~LystrParser();
    
    // File parsing
    bool ParseFile(const std::string& filepath);
    bool ParseString(const std::string& source);
    void Clear();
    
    // Result access
    const std::vector<LystrCommand>& GetCommands() const { return commands_; }
    const std::vector<std::string>& GetErrors() const { return errors_; }
    bool HasErrors() const { return !errors_.empty(); }
    
    // AST access (for debugging/advanced usage)
    const ASTNode* GetAST() const { return ast_.get(); }
    
    // Validation
    bool ValidateScript() const;
    std::vector<std::string> GetValidationErrors() const;
    
private:
    // Lexical analysis
    std::unique_ptr<Lexer> lexer_;
    std::vector<Token> tokens_;
    size_t currentToken_;
    
    // Parsing state
    std::unique_ptr<ASTNode> ast_;
    std::vector<LystrCommand> commands_;
    std::vector<std::string> errors_;
    
    // Parser methods
    std::unique_ptr<ASTNode> ParseProgram();
    std::unique_ptr<ASTNode> ParseStatement();
    std::unique_ptr<ASTNode> ParseCommand();
    std::unique_ptr<ASTNode> ParseExpression();
    std::unique_ptr<ASTNode> ParseParameterList();
    std::unique_ptr<ASTNode> ParseParameter();
    std::unique_ptr<ASTNode> ParseCondition();
    std::unique_ptr<ASTNode> ParseLoop();
    std::unique_ptr<ASTNode> ParseBlock();
    
    // Token utilities
    const Token& CurrentToken() const;
    const Token& PeekToken(int offset = 1) const;
    void ConsumeToken();
    bool Match(TokenType type);
    bool Expect(TokenType type);
    
    // Error handling
    void AddError(const std::string& message);
    void AddError(const std::string& message, const Token& token);
    
    // AST to command conversion
    void ConvertASTToCommands(const ASTNode* node);
    LystrCommand CreateCommand(const ASTNode* commandNode);
    LystrCommandType GetCommandType(const std::string& commandName);
    
    // Validation helpers
    bool ValidateCommand(const LystrCommand& command) const;
    bool ValidateParameters(const LystrCommand& command) const;
    bool ValidateTiming(const std::vector<LystrCommand>& commands) const;
    
    // Built-in functions and variables
    std::unordered_map<std::string, std::string> variables_;
    std::unordered_map<std::string, std::vector<std::string>> functions_;
    
    void InitializeBuiltins();
    bool IsBuiltinFunction(const std::string& name) const;
    bool IsValidVariable(const std::string& name) const;
};

// Utility functions for script analysis
class LystrAnalyzer {
public:
    static std::vector<std::string> ExtractLyricText(const std::vector<LystrCommand>& commands);
    static std::vector<uint32_t> ExtractTimestamps(const std::vector<LystrCommand>& commands);
    static uint32_t GetScriptDuration(const std::vector<LystrCommand>& commands);
    static bool HasAnimations(const std::vector<LystrCommand>& commands);
    static bool HasConditionals(const std::vector<LystrCommand>& commands);
    static std::unordered_map<std::string, int> GetCommandStats(const std::vector<LystrCommand>& commands);
};

} // namespace Lyricstator
