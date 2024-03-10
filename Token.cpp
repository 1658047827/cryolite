#include "Token.h"
#include "Diagnostic.h"
#include <algorithm>
#include <iostream>

std::string srcLocToStr(const SourceLocation &loc) {
    std::string s(*loc.filename);
    s += ":";
    s += std::to_string(loc.line);
    s += ":";
    s += std::to_string(loc.column);
    s += ": ";
    return s;
}

std::string srcLocToPos(const SourceLocation &loc) {
    std::string s("line:");
    s += std::to_string(loc.line);
    s += ", col:";
    s += std::to_string(loc.column);
    return s;
}

TokenKind Token::getKind() const {
    return kind;
}

SourceLocation Token::getLoc() const {
    return loc;
}

std::string Token::getStr() const {
    return str;
}

std::string Token::toString() const {
    if (str.empty()) {
        auto iter = tokenKind2Str.find(kind);
        if (iter != tokenKind2Str.cend())
            return iter->second;
        return "";
    } else {
        return str;
    }
}

void TokenSequence::pushBack(Token &token) {
    tokenList.push_back(std::make_unique<Token>(token));
}

void TokenSequence::print(std::ostream &out) {
    for (auto &tkp : tokenList) {
        out << tkp->toString() << ' ';
    }
    out << std::endl;
}

TokenSeqIter TokenSequence::begin() {
    return tokenList.begin();
}

TokenSeqIter TokenSequence::end() {
    return tokenList.end();
}

TokenSeqConstIter TokenSequence::cBegin() {
    return tokenList.cbegin();
}

TokenSeqConstIter TokenSequence::cEnd() {
    return tokenList.cend();
}

/**
 * integer-constant: [C99 6.4.4.1]
 *     decimal-constant integer-suffix{opt}
 *     octal-constant integer-suffix{opt}
 *     hexadecimal-constant integer-suffix{opt}
 *
 * decimal-constant:
 *     nonzero-digit
 *     decimal-constant digit
 *
 * octal-constant:
 *     0
 *     octal-constant octal-digit
 *
 * hexadecimal-constant:
 *     hexadecimal-prefix hexadecimal-digit
 *     hexadecimal-constant hexadecimal-digit
 *
 * hexadecimal-prefix: one of
 *     0x 0X
 *
 * integer-suffix:
 *     unsigned-suffix long-suffix{opt}
 *     unsigned-suffix long-long-suffix{opt}
 *     long-suffix unsigned-suffix{opt}
 *     long-long-suffix unsigned-sufix{opt}
 *
 * nonzero-digit: one of
 *     1 2 3 4 5 6 7 8 9
 *
 * octal-digit: one of
 *     0 1 2 3 4 5 6 7
 *
 * hexadecimal-digit: one of
 *     0 1 2 3 4 5 6 7 8 9
 *     a b c d e f
 *     A B C D E F
 *
 * unsigned-suffix: one of
 *     u U
 *
 * long-suffix: one of
 *     l L
 *
 * long-long-suffix: one of
 *     ll LL
 *
 * floating-constant: [C99 6.4.4.2]
 *     decimal-floating-constant
 *     hexadecimal-floating-constant
 *
 * decimal-floating-constant:
 *     fractional-constant exponent-part{opt} floating-suffix{opt}
 *     digit-sequence exponent-part floating-suffix{opt}
 *
 * hexadecimal-floating-constant:
 *     hexadecimal-prefix hexadecimal-fractional-constant binary-exponent-part floating-suffix{opt}
 *     hexadecimal-prefix hexadecimal-digit-sequence binary-exponent-part floating-suffix{opt}
 *
 * fractional-constant:
 *     digit-sequence{opt} . digit-sequence
 *     digit-sequence .
 *
 * exponent-part:
 *     e sign{opt} digit-sequence
 *     E sign{opt} digit-sequence
 *
 * sign: one of
 *     + -
 *
 * digit-sequence:
 *     digit
 *     digit-sequence digit
 *
 * hexadecimal-fractional-constant:
 *     hexadecimal-digit-sequence{opt} . hexadecimal-digit-sequence
 *     hexadecimal-digit-sequence .
 *
 * binary-exponent-part:
 *     p sign{opt} digit-sequence
 *     P sign{opt} digit-sequence
 *
 * hexadecimal-digit-sequence:
 *     hexadecimal-digit
 *     hexadecimal-digit-sequence hexadecimal-digit
 *
 * floating-suffix: one of
 *     f l F L
 */
