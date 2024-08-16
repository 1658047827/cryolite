#include "lexer.h"

typedef enum CharFlags {
    CHAR_HORZ_WS  = 0x01,  // ' ', '\t', '\f', '\v'. Note, no '\0'
    CHAR_VERT_WS  = 0x02,  // '\r', '\n'
    CHAR_LETTER   = 0x04,  // a-z, A-Z
    CHAR_NUMBER   = 0x08,  // 0-9
    CHAR_UNDER    = 0x10,  // _
    CHAR_PERIOD   = 0x20   // .
} CharFlags;

static const unsigned char charInfo[256] = {
// 0 NUL         1 SOH         2 STX         3 ETX
// 4 EOT         5 ENQ         6 ACK         7 BEL
   0           , 0           , 0           , 0           ,
   0           , 0           , 0           , 0           ,
// 8 BS          9 HT         10 NL         11 VT
//12 NP         13 CR         14 SO         15 SI
   0           , CHAR_HORZ_WS, CHAR_VERT_WS, CHAR_HORZ_WS,
   CHAR_HORZ_WS, CHAR_VERT_WS, 0           , 0           ,
//16 DLE        17 DC1        18 DC2        19 DC3
//20 DC4        21 NAK        22 SYN        23 ETB
   0           , 0           , 0           , 0           ,
   0           , 0           , 0           , 0           ,
//24 CAN        25 EM         26 SUB        27 ESC
//28 FS         29 GS         30 RS         31 US
   0           , 0           , 0           , 0           ,
   0           , 0           , 0           , 0           ,
//32 SP         33  !         34  "         35  #
//36  $         37  %         38  &         39  '
   CHAR_HORZ_WS, 0           , 0           , 0           ,
   0           , 0           , 0           , 0           ,
//40  (         41  )         42  *         43  +
//44  ,         45  -         46  .         47  /
   0           , 0           , 0           , 0           ,
   0           , 0           , CHAR_PERIOD , 0           ,
//48  0         49  1         50  2         51  3
//52  4         53  5         54  6         55  7
   CHAR_NUMBER , CHAR_NUMBER , CHAR_NUMBER , CHAR_NUMBER ,
   CHAR_NUMBER , CHAR_NUMBER , CHAR_NUMBER , CHAR_NUMBER ,
//56  8         57  9         58  :         59  ;
//60  <         61  =         62  >         63  ?
   CHAR_NUMBER , CHAR_NUMBER , 0           , 0           ,
   0           , 0           , 0           , 0           ,
//64  @         65  A         66  B         67  C
//68  D         69  E         70  F         71  G
   0           , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER ,
   CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER ,
//72  H         73  I         74  J         75  K
//76  L         77  M         78  N         79  O
   CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER ,
   CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER ,
//80  P         81  Q         82  R         83  S
//84  T         85  U         86  V         87  W
   CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER ,
   CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER ,
//88  X         89  Y         90  Z         91  [
//92  \         93  ]         94  ^         95  _
   CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , 0           ,
   0           , 0           , 0           , CHAR_UNDER  ,
//96  `         97  a         98  b         99  c
//100  d       101  e        102  f        103  g
   0           , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER ,
   CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER ,
//104  h       105  i        106  j        107  k
//108  l       109  m        110  n        111  o
   CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER ,
   CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER ,
//112  p       113  q        114  r        115  s
//116  t       117  u        118  v        119  w
   CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER ,
   CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER ,
//120  x       121  y        122  z        123  {
//124  |       125  }        126  ~        127 DEL
   CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , 0           ,
   0           , 0           , 0           , 0
};

static inline _Bool isIdentifierBody(unsigned char c) {
    return (charInfo[c] & (CHAR_LETTER | CHAR_NUMBER | CHAR_UNDER)) ? 1 : 0;
}

static inline _Bool isWhitespace(unsigned char c) {
    return (charInfo[c] & (CHAR_HORZ_WS | CHAR_VERT_WS)) ? 1 : 0;
}

static inline _Bool isNumberBody(unsigned char c) {
    return (charInfo[c] & (CHAR_LETTER | CHAR_NUMBER | CHAR_UNDER | CHAR_PERIOD)) ? 1 : 0;
}

void initLexer(Lexer *lexer) {
}

void lex(Lexer *lexer, Token *result) {
    startToken(result);
    lexTokenInternal(lexer, result);
}

