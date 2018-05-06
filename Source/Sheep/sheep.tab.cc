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
// //                    "%code top" blocks.
#line 2 "Sheep.yy" // lalr1.cc:397

#include <cstdlib>
#include <string>
#include "SheepScriptBuilder.h"
//#define YYSTYPE SheepNode*

#line 41 "Sheep.tab.cc" // lalr1.cc:397


// First part of user declarations.

#line 46 "Sheep.tab.cc" // lalr1.cc:404

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

#include "Sheep.tab.hh"

// User implementation prologue.

#line 60 "Sheep.tab.cc" // lalr1.cc:412
// Unqualified %code blocks.
#line 48 "Sheep.yy" // lalr1.cc:413

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

#line 80 "Sheep.tab.cc" // lalr1.cc:413


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

#line 19 "Sheep.yy" // lalr1.cc:479
namespace Sheep {
#line 166 "Sheep.tab.cc" // lalr1.cc:479

  /// Build a parser object.
  Parser::Parser (Sheep::Scanner& scanner_yyarg, Sheep::Driver& driver_yyarg, SheepScriptBuilder& builder_yyarg)
    :
#if YYDEBUG
      yydebug_ (false),
      yycdebug_ (&std::cerr),
#endif
      scanner (scanner_yyarg),
      driver (driver_yyarg),
      builder (builder_yyarg)
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
      case 27: // FLOAT
      case 67: // float_expr
        value.move< float > (that.value);
        break;

      case 26: // INT
      case 66: // int_expr
        value.move< int > (that.value);
        break;

      case 28: // STRING
      case 29: // USERID
      case 30: // SYSID
      case 69: // string_expr
        value.move< std::string > (that.value);
        break;

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
      case 27: // FLOAT
      case 67: // float_expr
        value.copy< float > (that.value);
        break;

      case 26: // INT
      case 66: // int_expr
        value.copy< int > (that.value);
        break;

      case 28: // STRING
      case 29: // USERID
      case 30: // SYSID
      case 69: // string_expr
        value.copy< std::string > (that.value);
        break;

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
            symbol_type yylookahead (yylex (scanner, driver, builder));
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
      case 27: // FLOAT
      case 67: // float_expr
        yylhs.value.build< float > ();
        break;

      case 26: // INT
      case 66: // int_expr
        yylhs.value.build< int > ();
        break;