NumericLiteralParser::NumericLiteralParser(const char *begin, const char *end, const SourceLocation &loc)
    : thisTokBegin(begin), thisTokEnd(end) {
    s = digitsBegin = begin;
    sawExponent = false;
    sawPeriod = false;
    isLong = false;
    isUnsigned = false;
    isLongLong = false;
    isFloat = false;
    hadError = false;

    if (*s == '0') { // parse radix
        parseNumberStartingWithZero(loc);
        if (hadError)
            return;
    } else { // the first digit is non-zero
        radix = 10;
        s = skipDigits(s);
        if (s == thisTokEnd) {
            // Done
        } else if (std::isxdigit(*s) && !(*s == 'e' || *s == 'E')) {
            error(loc, "invalid decimal digit");
            hadError = true;
            return;
        } else if (*s == '.') {
            ++s;
            sawPeriod = true;
            s = skipDigits(s);
        }
        if (*s == 'e' || *s == 'E') { // exponent
            const char *exponent = s;
            ++s;
            sawExponent = true;
            if (*s == '+' || *s == '-') ++s; // sign
            const char *first_non_digit = skipDigits(s);
            if (first_non_digit != s) {
                s = first_non_digit;
            } else {
                error(loc, "exponent has no digits");
                hadError = true;
                return;
            }
        }
    }

    suffixBegin = s;

    // Parse the suffix.
    // At this point we can classify whether we have an FP or integer constant.
    bool isFPConstant = isFloatingLiteral();

    // Loop over all of the characters of the suffix.
    // If we see something bad, we break out of the loop.
    for (; s != thisTokEnd; ++s) {
        switch (*s) {
        case 'f': // FP Suffix for "float"
        case 'F':
            if (!isFPConstant) break;     // Error for integer constant.
            if (isFloat || isLong) break; // FF, LF invalid.
            isFloat = true;
            continue; // Success.
        case 'u':
        case 'U':
            if (isFPConstant) break; // Error for floating constant.
            if (isUnsigned) break;   // Cannot be repeated.
            isUnsigned = true;
            continue; // Success.
        case 'l':
        case 'L':
            if (isLong || isLongLong) break; // Cannot be repeated.
            if (isFloat) break;              // LF invalid.

            // Check for long long.  The L's need to be adjacent and the same case.
            if (s + 1 != thisTokEnd && s[1] == s[0]) {
                if (isFPConstant) break; // long long invalid for floats.
                isLongLong = true;
                ++s; // Eat both of them.
            } else {
                isLong = true;
            }
            continue; // Success.
        default:
            break;
        }
        // If we reached here, there was an error.
        break;
    }

    if (s != thisTokEnd) {
        if (isFPConstant)
            error(loc, "invalid suffix floating constant");
        else
            error(loc, "invalid suffix integer constant");
        hadError = true;
    }
}

// parseNumberStartingWithZero - Called when first char is '0'.
// '04' '0x123a' '0b1010' '01239.123e4'
void NumericLiteralParser::parseNumberStartingWithZero(SourceLocation loc) {
    ++s; // Consume the '0'.

    // Handle a hex number like 0x1234.
    if ((*s == 'x' || *s == 'X') && (isxdigit(s[1]) || s[1] == '.')) {
        s++;
        radix = 16;
        digitsBegin = s;
        s = skipHexDigits(s);
        if (s == thisTokEnd) {
            // Done.
        } else if (*s == '.') {
            s++;
            sawPeriod = true;
            s = skipHexDigits(s);
        }
        // A binary exponent can appear with or with a '.'. If dotted, the
        // binary exponent is required.
        if (*s == 'p' || *s == 'P') {
            const char *Exponent = s;
            s++;
            sawExponent = true;
            if (*s == '+' || *s == '-') s++; // sign
            const char *first_non_digit = skipDigits(s);
            if (first_non_digit == s) {
                error(loc, "exponent has no digits");
                hadError = true;
                return;
            }
            s = first_non_digit;
        } else if (sawPeriod) {
            error(loc, "hexconstant requires exponent");
            hadError = true;
        }
        return;
    }

    // Handle simple binary numbers 0b01010
    if (*s == 'b' || *s == 'B') {
        // 0b101010 is a GCC extension.
        warning(loc, "using binary literal");
        ++s;
        radix = 2;
        digitsBegin = s;
        s = SkipBinaryDigits(s);
        if (s == thisTokEnd) {
            // Done.
        } else if (isxdigit(*s)) {
            error(loc, "invalid binary digit");
            hadError = true;
        }
        // Other suffixes will be diagnosed by the caller.
        return;
    }

    // For now, the radix is set to 8. If we discover that we have a
    // floating point constant, the radix will change to 10. Octal floating
    // point constants are not permitted (only decimal and hexadecimal).
    radix = 8;
    digitsBegin = s;
    s = skipOctalDigits(s);
    if (s == thisTokEnd)
        return; // Done, simple octal number like 01234

    // If we have some other non-octal digit that *is* a decimal digit, see if
    // this is part of a floating point number like 094.123 or 09e1.
    if (std::isdigit(*s)) {
        const char *EndDecimal = skipDigits(s);
        if (EndDecimal[0] == '.' || EndDecimal[0] == 'e' || EndDecimal[0] == 'E') {
            s = EndDecimal;
            radix = 10;
        }
    }

    // If we have a hex digit other than 'e' (which denotes a FP exponent) then
    // the code is using an incorrect base.
    if (std::isxdigit(*s) && *s != 'e' && *s != 'E') {
        error(loc, "invalid octal digit");
        hadError = true;
        return;
    }

    if (*s == '.') {
        s++;
        radix = 10;
        sawPeriod = true;
        s = skipDigits(s); // Skip suffix.
    }
    if (*s == 'e' || *s == 'E') { // exponent
        const char *Exponent = s;
        s++;
        radix = 10;
        sawExponent = true;
        if (*s == '+' || *s == '-') s++; // sign
        const char *first_non_digit = skipDigits(s);
        if (first_non_digit != s) {
            s = first_non_digit;
        } else {
            error(loc, "exponent has no digits");
            hadError = true;
            return;
        }
    }
}
