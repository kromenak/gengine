// A Bison parser, made by GNU Bison 3.0.4.

// Skeleton implementation for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2015 Free Software Foundation, Inc.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// As a special exception, you may create a larger work that contains
// part or all of the Bison parser skeleton and distribute that work
// under terms of your choice, so long as that work isn't itself a
// parser generator using the skeleton or a modified version thereof
// as a parser skeleton.  Alternatively, if you modify or redistribute
// the parser skeleton itself, you may (at your option) remove this
// special exception, which will cause the skeleton and the resulting
// Bison output files to be licensed under the GNU General Public
// License without this special exception.

// This special exception was added by the Free Software Foundation in
// version 2.2 of Bison.


// First part of user declarations.
#line 1 "Sheep.yy" // lalr1.cc:404

#include <cstdlib>
#include <string>
#include "SheepScript.h"
#define YYSTYPE SheepNode*

char* removeQuotes(char* str)
{
	if(str[0] == '"')
	{
		str[strlen(str)-1] = 0;
		return str+1;
	}
	else // assume "|< >|" strings
	{
		str[strlen(str)-2] = 0;
		return str+2;
	}
}

#line 57 "Sheep.tab.cc" // lalr1.cc:404

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

#include "Sheep.tab.hh"

// User implementation prologue.

#line 71 "Sheep.tab.cc" // lalr1.cc:412
// Unqualified %code blocks.
#line 59 "Sheep.yy" // lalr1.cc:413

	#include "SheepDriver.h"

	// When requesting yylex, redirect to scanner.yylex.
	#undef yylex
	#define yylex scanner.yylex

	// The C++ yytext is const, but we need to modify to remove
	// quotes in some cases. So, this is a bit of a hack.
	#undef yytext
	#define yytext scanner.GetYYText()

	void Sheep::Parser::error(const location_type& loc, const std::string& msg)
	{
		driver.error(loc, msg);
	}

#line 91 "Sheep.tab.cc" // lalr1.cc:413


#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> // FIXME: INFRINGES ON USER NAME SPACE.
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K].location)
/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

# ifndef YYLLOC_DEFAULT
#  define YYLLOC_DEFAULT(Current, Rhs, N)                               \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).begin  = YYRHSLOC (Rhs, 1).begin;                   \
          (Current).end    = YYRHSLOC (Rhs, N).end;                     \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;      \
        }                                                               \
    while (/*CONSTCOND*/ false)
# endif


// Suppress unused-variable warnings by "using" E.
#define YYUSE(E) ((void) (E))

// Enable debugging if requested.
#if YYDEBUG

// A pseudo ostream that takes yydebug_ into account.
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Symbol)         \
  do {                                          \
    if (yydebug_)                               \
    {                                           \
      *yycdebug_ << Title << ' ';               \
      yy_print_ (*yycdebug_, Symbol);           \
      *yycdebug_ << std::endl;                  \
    }                                           \
  } while (false)

# define YY_REDUCE_PRINT(Rule)          \
  do {                                  \
    if (yydebug_)                       \
      yy_reduce_print_ (Rule);          \
  } while (false)

# define YY_STACK_PRINT()               \
  do {                                  \
    if (yydebug_)                       \
      yystack_print_ ();                \
  } while (false)

#else // !YYDEBUG

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Symbol)  YYUSE(Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void>(0)
# define YY_STACK_PRINT()                static_cast<void>(0)

#endif // !YYDEBUG

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyla.clear ())

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)

#line 32 "Sheep.yy" // lalr1.cc:479
namespace Sheep {
#line 177 "Sheep.tab.cc" // lalr1.cc:479

  /// Build a parser object.
  Parser::Parser (Sheep::Scanner& scanner_yyarg, Sheep::Driver& driver_yyarg)
    :
#if YYDEBUG
      yydebug_ (false),
      yycdebug_ (&std::cerr),
#endif
      scanner (scanner_yyarg),
      driver (driver_yyarg)
  {}

  Parser::~Parser ()
  {}


  /*---------------.
  | Symbol types.  |
  `---------------*/



  // by_state.
  inline
  Parser::by_state::by_state ()
    : state (empty_state)
  {}

  inline
  Parser::by_state::by_state (const by_state& other)
    : state (other.state)
  {}

  inline
  void
  Parser::by_state::clear ()
  {
    state = empty_state;
  }

  inline
  void
  Parser::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

  inline
  Parser::by_state::by_state (state_type s)
    : state (s)
  {}

  inline
  Parser::symbol_number_type
  Parser::by_state::type_get () const
  {
    if (state == empty_state)
      return empty_symbol;
    else
      return yystos_[state];
  }