      case 28: // STRING
      case 29: // USERID
      case 30: // SYSID
      case 69: // string_expr
        yylhs.value.build< std::string > ();
        break;

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
  case 2:
#line 130 "Sheep.yy" // lalr1.cc:859
    { std::cout << "EMPTY SCRIPT" << std::endl; }
#line 556 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 3:
#line 131 "Sheep.yy" // lalr1.cc:859
    { std::cout << "SYMBOLS ONLY SCRIPT" << std::endl; }
#line 562 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 4:
#line 132 "Sheep.yy" // lalr1.cc:859
    { std::cout << "CODE ONLY SCRIPT" << std::endl; }
#line 568 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 5:
#line 133 "Sheep.yy" // lalr1.cc:859
    { std::cout << "SYMBOLS AND CODE SCRIPT" << std::endl; }
#line 574 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 6:
#line 138 "Sheep.yy" // lalr1.cc:859
    { std::cout << "SYMBOLS" << std::endl; }
#line 580 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 7:
#line 141 "Sheep.yy" // lalr1.cc:859
    {  }
#line 586 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 8:
#line 142 "Sheep.yy" // lalr1.cc:859
    {  }
#line 592 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 9:
#line 148 "Sheep.yy" // lalr1.cc:859
    { std::cout << "INT SYMBOL" << std::endl; }
#line 598 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 10:
#line 149 "Sheep.yy" // lalr1.cc:859
    { std::cout << "FLOAT SYMBOL" << std::endl; }
#line 604 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 11:
#line 150 "Sheep.yy" // lalr1.cc:859
    { std::cout << "STRING SYMBOL" << std::endl; }
#line 610 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 12:
#line 153 "Sheep.yy" // lalr1.cc:859
    { builder.AddIntVariable(yystack_[0].value.as< std::string > (), 0); std::cout << "SYMBOL " << yystack_[0].value.as< std::string > () << std::endl; }
#line 616 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 13:
#line 154 "Sheep.yy" // lalr1.cc:859
    { builder.AddIntVariable(yystack_[2].value.as< std::string > (), yystack_[0].value.as< int > ()); std::cout << "SYMBOL " << yystack_[2].value.as< std::string > () << " EQUALS " << yystack_[0].value.as< int > () << std::endl; }
#line 622 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 14:
#line 155 "Sheep.yy" // lalr1.cc:859
    { builder.AddIntVariable(yystack_[0].value.as< std::string > (), 0); std::cout << "AND SYMBOL " << yystack_[0].value.as< std::string > () << std::endl; }
#line 628 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 15:
#line 156 "Sheep.yy" // lalr1.cc:859
    { builder.AddIntVariable(yystack_[2].value.as< std::string > (), yystack_[0].value.as< int > ()); std::cout << "AND SYMBOL " << yystack_[2].value.as< std::string > () << " EQUALS " << yystack_[0].value.as< int > () << std::endl; }
#line 634 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 16:
#line 159 "Sheep.yy" // lalr1.cc:859
    { builder.AddFloatVariable(yystack_[0].value.as< std::string > (), 0.0f); std::cout << "SYMBOL " << yystack_[0].value.as< std::string > () << std::endl; }
#line 640 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 17:
#line 160 "Sheep.yy" // lalr1.cc:859
    { builder.AddFloatVariable(yystack_[2].value.as< std::string > (), yystack_[0].value.as< float > ()); std::cout << "SYMBOL " << yystack_[2].value.as< std::string > () << " EQUALS " << yystack_[0].value.as< float > () << std::endl; }
#line 646 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 18:
#line 161 "Sheep.yy" // lalr1.cc:859
    { builder.AddFloatVariable(yystack_[0].value.as< std::string > (), 0.0f); std::cout << "AND SYMBOL " << yystack_[0].value.as< std::string > () << std::endl; }
#line 652 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 19:
#line 162 "Sheep.yy" // lalr1.cc:859
    { builder.AddFloatVariable(yystack_[2].value.as< std::string > (), yystack_[0].value.as< float > ()); std::cout << "AND SYMBOL " << yystack_[2].value.as< std::string > () << " EQUALS " << yystack_[0].value.as< float > () << std::endl; }
#line 658 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 20:
#line 165 "Sheep.yy" // lalr1.cc:859
    { builder.AddStringVariable(yystack_[0].value.as< std::string > (), ""); std::cout << "SYMBOL " << yystack_[0].value.as< std::string > () << std::endl; }
#line 664 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 21:
#line 166 "Sheep.yy" // lalr1.cc:859
    { builder.AddStringVariable(yystack_[2].value.as< std::string > (), yystack_[0].value.as< std::string > ()); std::cout << "SYMBOL " << yystack_[2].value.as< std::string > () << " EQUALS " << yystack_[0].value.as< std::string > () << std::endl; }
#line 670 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 22:
#line 167 "Sheep.yy" // lalr1.cc:859
    { builder.AddStringVariable(yystack_[0].value.as< std::string > (), ""); std::cout << "AND SYMBOL " << yystack_[0].value.as< std::string > () << std::endl; }
#line 676 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 23:
#line 168 "Sheep.yy" // lalr1.cc:859
    { builder.AddStringVariable(yystack_[2].value.as< std::string > (), yystack_[0].value.as< std::string > ()); std::cout << "AND SYMBOL " << yystack_[2].value.as< std::string > () << " EQUALS " << yystack_[0].value.as< std::string > () << std::endl; }
#line 682 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 24:
#line 174 "Sheep.yy" // lalr1.cc:859
    { std::cout << "CODE" << std::endl; }
#line 688 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 25:
#line 178 "Sheep.yy" // lalr1.cc:859
    { }
#line 694 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 26:
#line 179 "Sheep.yy" // lalr1.cc:859
    { }
#line 700 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 27:
#line 184 "Sheep.yy" // lalr1.cc:859
    { builder.AddFunction(yystack_[5].value.as< std::string > ()); std::cout << "FUNCTION " << yystack_[5].value.as< std::string > () << std::endl; }
#line 706 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 30:
#line 193 "Sheep.yy" // lalr1.cc:859
    { std::cout << "STMNT USERID ASSIGN expr" << std::endl; }
#line 712 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 32:
#line 195 "Sheep.yy" // lalr1.cc:859
    { builder.ReturnV(); }
#line 718 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 33:
#line 196 "Sheep.yy" // lalr1.cc:859
    { builder.Breakpoint(); }
#line 724 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 34:
#line 197 "Sheep.yy" // lalr1.cc:859
    { builder.SitNSpin(); }
#line 730 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 37:
#line 200 "Sheep.yy" // lalr1.cc:859
    { builder.BeginWait(); builder.EndWait(); }
#line 736 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 38:
#line 201 "Sheep.yy" // lalr1.cc:859
    { builder.BeginWait(); }
#line 742 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 39:
#line 201 "Sheep.yy" // lalr1.cc:859
    { builder.EndWait(); }
#line 748 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 40:
#line 202 "Sheep.yy" // lalr1.cc:859
    { builder.BeginWait(); }
#line 754 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 41:
#line 202 "Sheep.yy" // lalr1.cc:859
    { builder.EndWait(); }
#line 760 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 43:
#line 204 "Sheep.yy" // lalr1.cc:859
    { std::cout << "SEMICOLON" << std::endl; }
#line 766 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 46:
#line 214 "Sheep.yy" // lalr1.cc:859
    { std::cout << "SYSFUNC CALL" << std::endl; }
#line 772 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 48:
#line 219 "Sheep.yy" // lalr1.cc:859
    { std::cout << "SYSFUNC ARG " << std::endl; }
#line 778 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 50:
#line 224 "Sheep.yy" // lalr1.cc:859
    { std::cout << "EXPR SYSFUNC CALL" << std::endl; }
#line 784 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 51:
#line 229 "Sheep.yy" // lalr1.cc:859
    { std::cout << "EXPR NUMBER" << std::endl; }
#line 790 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 52:
#line 230 "Sheep.yy" // lalr1.cc:859
    { std::cout << "EXPR STRING" << std::endl; }
#line 796 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 53:
#line 231 "Sheep.yy" // lalr1.cc:859
    { std::cout << "EXPR (EXPR)" << std::endl; }
#line 802 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 54:
#line 250 "Sheep.yy" // lalr1.cc:859
    { yylhs.value.as< int > () = yystack_[0].value.as< int > (); builder.PushI(yystack_[0].value.as< int > ()); std::cout << "INT " << yystack_[0].value.as< int > () << std::endl; }
#line 808 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 55:
#line 251 "Sheep.yy" // lalr1.cc:859
    { std::cout << yystack_[2].value.as< int > () << " + " << yystack_[0].value.as< int > () << std::endl; }
#line 814 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 70:
#line 268 "Sheep.yy" // lalr1.cc:859
    { yylhs.value.as< float > () = yystack_[0].value.as< float > (); std::cout << "FLOAT " << yystack_[0].value.as< float > () << std::endl; }
#line 820 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 95:
#line 297 "Sheep.yy" // lalr1.cc:859
    { builder.AddStringConst(yystack_[0].value.as< std::string > ()); yylhs.value.as< std::string > () = yystack_[0].value.as< std::string > (); std::cout << "STRING " << yystack_[0].value.as< std::string > () << std::endl; }
#line 826 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 96:
#line 298 "Sheep.yy" // lalr1.cc:859
    { yylhs.value.as< std::string > () = yystack_[0].value.as< std::string > (); std::cout << "USERID " << yystack_[0].value.as< std::string > () << std::endl; }
#line 832 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 101:
#line 309 "Sheep.yy" // lalr1.cc:859
    { }
#line 838 "Sheep.tab.cc" // lalr1.cc:859
    break;


#line 842 "Sheep.tab.cc" // lalr1.cc:859
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