void lexTokenInternal(Lexer *lexer, Token *result) {
    // curPtr - Cache bufferPtr in an automatic variable.
    const char *curPtr = lexer->bufferPtr;

    // Small amounts of horizontal whitespace is very common between tokens.
    if ((*curPtr == ' ') || (*curPtr == '\t')) {
        do {
            ++curPtr;
        } while ((*curPtr == ' ') || (*curPtr == '\t'));
        lexer->bufferPtr = curPtr;
    }

    char ch = getAndAdvanceChar(&curPtr, result);
    TokenKind kind = TK_UNKNOWN;

    switch (ch) {
    case 0:
        if (curPtr - 1 == lexer->bufferEnd) {
        }
    case '\n':
    case '\r':

    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
        return lexNumericConstant(lexer, result, curPtr);

    case 'L':

    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
    case 'H': case 'I': case 'J': case 'K':    /*'L'*/case 'M': case 'N':
    case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
    case 'V': case 'W': case 'X': case 'Y': case 'Z':
    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
    case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
    case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
    case 'v': case 'w': case 'x': case 'y': case 'z':
    case '_':
        return ;

    default:
        return ;
    }
}

char getCharAndSize(const char *ptr, unsigned *size) {
    // If this is not a UCN or escaped newline, return quickly.
    if (ptr[0] != '\\') {
        *size = 1;
        return *ptr;
    }

    *size = 0;
    return getCharAndSizeSlow(ptr, size, 0);
}

char getCharAndSizeSlow(const char *ptr, unsigned *size, Token *tok) {
    return 0;
}

const char *consumeChar(const char *ptr, unsigned int size, Token *tok) {
    if (size == 1) 
        return ptr + 1;
    
    size = 0;
    getCharAndSizeSlow(ptr, &size, tok);
    return ptr + size;
}

char getAndAdvanceChar(const char **pptr, Token *tok) {
    char ch = **pptr;
    // If this is not a UCN or escaped newline, return quickly.
    if (ch != '\\') {
        *pptr += 1;
        return ch;
    }
    
    unsigned size = 0;
    ch = getCharAndSizeSlow(*pptr, &size, tok);
    *pptr += size;
    return ch;
}

// skipWhitespace - Efficiently skip over a series of whitespace characters.
// Update bufferPtr to point to the next non-whitespace character and return.
void skipWhitespace(Lexer *lexer, const char *curPtr) {
    unsigned char ch = *curPtr;
    while (isWhitespace(ch))
        ch = *++curPtr;
    lexer->bufferPtr = curPtr;
}

// skipLineComment - We have just read the // characters from input. Skip until
// we find the newline character that terminates the comment. Then update
// bufferPtr and return.
void skipLineComment(Lexer *lexer, const char *curPtr) {
    char ch = *curPtr;
    while (ch != 0 &&                 // Potentially EOF.
           ch != '\n' && ch != '\r')  // Newline or DOS-style newline.
        ch = *++curPtr;
    // If this is a newline, we're done.
    if (ch == '\n' || ch == '\r')
        ++curPtr;
    // Otherwise it is EOF.
    lexer->bufferPtr = curPtr;
}

void skipBlockComment(Lexer *lexer, const char *curPtr) {
    
}

// lexNumericConstant - Lex the remainder of a integer or floating point
// constant. From[-1] is the first character lexed. Return the end of the
// constant.
void lexNumericConstant(Lexer *lexer, Token *result, const char *curPtr) {
    unsigned size;
    char ch = getCharAndSize(curPtr, &size);
    char prevCh = 0;
    while (isNumberBody(ch)) {
        curPtr = consumeChar(curPtr, size, result);
        prevCh = ch;
        ch = getCharAndSize(curPtr, &size);
    }

    // If we fell out, check for a sign, due to 1e+12. If we have one, continue.
    if ((ch == '-' || ch == '+') && (prevCh == 'E' || prevCh == 'e'))
        return lexNumericConstant(lexer, result, consumeChar(curPtr, size, result));

    // If we have a hex FP constant, continue.
    if ((ch == '-' || ch == '+') && (prevCh == 'P' || prevCh == 'p'))
        return lexNumericConstant(lexer, result, consumeChar(curPtr, size, result));

    const char *tokStart = lexer->bufferPtr;
    formTokenWithChars(lexer, result, curPtr, TK_NUMERIC_CONSTANT);
    result->ptrData = (void *)tokStart;
}

void formTokenWithChars(Lexer *lexer, Token *result, const char *tokEnd, TokenKind kind) {
    unsigned tokLen = tokEnd - lexer->bufferPtr;
    result->length = tokLen;
    result->kind = kind;
    lexer->bufferPtr = tokEnd;
}