  inline
  Parser::stack_symbol_type::stack_symbol_type ()
  {}


  inline
  Parser::stack_symbol_type::stack_symbol_type (state_type s, symbol_type& that)
    : super_type (s, that.location)
  {
      switch (that.type_get ())
    {
      default:
        break;
    }

    // that is emptied.
    that.type = empty_symbol;
  }

  inline
  Parser::stack_symbol_type&
  Parser::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
      switch (that.type_get ())
    {
      default:
        break;
    }

    location = that.location;
    return *this;
  }


  template <typename Base>
  inline
  void
  Parser::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);
  }

#if YYDEBUG
  template <typename Base>
  void
  Parser::yy_print_ (std::ostream& yyo,
                                     const basic_symbol<Base>& yysym) const
  {
    std::ostream& yyoutput = yyo;
    YYUSE (yyoutput);
    symbol_number_type yytype = yysym.type_get ();
    // Avoid a (spurious) G++ 4.8 warning about "array subscript is
    // below array bounds".
    if (yysym.empty ())
      std::abort ();
    yyo << (yytype < yyntokens_ ? "token" : "nterm")
        << ' ' << yytname_[yytype] << " ("
        << yysym.location << ": ";
    YYUSE (yytype);
    yyo << ')';
  }
#endif

  inline
  void
  Parser::yypush_ (const char* m, state_type s, symbol_type& sym)
  {
    stack_symbol_type t (s, sym);
    yypush_ (m, t);
  }

  inline
  void
  Parser::yypush_ (const char* m, stack_symbol_type& s)
  {
    if (m)
      YY_SYMBOL_PRINT (m, s);
    yystack_.push (s);
  }

  inline
  void
  Parser::yypop_ (unsigned int n)
  {
    yystack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
  Parser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  Parser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  Parser::debug_level_type
  Parser::debug_level () const
  {
    return yydebug_;
  }

  void
  Parser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif // YYDEBUG

  inline Parser::state_type
  Parser::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - yyntokens_] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - yyntokens_];
  }

  inline bool
  Parser::yy_pact_value_is_default_ (int yyvalue)
  {
    return yyvalue == yypact_ninf_;
  }

  inline bool
  Parser::yy_table_value_is_error_ (int yyvalue)
  {
    return yyvalue == yytable_ninf_;
  }

  int
  Parser::parse ()
  {
    // State.
    int yyn;
    /// Length of the RHS of the rule being reduced.
    int yylen = 0;

    // Error handling.
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// The lookahead symbol.
    symbol_type yyla;

    /// The locations where the error started and ended.
    stack_symbol_type yyerror_range[3];

    /// The return value of parse ().
    int yyresult;

    // FIXME: This shoud be completely indented.  It is not yet to
    // avoid gratuitous conflicts when merging into the master branch.
    try
      {
    YYCDEBUG << "Starting parse" << std::endl;


    /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystack_.clear ();
    yypush_ (YY_NULLPTR, 0, yyla);

    // A new symbol was pushed on the stack.
  yynewstate:
    YYCDEBUG << "Entering state " << yystack_[0].state << std::endl;

    // Accept?
    if (yystack_[0].state == yyfinal_)
      goto yyacceptlab;

    goto yybackup;

    // Backup.
  yybackup:

    // Try to take a decision without lookahead.
    yyn = yypact_[yystack_[0].state];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    // Read a lookahead token.
    if (yyla.empty ())
      {
        YYCDEBUG << "Reading a token: ";
        try
          {
            symbol_type yylookahead (yylex (scanner, driver));
            yyla.move (yylookahead);
          }
        catch (const syntax_error& yyexc)
          {
            error (yyexc);
            goto yyerrlab1;
          }
      }
    YY_SYMBOL_PRINT ("Next token is", yyla);

    /* If the proper action on seeing token YYLA.TYPE is to reduce or
       to detect an error, take that action.  */
    yyn += yyla.type_get ();
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.type_get ())
      goto yydefault;

    // Reduce or error.
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
        if (yy_table_value_is_error_ (yyn))
          goto yyerrlab;
        yyn = -yyn;
        goto yyreduce;
      }

    // Count tokens shifted since error; after three, turn off error status.
    if (yyerrstatus_)
      --yyerrstatus_;

    // Shift the lookahead token.
    yypush_ ("Shifting", yyn, yyla);
    goto yynewstate;

  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[yystack_[0].state];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;

  /*-----------------------------.
  | yyreduce -- Do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    {
      stack_symbol_type yylhs;
      yylhs.state = yy_lr_goto_state_(yystack_[yylen].state, yyr1_[yyn]);
      /* Variants are always initialized to an empty instance of the
         correct type. The default '$$ = $1' action is NOT applied
         when using variants.  */
        switch (yyr1_[yyn])
    {
      default:
        break;
    }


      // Compute the default @$.
      {
        slice<stack_symbol_type, stack_type> slice (yystack_, yylen);
        YYLLOC_DEFAULT (yylhs.location, slice, yylen);
      }

      // Perform the reduction.
      YY_REDUCE_PRINT (yyn);
      try
        {
          switch (yyn)
            {
  case 3:
#line 140 "Sheep.yy" // lalr1.cc:859
    { }
#line 515 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 4:
#line 141 "Sheep.yy" // lalr1.cc:859
    { }
#line 521 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 5:
#line 142 "Sheep.yy" // lalr1.cc:859
    { }
#line 527 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 6:
#line 146 "Sheep.yy" // lalr1.cc:859
    { }
#line 533 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 7:
#line 147 "Sheep.yy" // lalr1.cc:859
    { }
#line 539 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 8:
#line 151 "Sheep.yy" // lalr1.cc:859
    { }
#line 545 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 9:
#line 152 "Sheep.yy" // lalr1.cc:859
    { }
#line 551 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 10:
#line 156 "Sheep.yy" // lalr1.cc:859
    { }
#line 557 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 11:
#line 157 "Sheep.yy" // lalr1.cc:859
    { }
#line 563 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 12:
#line 162 "Sheep.yy" // lalr1.cc:859
    { }
#line 569 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 13:
#line 167 "Sheep.yy" // lalr1.cc:859
    { }
#line 575 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 14:
#line 168 "Sheep.yy" // lalr1.cc:859
    { }
#line 581 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 15:
#line 169 "Sheep.yy" // lalr1.cc:859
    { }
#line 587 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 16:
#line 170 "Sheep.yy" // lalr1.cc:859
    { }
#line 593 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 17:
#line 174 "Sheep.yy" // lalr1.cc:859
    { }
#line 599 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 18:
#line 175 "Sheep.yy" // lalr1.cc:859
    { }
#line 605 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 19:
#line 178 "Sheep.yy" // lalr1.cc:859
    { }
#line 611 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 20:
#line 181 "Sheep.yy" // lalr1.cc:859
    { }
#line 617 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 21:
#line 182 "Sheep.yy" // lalr1.cc:859
    { }
#line 623 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 22:
#line 185 "Sheep.yy" // lalr1.cc:859
    { }
#line 629 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 23:
#line 188 "Sheep.yy" // lalr1.cc:859
    { }
#line 635 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 24:
#line 189 "Sheep.yy" // lalr1.cc:859
    { }
#line 641 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 25:
#line 192 "Sheep.yy" // lalr1.cc:859
    { }
#line 647 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 26:
#line 193 "Sheep.yy" // lalr1.cc:859
    { }
#line 653 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 27:
#line 196 "Sheep.yy" // lalr1.cc:859
    { }
#line 659 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 28:
#line 197 "Sheep.yy" // lalr1.cc:859
    { }
#line 665 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 29:
#line 200 "Sheep.yy" // lalr1.cc:859
    { }
#line 671 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 30:
#line 201 "Sheep.yy" // lalr1.cc:859
    { }
#line 677 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 31:
#line 204 "Sheep.yy" // lalr1.cc:859
    { }
#line 683 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 32:
#line 205 "Sheep.yy" // lalr1.cc:859
    { }
#line 689 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 33:
#line 208 "Sheep.yy" // lalr1.cc:859
    { }
#line 695 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 34:
#line 209 "Sheep.yy" // lalr1.cc:859
    { }
#line 701 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 35:
#line 210 "Sheep.yy" // lalr1.cc:859
    { }
#line 707 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 36:
#line 211 "Sheep.yy" // lalr1.cc:859
    { }
#line 713 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 37:
#line 214 "Sheep.yy" // lalr1.cc:859
    { }
#line 719 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 38:
#line 215 "Sheep.yy" // lalr1.cc:859
    { }
#line 725 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 39:
#line 216 "Sheep.yy" // lalr1.cc:859
    { }
#line 731 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 40:
#line 217 "Sheep.yy" // lalr1.cc:859
    { }
#line 737 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 41:
#line 218 "Sheep.yy" // lalr1.cc:859
    { }
#line 743 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 42:
#line 219 "Sheep.yy" // lalr1.cc:859
    { }
#line 749 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 43:
#line 220 "Sheep.yy" // lalr1.cc:859
    { }
#line 755 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 44:
#line 221 "Sheep.yy" // lalr1.cc:859
    { }
#line 761 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 45:
#line 224 "Sheep.yy" // lalr1.cc:859
    { }
#line 767 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 46:
#line 225 "Sheep.yy" // lalr1.cc:859
    { }
#line 773 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 48:
#line 229 "Sheep.yy" // lalr1.cc:859
    { yylhs.value = yystack_[2].value; }
#line 779 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 49:
#line 230 "Sheep.yy" // lalr1.cc:859
    { yylhs.value = yystack_[3].value; yylhs.value->SetChild(0, yystack_[1].value); }
#line 785 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 50:
#line 233 "Sheep.yy" // lalr1.cc:859
    { yylhs.value = yystack_[0].value; }
#line 791 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 51:
#line 234 "Sheep.yy" // lalr1.cc:859
    { yylhs.value->SetSibling(yystack_[0].value); }
#line 797 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 52:
#line 237 "Sheep.yy" // lalr1.cc:859
    { yylhs.value = yystack_[0].value; }
#line 803 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 53:
#line 238 "Sheep.yy" // lalr1.cc:859
    { yylhs.value->SetSibling(yystack_[0].value); }
#line 809 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 54:
#line 241 "Sheep.yy" // lalr1.cc:859
    { }
#line 815 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 55:
#line 242 "Sheep.yy" // lalr1.cc:859
    { yylhs.value = yystack_[0].value; }
#line 821 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 56:
#line 243 "Sheep.yy" // lalr1.cc:859
    { yylhs.value = yystack_[0].value; }
#line 827 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 57:
#line 244 "Sheep.yy" // lalr1.cc:859
    { yylhs.value = yystack_[1].value; }
#line 833 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 58:
#line 245 "Sheep.yy" // lalr1.cc:859
    { yylhs.value = SheepNode::CreateOperation(SheepOperationType::Not); 
		yylhs.value->SetChild(0, yystack_[1].value); }
#line 840 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 59:
#line 247 "Sheep.yy" // lalr1.cc:859
    { yylhs.value = SheepNode::CreateOperation(SheepOperationType::Negate); 
		yylhs.value->SetChild(0, yystack_[1].value); }
#line 847 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 60:
#line 249 "Sheep.yy" // lalr1.cc:859
    { yylhs.value = SheepNode::CreateOperation(SheepOperationType::Add); 
		yylhs.value->SetChild(0, yystack_[2].value); yylhs.value->SetChild(1, yystack_[0].value); }
#line 854 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 61:
#line 251 "Sheep.yy" // lalr1.cc:859
    { yylhs.value = SheepNode::CreateOperation(SheepOperationType::Subtract); 
		yylhs.value->SetChild(0, yystack_[2].value); yylhs.value->SetChild(1, yystack_[0].value); }
#line 861 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 62:
#line 253 "Sheep.yy" // lalr1.cc:859
    { yylhs.value = SheepNode::CreateOperation(SheepOperationType::Multiply); 
		yylhs.value->SetChild(0, yystack_[2].value); yylhs.value->SetChild(1, yystack_[0].value); }
#line 868 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 63:
#line 255 "Sheep.yy" // lalr1.cc:859
    { yylhs.value = SheepNode::CreateOperation(SheepOperationType::Divide); 
		yylhs.value->SetChild(0, yystack_[2].value); yylhs.value->SetChild(1, yystack_[0].value); }
#line 875 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 64:
#line 257 "Sheep.yy" // lalr1.cc:859
    { yylhs.value = SheepNode::CreateOperation(SheepOperationType::LessThan); 
		yylhs.value->SetChild(0, yystack_[2].value); yylhs.value->SetChild(1, yystack_[0].value); }
#line 882 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 65:
#line 259 "Sheep.yy" // lalr1.cc:859
    { yylhs.value = SheepNode::CreateOperation(SheepOperationType::LessThanOrEqual); 
		yylhs.value->SetChild(0, yystack_[2].value); yylhs.value->SetChild(1, yystack_[0].value); }
#line 889 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 66:
#line 261 "Sheep.yy" // lalr1.cc:859
    { yylhs.value = SheepNode::CreateOperation(SheepOperationType::GreaterThan); 
		yylhs.value->SetChild(0, yystack_[2].value); yylhs.value->SetChild(1, yystack_[0].value); }
#line 896 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 67:
#line 263 "Sheep.yy" // lalr1.cc:859
    { yylhs.value = SheepNode::CreateOperation(SheepOperationType::GreaterThanOrEqual); 
		yylhs.value->SetChild(0, yystack_[2].value); yylhs.value->SetChild(1, yystack_[0].value); }
#line 903 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 68:
#line 265 "Sheep.yy" // lalr1.cc:859
    { yylhs.value = SheepNode::CreateOperation(SheepOperationType::AreEqual); 
		yylhs.value->SetChild(0, yystack_[2].value); yylhs.value->SetChild(1, yystack_[0].value); }
#line 910 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 69:
#line 267 "Sheep.yy" // lalr1.cc:859
    { yylhs.value = SheepNode::CreateOperation(SheepOperationType::NotEqual); 
		yylhs.value->SetChild(0, yystack_[2].value); yylhs.value->SetChild(1, yystack_[0].value); }
#line 917 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 70:
#line 269 "Sheep.yy" // lalr1.cc:859
    { yylhs.value = SheepNode::CreateOperation(SheepOperationType::LogicalOr); 
		yylhs.value->SetChild(0, yystack_[2].value); yylhs.value->SetChild(1, yystack_[0].value); }
#line 924 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 71:
#line 271 "Sheep.yy" // lalr1.cc:859
    { yylhs.value = SheepNode::CreateOperation(SheepOperationType::LogicalAnd); 
		yylhs.value->SetChild(0, yystack_[2].value); yylhs.value->SetChild(1, yystack_[0].value); }
#line 931 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 72:
#line 276 "Sheep.yy" // lalr1.cc:859
    { yylhs.value = SheepNode::CreateIntegerConstant(atoi(yytext)); }
#line 937 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 73:
#line 277 "Sheep.yy" // lalr1.cc:859
    { yylhs.value = SheepNode::CreateFloatConstant(atof(yytext)); }
#line 943 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 74:
#line 278 "Sheep.yy" // lalr1.cc:859
    { yylhs.value = SheepNode::CreateStringConstant(removeQuotes(yytext)); }
#line 949 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 75:
#line 282 "Sheep.yy" // lalr1.cc:859
    { yylhs.value = SheepNode::CreateTypeRef(SheepReferenceType::Int); }
#line 955 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 76:
#line 283 "Sheep.yy" // lalr1.cc:859
    { yylhs.value = SheepNode::CreateTypeRef(SheepReferenceType::Float); }
#line 961 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 77:
#line 284 "Sheep.yy" // lalr1.cc:859
    { yylhs.value = SheepNode::CreateTypeRef(SheepReferenceType::String); }
#line 967 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 78:
#line 287 "Sheep.yy" // lalr1.cc:859
    { yylhs.value = SheepNode::CreateNameRef(yytext, false); }
#line 973 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 79:
#line 289 "Sheep.yy" // lalr1.cc:859
    { yylhs.value = SheepNode::CreateNameRef(yytext, true); }
#line 979 "Sheep.tab.cc" // lalr1.cc:859
    break;


#line 983 "Sheep.tab.cc" // lalr1.cc:859
            default:
              break;
            }
        }
      catch (const syntax_error& yyexc)
        {
          error (yyexc);
          YYERROR;
        }
      YY_SYMBOL_PRINT ("-> $$ =", yylhs);
      yypop_ (yylen);
      yylen = 0;
      YY_STACK_PRINT ();

      // Shift the result of the reduction.
      yypush_ (YY_NULLPTR, yylhs);
    }
    goto yynewstate;

  /*--------------------------------------.
  | yyerrlab -- here on detecting error.  |
  `--------------------------------------*/
  yyerrlab:
    // If not already recovering from an error, report this error.
    if (!yyerrstatus_)
      {
        ++yynerrs_;
        error (yyla.location, yysyntax_error_ (yystack_[0].state, yyla));
      }


    yyerror_range[1].location = yyla.location;
    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */

        // Return failure if at end of input.
        if (yyla.type_get () == yyeof_)
          YYABORT;
        else if (!yyla.empty ())
          {
            yy_destroy_ ("Error: discarding", yyla);
            yyla.clear ();
          }
      }

    // Else will try to reuse lookahead token after shifting the error token.
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:

    /* Pacify compilers like GCC when the user code never invokes
       YYERROR and the label yyerrorlab therefore never appears in user
       code.  */
    if (false)
      goto yyerrorlab;
    yyerror_range[1].location = yystack_[yylen - 1].location;
    /* Do not reclaim the symbols of the rule whose action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    goto yyerrlab1;

  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;   // Each real token shifted decrements this.
    {
      stack_symbol_type error_token;
      for (;;)
        {
          yyn = yypact_[yystack_[0].state];
          if (!yy_pact_value_is_default_ (yyn))
            {
              yyn += yyterror_;
              if (0 <= yyn && yyn <= yylast_ && yycheck_[yyn] == yyterror_)
                {
                  yyn = yytable_[yyn];
                  if (0 < yyn)
                    break;
                }
            }

          // Pop the current state because it cannot handle the error token.
          if (yystack_.size () == 1)
            YYABORT;

          yyerror_range[1].location = yystack_[0].location;
          yy_destroy_ ("Error: popping", yystack_[0]);
          yypop_ ();
          YY_STACK_PRINT ();
        }

      yyerror_range[2].location = yyla.location;
      YYLLOC_DEFAULT (error_token.location, yyerror_range, 2);

      // Shift the error token.
      error_token.state = yyn;
      yypush_ ("Shifting", error_token);
    }
    goto yynewstate;

    // Accept.
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;

    // Abort.
  yyabortlab:
    yyresult = 1;
    goto yyreturn;

  yyreturn:
    if (!yyla.empty ())
      yy_destroy_ ("Cleanup: discarding lookahead", yyla);

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    while (1 < yystack_.size ())
      {
        yy_destroy_ ("Cleanup: popping", yystack_[0]);
        yypop_ ();
      }

    return yyresult;
  }
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack"
                 << std::endl;
        // Do not try to display the values of the reclaimed symbols,
        // as their printer might throw an exception.
        if (!yyla.empty ())
          yy_destroy_ (YY_NULLPTR, yyla);

        while (1 < yystack_.size ())
          {
            yy_destroy_ (YY_NULLPTR, yystack_[0]);
            yypop_ ();
          }
        throw;
      }
  }

  void
  Parser::error (const syntax_error& yyexc)
  {
    error (yyexc.location, yyexc.what());
  }

  // Generate an error message.
  std::string
  Parser::yysyntax_error_ (state_type, const symbol_type&) const
  {
    return YY_("syntax error");
  }


  const signed char Parser::yypact_ninf_ = -127;

  const signed char Parser::yytable_ninf_ = -1;

  const short int
  Parser::yypact_[] =
  {
      43,   -19,    -1,    42,    47,  -127,    10,     6,  -127,  -127,
    -127,  -127,    55,    51,  -127,    61,  -127,  -127,  -127,  -127,
       9,    30,  -127,    51,  -127,  -127,    30,  -127,    30,  -127,
      52,    54,    64,    71,    51,    51,  -127,   118,    69,    30,
    -127,    63,  -127,  -127,  -127,  -127,   154,    51,   118,    74,
      51,   115,  -127,  -127,   176,    67,    31,   176,   176,    94,
    -127,  -127,  -127,  -127,  -127,  -127,   171,  -127,   -11,    80,
    -127,  -127,   176,    86,  -127,   195,  -127,   217,  -127,   133,
    -127,    92,    96,  -127,  -127,  -127,  -127,  -127,   176,   176,
     176,   176,   176,   176,   176,   176,   176,   176,   176,   176,
    -127,   176,   173,   239,  -127,  -127,  -127,  -127,  -127,    99,
    -127,   262,   272,   204,   204,    57,    57,    57,    57,    91,
      91,  -127,  -127,   251,  -127,   -13,   251,   154,    14,   176,
    -127,  -127,   110,  -127,  -127,   251,   154,  -127,  -127
  };

  const unsigned char
  Parser::yydefact_[] =
  {
       2,     0,     0,     0,     4,     5,     0,     0,     1,     3,
      78,    17,     0,    19,    20,     0,    75,    76,    77,     6,
       0,     8,    10,     0,    18,    21,    23,     7,     9,    11,
       0,    13,     0,    24,     0,     0,    12,     0,     0,     0,
      25,    15,    72,    73,    74,    14,     0,     0,     0,     0,
       0,     0,    79,    37,     0,     0,     0,     0,     0,     0,
      27,    29,    30,    33,    43,    55,     0,    54,    56,     0,
      26,    16,     0,     0,    41,     0,    56,     0,    34,     0,
      45,     0,     0,    58,    59,    22,    28,    40,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      38,     0,     0,     0,    39,    42,    57,    35,    50,     0,
      46,    70,    71,    68,    69,    64,    65,    66,    67,    60,
      61,    62,    63,    44,    48,     0,    52,     0,     0,     0,
      49,    31,    30,    47,    51,    53,     0,    32,    36
  };

  const short int
  Parser::yypgoto_[] =
  {
    -127,  -127,  -127,  -127,   109,    24,  -127,   134,  -127,  -127,
     124,  -127,  -127,    90,   -56,    12,  -126,  -127,  -127,   -54,
    -127,  -127,   -33,    -5,   -20,    -6,  -127
  };

  const signed char
  Parser::yydefgoto_[] =
  {
      -1,     3,     4,    20,    21,    22,    30,     5,    12,    13,
      14,    32,    33,    59,    60,    61,    62,    63,    64,    65,
     109,   125,    66,    67,    23,    76,    69
  };

  const unsigned char
  Parser::yytable_[] =
  {
      15,   132,    82,    86,     6,   129,    34,    15,   100,   130,
     138,    16,    17,    18,    16,    17,    18,    31,    75,    47,
     101,    77,     7,    86,    83,    84,    10,   108,    40,    41,
      19,    52,    45,    27,    11,    16,    17,    18,   133,   103,
      68,    70,     8,    71,    73,    29,     1,     2,    52,    68,
       1,    80,    29,    68,    81,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,    10,   123,   126,
      35,   131,    36,    68,   134,    42,    43,    44,    49,    24,
      50,    51,    26,    10,    52,    37,    38,    53,    54,    39,
      55,    78,    46,    56,    48,    72,   135,    96,    97,    98,
      99,   102,    42,    43,    44,    49,   104,    50,    51,    52,
      10,    52,    57,    58,    53,    54,   110,    55,    85,   128,
      56,    68,   136,    42,    43,    44,    42,    43,    44,    28,
      68,    10,    52,    98,    99,    74,    54,    25,     9,    57,
      58,    42,    43,    44,    49,    79,    50,    51,   137,    10,
      52,     0,     0,    53,    54,     0,    55,   107,     0,    56,
      57,    58,    42,    43,    44,    49,     0,    50,    51,     0,
      10,    52,     0,     0,    53,    54,     0,    55,    57,    58,
      56,    42,    43,    44,    42,    43,    44,     0,     0,    10,
      52,    87,    10,    52,    54,   124,     0,    54,     0,    57,
      58,     0,     0,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   105,     0,     0,    57,    58,
       0,    57,    58,     0,     0,     0,     0,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   106,
      92,    93,    94,    95,    96,    97,    98,    99,     0,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   127,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99
  };

  const short int
  Parser::yycheck_[] =
  {
       6,   127,    56,    59,    23,    18,    26,    13,    19,    22,
     136,     5,     6,     7,     5,     6,     7,    23,    51,    39,
      31,    54,    23,    79,    57,    58,    16,    81,    34,    35,
      24,    17,    37,    24,    24,     5,     6,     7,    24,    72,
      46,    47,     0,    48,    50,    21,     3,     4,    17,    55,
       3,    20,    28,    59,    23,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,    16,   101,   102,
      18,   127,    20,    79,   128,     8,     9,    10,    11,    24,
      13,    14,    21,    16,    17,    31,    22,    20,    21,    18,
      23,    24,    23,    26,    31,    21,   129,    40,    41,    42,
      43,    21,     8,     9,    10,    11,    20,    13,    14,    17,
      16,    17,    45,    46,    20,    21,    20,    23,    24,    20,
      26,   127,    12,     8,     9,    10,     8,     9,    10,    20,
     136,    16,    17,    42,    43,    20,    21,    13,     4,    45,
      46,     8,     9,    10,    11,    55,    13,    14,   136,    16,
      17,    -1,    -1,    20,    21,    -1,    23,    24,    -1,    26,
      45,    46,     8,     9,    10,    11,    -1,    13,    14,    -1,
      16,    17,    -1,    -1,    20,    21,    -1,    23,    45,    46,
      26,     8,     9,    10,     8,     9,    10,    -1,    -1,    16,
      17,    20,    16,    17,    21,    22,    -1,    21,    -1,    45,
      46,    -1,    -1,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    20,    -1,    -1,    45,    46,
      -1,    45,    46,    -1,    -1,    -1,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    22,
      36,    37,    38,    39,    40,    41,    42,    43,    -1,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    22,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43
  };

  const unsigned char
  Parser::yystos_[] =
  {
       0,     3,     4,    48,    49,    54,    23,    23,     0,    54,
      16,    24,    55,    56,    57,    72,     5,     6,     7,    24,
      50,    51,    52,    71,    24,    57,    21,    24,    51,    52,
      53,    72,    58,    59,    71,    18,    20,    31,    22,    18,
      72,    72,     8,     9,    10,    70,    23,    71,    31,    11,
      13,    14,    17,    20,    21,    23,    26,    45,    46,    60,
      61,    62,    63,    64,    65,    66,    69,    70,    72,    73,
      72,    70,    21,    72,    20,    69,    72,    69,    24,    60,
      20,    23,    66,    69,    69,    24,    61,    20,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      19,    31,    21,    69,    20,    20,    22,    24,    66,    67,
      20,    69,    69,    69,    69,    69,    69,    69,    69,    69,
      69,    69,    69,    69,    22,    68,    69,    22,    20,    18,
      22,    61,    63,    24,    66,    69,    12,    62,    63
  };

  const unsigned char
  Parser::yyr1_[] =
  {
       0,    47,    48,    48,    48,    48,    49,    49,    50,    50,
      51,    51,    52,    53,    53,    53,    53,    54,    54,    55,
      56,    56,    57,    58,    58,    59,    59,    60,    60,    61,
      61,    62,    62,    63,    63,    63,    63,    64,    64,    64,
      64,    64,    64,    64,    64,    65,    65,    65,    66,    66,
      67,    67,    68,    68,    69,    69,    69,    69,    69,    69,
      69,    69,    69,    69,    69,    69,    69,    69,    69,    69,
      69,    69,    70,    70,    70,    71,    71,    71,    72,    73
  };

  const unsigned char
  Parser::yyr2_[] =
  {
       0,     2,     0,     2,     1,     1,     3,     4,     1,     2,
       1,     2,     3,     1,     3,     3,     5,     3,     4,     1,
       1,     2,     7,     0,     1,     2,     4,     1,     2,     1,
       1,     5,     7,     1,     2,     3,     7,     1,     2,     3,
       2,     2,     3,     1,     3,     2,     3,     5,     3,     4,
       1,     3,     1,     3,     1,     1,     1,     3,     2,     2,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     1,     1,     1,     1,     1,     1,     1,     1
  };


#if YYDEBUG
  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a yyntokens_, nonterminals.
  const char*
  const Parser::yytname_[] =
  {
  "\"end of file\"", "error", "$undefined", "CODE", "SYMBOLS", "INTVAR",
  "FLOATVAR", "STRINGVAR", "INT", "FLOAT", "STRING", "IF", "ELSE", "GOTO",
  "RETURN", "DOLLAR", "USERNAME", "SYSNAME", "COMMA", "COLON", "SEMICOLON",
  "OPENPAREN", "CLOSEPAREN", "OPENBRACKET", "CLOSEBRACKET", "QUOTE",
  "WAIT", "YIELD", "EXPORT", "BREAKPOINT", "SITNSPIN", "ASSIGN", "OR",
  "AND", "EQUAL", "NOTEQUAL", "LT", "LTE", "GT", "GTE", "PLUS", "MINUS",
  "MULTIPLY", "DIVIDE", "MOD", "NOT", "NEGATE", "$accept", "script",
  "symbols", "symbols_contents", "symbols_line", "symbol_decl",
  "symbol_decl_list", "code", "code_contents", "functions_list",
  "function", "function_args_branch", "function_args", "statement_list",
  "statement", "open_statement", "closed_statement", "simple_statement",
  "wait_statement", "system_function_call", "system_function_call_list",
  "args_list", "expression", "constant", "symbol_type", "user_name",
  "sys_name", YY_NULLPTR
  };


  const unsigned short int
  Parser::yyrline_[] =
  {
       0,   139,   139,   140,   141,   142,   146,   147,   151,   152,
     156,   157,   162,   167,   168,   169,   170,   174,   175,   178,
     181,   182,   185,   188,   189,   192,   193,   196,   197,   200,
     201,   204,   205,   208,   209,   210,   211,   214,   215,   216,
     217,   218,   219,   220,   221,   224,   225,   226,   229,   230,
     233,   234,   237,   238,   241,   242,   243,   244,   245,   247,
     249,   251,   253,   255,   257,   259,   261,   263,   265,   267,
     269,   271,   276,   277,   278,   282,   283,   284,   287,   289
  };

  // Print the state stack on the debug stream.
  void
  Parser::yystack_print_ ()
  {
    *yycdebug_ << "Stack now";
    for (stack_type::const_iterator
           i = yystack_.begin (),
           i_end = yystack_.end ();
         i != i_end; ++i)
      *yycdebug_ << ' ' << i->state;
    *yycdebug_ << std::endl;
  }

  // Report on the debug stream that the rule \a yyrule is going to be reduced.
  void
  Parser::yy_reduce_print_ (int yyrule)
  {
    unsigned int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    // Print the symbols being reduced, and their result.
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
               << " (line " << yylno << "):" << std::endl;
    // The symbols being reduced.
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
                       yystack_[(yynrhs) - (yyi + 1)]);
  }
#endif // YYDEBUG


#line 32 "Sheep.yy" // lalr1.cc:1167
} // Sheep
#line 1395 "Sheep.tab.cc" // lalr1.cc:1167