  const signed char Parser::yypact_ninf_ = -119;

  const signed char Parser::yytable_ninf_ = -41;

  const short int
  Parser::yypact_[] =
  {
      29,    31,    44,    16,    60,  -119,  -119,  -119,  -119,  -119,
       5,   -10,  -119,    50,  -119,  -119,    45,    68,   100,  -119,
      58,   110,   118,   115,   122,   117,   123,   134,   128,   126,
    -119,   131,   130,  -119,   132,   138,  -119,  -119,  -119,   139,
    -119,   140,  -119,   141,   135,   148,   149,   156,   171,   157,
     181,  -119,    -9,  -119,  -119,    49,   184,   185,  -119,  -119,
    -119,     0,   188,   -18,   -18,  -119,  -119,  -119,   198,    48,
     202,  -119,  -119,  -119,   192,  -119,  -119,  -119,    -9,   199,
    -119,  -119,   197,   161,  -119,   170,   215,  -119,  -119,  -119,
      -9,    -9,  -119,  -119,  -119,  -119,  -119,   -16,   -16,   -16,
     -16,   -16,   -16,   -16,   -16,   -18,   -18,   -18,   -18,   -16,
     -21,   -21,   -21,   -21,   -21,   -21,   -21,   -21,   -18,   -18,
     -18,   -18,     7,  -119,   296,  -119,  -119,  -119,  -119,  -119,
     300,    25,  -119,   -16,   -16,   -16,   -16,   -21,   -21,   -21,
     -21,   -16,   -16,   213,   236,   266,   266,    28,    28,    28,
      28,    83,   226,    83,   226,  -119,   226,  -119,   226,  -119,
     -21,   -21,   248,   258,    77,    77,    53,    53,    53,    53,
     186,    -1,   186,    -1,   186,  -119,   186,  -119,  -119,  -119,
     297,   187,  -119,    -9,  -119,  -119,  -119,  -119
  };

