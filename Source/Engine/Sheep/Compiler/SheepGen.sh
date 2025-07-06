#!/bin/sh

# Executes flex and bison to regenerate Sheep Scanner/Parser files.
flex sheep.l
bison sheep.yy

# We don't want lex.yy.cc to use "register" keyword, which is deprecated in C++11 and removed in C++17.
# So, we'll just replace it entirely.
sed -i '' 's/register//g' lex.yy.cc

# Some generated code uses "if (false)" to pacify GCC...but then clang generates a warning about unreachable code!
# We DO want to keep this code unreachable, but using double-parentheses makes it explicit and suppresses the warning.
# Leading spaces are to avoid matching another 'if (false)' that we DON'T want to change :P
sed -i '' 's/    if (false)/    if ((false))/g' sheep.tab.cc