  const unsigned char
  Parser::yydefact_[] =
  {
       2,     0,     0,     0,     3,     4,    25,     7,     1,     5,
       0,     0,    24,     0,    26,     6,     0,     0,     0,     8,
       0,    12,     0,    16,     0,    20,     0,     0,     0,     0,
       9,     0,     0,    10,     0,     0,    11,    28,    13,    14,
      17,    18,    21,    22,     0,     0,     0,     0,     0,     0,
       0,    43,     0,    28,    27,    38,     0,     0,    54,    70,
      95,    96,     0,     0,     0,    29,    42,    50,     0,    85,
      86,    51,    52,    44,    97,    15,    19,    23,     0,     0,
      32,    96,     0,     0,    37,     0,     0,    33,    34,    36,
       0,    47,    69,    84,    68,    83,    31,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    98,     0,    35,    53,    45,    39,    28,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    66,    67,    65,    64,    60,    62,    61,
      63,    55,    87,    56,    89,    57,    91,    58,    93,    59,
       0,     0,    81,    82,    80,    79,    75,    77,    76,    78,
      88,    71,    90,    72,    92,    73,    94,    74,   100,   101,
       0,     0,    30,     0,    46,    99,    41,    49
  };

  const short int
  Parser::yypgoto_[] =
  {
    -119,  -119,  -119,  -119,  -119,  -119,  -119,  -119,   310,  -119,
    -119,   -50,  -119,  -119,  -119,  -118,   230,  -119,   -52,     3,
     -62,  -119,  -119,   194,  -119,  -119
  };

  const short int
  Parser::yydefgoto_[] =
  {
      -1,     3,     4,    11,    19,    22,    24,    26,     5,    10,
      14,    44,    65,    85,    86,    66,    67,   131,    68,    69,
      70,    71,    72,    73,    74,   123
  };

  const short int
  Parser::yytable_[] =
  {
      82,    93,    95,    83,   178,    52,    59,    15,    58,    59,
      58,    89,    48,    16,    17,    18,     8,    58,    59,    60,
      81,    62,    12,    53,   160,   161,   124,    63,    64,   141,
     142,    90,     1,     2,    13,   183,    63,    64,   130,   132,
     184,   139,   140,   152,   154,   156,   158,     6,   162,   163,
     164,   165,   166,   167,   168,   169,   171,   173,   175,   177,
       7,    84,   185,     1,    20,   -40,    92,    94,   133,   134,
     135,   136,   109,    27,    21,   171,   173,   175,   177,   181,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   137,   138,   139,   140,    23,    93,    95,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   153,
     155,   157,   159,   114,   115,   116,   117,   137,   138,   139,
     140,   170,   172,   174,   176,   135,   136,   109,    29,    25,
      30,   187,    32,    35,    33,    36,   151,   153,   155,   157,
      48,    28,    49,    50,    92,    94,    31,    51,    34,    52,
      37,    53,    54,    55,    38,    39,    56,    57,    40,    41,
      42,    58,    59,    60,    61,    62,    48,    43,    49,    50,
      45,    46,    47,    51,    75,    52,    76,    53,   127,    55,
      63,    64,    56,    57,    77,    78,    79,    58,    59,    60,
      61,    62,    48,    80,    49,    50,    87,    88,   122,    51,
      62,    52,    91,    53,   186,    55,    63,    64,    56,    57,
      96,   125,   126,    58,    59,    60,    61,    62,    97,    98,
      99,   100,   101,   102,   103,   104,   133,   134,   135,   136,
     109,   129,    63,    64,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,    98,    99,   100,   101,
     102,   103,   104,   133,   134,   135,   136,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   137,   138,   139,   140,
      99,   100,   101,   102,   103,   104,   133,   134,   135,   136,
     109,   111,   112,   113,   114,   115,   116,   117,   137,   138,
     139,   140,   112,   113,   114,   115,   116,   117,   137,   138,
     139,   140,   101,   102,   103,   104,   133,   134,   135,   136,
     109,   180,   182,    53,     9,   128,   179
  };

  const unsigned char
  Parser::yycheck_[] =
  {
      52,    63,    64,    53,   122,    14,    27,    17,    26,    27,
      26,    11,     5,    23,    24,    25,     0,    26,    27,    28,
      29,    30,    17,    16,    45,    46,    78,    45,    46,    45,
      46,    31,     3,     4,    29,    10,    45,    46,    90,    91,
      15,    42,    43,   105,   106,   107,   108,    16,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
      16,    12,   180,     3,    14,    16,    63,    64,    40,    41,
      42,    43,    44,    15,    29,   137,   138,   139,   140,   129,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    40,    41,    42,    43,    29,   160,   161,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,    36,    37,    38,    39,    40,    41,    42,
      43,   118,   119,   120,   121,    42,    43,    44,    10,    29,
      12,   183,    10,    10,    12,    12,   133,   134,   135,   136,
       5,    31,     7,     8,   141,   142,    31,    12,    31,    14,
      16,    16,    17,    18,    26,    29,    21,    22,    27,    29,
      28,    26,    27,    28,    29,    30,     5,    29,     7,     8,
      31,    31,    31,    12,    26,    14,    27,    16,    17,    18,
      45,    46,    21,    22,    28,    14,    29,    26,    27,    28,
      29,    30,     5,    12,     7,     8,    12,    12,     6,    12,
      30,    14,    14,    16,    17,    18,    45,    46,    21,    22,
      12,    12,    15,    26,    27,    28,    29,    30,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    16,    45,    46,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    15,    12,    16,     4,    85,   122
  };

  const unsigned char
  Parser::yystos_[] =
  {
       0,     3,     4,    48,    49,    55,    16,    16,     0,    55,
      56,    50,    17,    29,    57,    17,    23,    24,    25,    51,
      14,    29,    52,    29,    53,    29,    54,    15,    31,    10,
      12,    31,    10,    12,    31,    10,    12,    16,    26,    29,
      27,    29,    28,    29,    58,    31,    31,    31,     5,     7,
       8,    12,    14,    16,    17,    18,    21,    22,    26,    27,
      28,    29,    30,    45,    46,    59,    62,    63,    65,    66,
      67,    68,    69,    70,    71,    26,    27,    28,    14,    29,
      12,    29,    65,    58,    12,    60,    61,    12,    12,    11,
      31,    14,    66,    67,    66,    67,    12,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,     6,    72,    65,    12,    15,    17,    63,    16,
      65,    64,    65,    40,    41,    42,    43,    40,    41,    42,
      43,    45,    46,    66,    66,    66,    66,    66,    66,    66,
      66,    66,    67,    66,    67,    66,    67,    66,    67,    66,
      45,    46,    67,    67,    67,    67,    67,    67,    67,    67,
      66,    67,    66,    67,    66,    67,    66,    67,    62,    70,
      15,    58,    12,    10,    15,    62,    17,    65
  };

  const unsigned char
  Parser::yyr1_[] =
  {
       0,    47,    48,    48,    48,    48,    49,    50,    50,    51,
      51,    51,    52,    52,    52,    52,    53,    53,    53,    53,
      54,    54,    54,    54,    55,    56,    56,    57,    58,    58,
      59,    59,    59,    59,    59,    59,    59,    59,    60,    59,
      61,    59,    59,    59,    59,    62,    63,    64,    64,    64,
      65,    65,    65,    65,    66,    66,    66,    66,    66,    66,
      66,    66,    66,    66,    66,    66,    66,    66,    66,    66,
      67,    67,    67,    67,    67,    67,    67,    67,    67,    67,
      67,    67,    67,    67,    67,    68,    68,    68,    68,    68,
      68,    68,    68,    68,    68,    69,    69,    70,    70,    71,
      72,    72
  };

  const unsigned char
  Parser::yyr2_[] =
  {
       0,     2,     0,     1,     1,     2,     4,     0,     2,     3,
       3,     3,     1,     3,     3,     5,     1,     3,     3,     5,
       1,     3,     3,     5,     4,     0,     2,     6,     0,     2,
       4,     2,     2,     2,     2,     3,     2,     2,     0,     3,
       0,     5,     1,     1,     1,     3,     4,     0,     1,     3,
       1,     1,     1,     3,     1,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     2,
       1,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     2,     2,     1,     1,     3,     3,     3,
       3,     3,     3,     3,     3,     1,     1,     1,     2,     5,
       2,     2
  };


#if YYDEBUG
  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a yyntokens_, nonterminals.
  const char*
  const Parser::yytname_[] =
  {
  "\"end of file\"", "error", "$undefined", "CODE", "SYMBOLS", "IF",
  "ELSE", "GOTO", "RETURN", "DOLLAR", "COMMA", "COLON", "SEMICOLON",
  "QUOTE", "OPENPAREN", "CLOSEPAREN", "OPENBRACKET", "CLOSEBRACKET",
  "WAIT", "YIELD", "EXPORT", "BREAKPOINT", "SITNSPIN", "INTVAR",
  "FLOATVAR", "STRINGVAR", "INT", "FLOAT", "STRING", "USERID", "SYSID",
  "ASSIGN", "OR", "AND", "EQUAL", "NOTEQUAL", "LT", "LTE", "GT", "GTE",
  "PLUS", "MINUS", "MULTIPLY", "DIVIDE", "MOD", "NOT", "NEGATE", "$accept",
  "script", "symbols_section", "symbol_decls", "symbol_decl",
  "symbol_decl_int", "symbol_decl_float", "symbol_decl_string",
  "code_section", "functions", "function", "statements", "statement",
  "$@1", "$@2", "statements_block", "sysfunc_call", "sysfunc_call_args",
  "expr", "int_expr", "float_expr", "number_expr", "string_expr",
  "if_else_block", "if_statement", "else_statement", YY_NULLPTR
  };


  const unsigned short int
  Parser::yyrline_[] =
  {
       0,   130,   130,   131,   132,   133,   138,   141,   142,   148,
     149,   150,   153,   154,   155,   156,   159,   160,   161,   162,
     165,   166,   167,   168,   174,   178,   179,   184,   188,   189,
     193,   194,   195,   196,   197,   198,   199,   200,   201,   201,
     202,   202,   203,   204,   205,   209,   214,   218,   219,   220,
     224,   229,   230,   231,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
     268,   269,   270,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,   281,   282,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   297,   298,   301,   302,   305,
     308,   309
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


#line 19 "Sheep.yy" // lalr1.cc:1167
} // Sheep
#line 1277 "Sheep.tab.cc" // lalr1.cc:1167
#line 312 "Sheep.yy" // lalr1.cc:1168


/* No epilogue is needed. */

