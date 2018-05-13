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

#line 40 "Sheep.tab.cc" // lalr1.cc:397


// First part of user declarations.

#line 45 "Sheep.tab.cc" // lalr1.cc:404

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

#include "Sheep.tab.hh"

// User implementation prologue.

#line 59 "Sheep.tab.cc" // lalr1.cc:412
// Unqualified %code blocks.
#line 47 "Sheep.yy" // lalr1.cc:413

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

#line 79 "Sheep.tab.cc" // lalr1.cc:413


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

#line 18 "Sheep.yy" // lalr1.cc:479
namespace Sheep {
#line 165 "Sheep.tab.cc" // lalr1.cc:479

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
      case 68: // float_expr
        value.move< float > (that.value);
        break;

      case 26: // INT
      case 67: // int_expr
        value.move< int > (that.value);
        break;

      case 28: // STRING
      case 29: // USERID
      case 30: // SYSID
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
      case 68: // float_expr
        value.copy< float > (that.value);
        break;

      case 26: // INT
      case 67: // int_expr
        value.copy< int > (that.value);
        break;

      case 28: // STRING
      case 29: // USERID
      case 30: // SYSID
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
      case 68: // float_expr
        yylhs.value.build< float > ();
        break;

      case 26: // INT
      case 67: // int_expr
        yylhs.value.build< int > ();
        break;

      case 28: // STRING
      case 29: // USERID
      case 30: // SYSID
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
#line 128 "Sheep.yy" // lalr1.cc:859
    { }
#line 552 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 3:
#line 129 "Sheep.yy" // lalr1.cc:859
    { }
#line 558 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 4:
#line 130 "Sheep.yy" // lalr1.cc:859
    { }
#line 564 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 5:
#line 131 "Sheep.yy" // lalr1.cc:859
    { }
#line 570 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 6:
#line 136 "Sheep.yy" // lalr1.cc:859
    { }
#line 576 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 7:
#line 139 "Sheep.yy" // lalr1.cc:859
    {  }
#line 582 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 8:
#line 140 "Sheep.yy" // lalr1.cc:859
    {  }
#line 588 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 9:
#line 146 "Sheep.yy" // lalr1.cc:859
    { }
#line 594 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 10:
#line 147 "Sheep.yy" // lalr1.cc:859
    { }
#line 600 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 11:
#line 148 "Sheep.yy" // lalr1.cc:859
    { }
#line 606 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 12:
#line 151 "Sheep.yy" // lalr1.cc:859
    { builder.AddIntVariable(yystack_[0].value.as< std::string > (), 0); }
#line 612 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 13:
#line 152 "Sheep.yy" // lalr1.cc:859
    { builder.AddIntVariable(yystack_[2].value.as< std::string > (), yystack_[0].value.as< int > ()); }
#line 618 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 14:
#line 153 "Sheep.yy" // lalr1.cc:859
    { builder.AddIntVariable(yystack_[0].value.as< std::string > (), 0); }
#line 624 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 15:
#line 154 "Sheep.yy" // lalr1.cc:859
    { builder.AddIntVariable(yystack_[2].value.as< std::string > (), yystack_[0].value.as< int > ()); }
#line 630 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 16:
#line 157 "Sheep.yy" // lalr1.cc:859
    { builder.AddFloatVariable(yystack_[0].value.as< std::string > (), 0.0f); }
#line 636 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 17:
#line 158 "Sheep.yy" // lalr1.cc:859
    { builder.AddFloatVariable(yystack_[2].value.as< std::string > (), yystack_[0].value.as< float > ()); }
#line 642 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 18:
#line 159 "Sheep.yy" // lalr1.cc:859
    { builder.AddFloatVariable(yystack_[0].value.as< std::string > (), 0.0f); }
#line 648 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 19:
#line 160 "Sheep.yy" // lalr1.cc:859
    { builder.AddFloatVariable(yystack_[2].value.as< std::string > (), yystack_[0].value.as< float > ()); }
#line 654 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 20:
#line 163 "Sheep.yy" // lalr1.cc:859
    { builder.AddStringVariable(yystack_[0].value.as< std::string > (), ""); }
#line 660 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 21:
#line 164 "Sheep.yy" // lalr1.cc:859
    { builder.AddStringVariable(yystack_[2].value.as< std::string > (), yystack_[0].value.as< std::string > ()); }
#line 666 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 22:
#line 165 "Sheep.yy" // lalr1.cc:859
    { builder.AddStringVariable(yystack_[0].value.as< std::string > (), ""); }
#line 672 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 23:
#line 166 "Sheep.yy" // lalr1.cc:859
    { builder.AddStringVariable(yystack_[2].value.as< std::string > (), yystack_[0].value.as< std::string > ()); }
#line 678 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 24:
#line 172 "Sheep.yy" // lalr1.cc:859
    { }
#line 684 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 25:
#line 176 "Sheep.yy" // lalr1.cc:859
    { }
#line 690 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 26:
#line 177 "Sheep.yy" // lalr1.cc:859
    { }
#line 696 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 27:
#line 182 "Sheep.yy" // lalr1.cc:859
    { builder.StartFunction(yystack_[2].value.as< std::string > ()); }
#line 702 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 28:
#line 182 "Sheep.yy" // lalr1.cc:859
    { builder.EndFunction(yystack_[6].value.as< std::string > ()); }
#line 708 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 31:
#line 191 "Sheep.yy" // lalr1.cc:859
    { builder.Store(yystack_[3].value.as< std::string > ()); }
#line 714 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 33:
#line 193 "Sheep.yy" // lalr1.cc:859
    { builder.ReturnV(); }
#line 720 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 34:
#line 194 "Sheep.yy" // lalr1.cc:859
    { builder.Breakpoint(); }
#line 726 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 35:
#line 195 "Sheep.yy" // lalr1.cc:859
    { builder.SitnSpin(); }
#line 732 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 36:
#line 196 "Sheep.yy" // lalr1.cc:859
    { builder.BranchGoto(yystack_[1].value.as< std::string > ()); }
#line 738 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 37:
#line 197 "Sheep.yy" // lalr1.cc:859
    { builder.AddGoto(yystack_[1].value.as< std::string > ()); }
#line 744 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 38:
#line 198 "Sheep.yy" // lalr1.cc:859
    { builder.BeginWait(); builder.EndWait(); }
#line 750 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 39:
#line 199 "Sheep.yy" // lalr1.cc:859
    { builder.BeginWait(); }
#line 756 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 40:
#line 199 "Sheep.yy" // lalr1.cc:859
    { builder.EndWait(); }
#line 762 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 41:
#line 200 "Sheep.yy" // lalr1.cc:859
    { builder.BeginWait(); }
#line 768 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 42:
#line 200 "Sheep.yy" // lalr1.cc:859
    { builder.EndWait(); }
#line 774 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 44:
#line 202 "Sheep.yy" // lalr1.cc:859
    { }
#line 780 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 47:
#line 212 "Sheep.yy" // lalr1.cc:859
    { builder.CallSysFunction(yystack_[3].value.as< std::string > ()); }
#line 786 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 48:
#line 216 "Sheep.yy" // lalr1.cc:859
    { }
#line 792 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 49:
#line 217 "Sheep.yy" // lalr1.cc:859
    { }
#line 798 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 50:
#line 218 "Sheep.yy" // lalr1.cc:859
    { }
#line 804 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 51:
#line 222 "Sheep.yy" // lalr1.cc:859
    { }
#line 810 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 52:
#line 223 "Sheep.yy" // lalr1.cc:859
    { builder.Load(yystack_[0].value.as< std::string > ()); }
#line 816 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 53:
#line 224 "Sheep.yy" // lalr1.cc:859
    { }
#line 822 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 54:
#line 225 "Sheep.yy" // lalr1.cc:859
    { }
#line 828 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 55:
#line 226 "Sheep.yy" // lalr1.cc:859
    { builder.AddStringConst(yystack_[0].value.as< std::string > ()); builder.PushS(yystack_[0].value.as< std::string > ()); }
#line 834 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 56:
#line 227 "Sheep.yy" // lalr1.cc:859
    { }
#line 840 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 57:
#line 230 "Sheep.yy" // lalr1.cc:859
    { builder.PushI(yystack_[0].value.as< int > ()); yylhs.value.as< int > () = yystack_[0].value.as< int > (); }
#line 846 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 58:
#line 231 "Sheep.yy" // lalr1.cc:859
    { builder.AddI(); }
#line 852 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 59:
#line 232 "Sheep.yy" // lalr1.cc:859
    { builder.SubtractI(); }
#line 858 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 60:
#line 233 "Sheep.yy" // lalr1.cc:859
    { builder.MultiplyI(); }
#line 864 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 61:
#line 234 "Sheep.yy" // lalr1.cc:859
    { builder.DivideI(); }
#line 870 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 62:
#line 235 "Sheep.yy" // lalr1.cc:859
    { builder.Modulo(); }
#line 876 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 63:
#line 236 "Sheep.yy" // lalr1.cc:859
    { builder.NegateI(); }
#line 882 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 64:
#line 238 "Sheep.yy" // lalr1.cc:859
    { builder.IsLessI(); }
#line 888 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 65:
#line 239 "Sheep.yy" // lalr1.cc:859
    { builder.IsGreaterI(); }
#line 894 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 66:
#line 240 "Sheep.yy" // lalr1.cc:859
    { builder.IsLessEqualI(); }
#line 900 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 67:
#line 241 "Sheep.yy" // lalr1.cc:859
    { builder.IsGreaterEqualI(); }
#line 906 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 68:
#line 242 "Sheep.yy" // lalr1.cc:859
    { builder.IsNotEqualI(); }
#line 912 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 69:
#line 243 "Sheep.yy" // lalr1.cc:859
    { builder.IsEqualI(); }
#line 918 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 70:
#line 244 "Sheep.yy" // lalr1.cc:859
    { builder.Or(); }
#line 924 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 71:
#line 245 "Sheep.yy" // lalr1.cc:859
    { builder.And(); }
#line 930 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 72:
#line 246 "Sheep.yy" // lalr1.cc:859
    { builder.Not(); }
#line 936 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 73:
#line 249 "Sheep.yy" // lalr1.cc:859
    { builder.PushF(yystack_[0].value.as< float > ()); yylhs.value.as< float > () = yystack_[0].value.as< float > (); }
#line 942 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 74:
#line 250 "Sheep.yy" // lalr1.cc:859
    { builder.AddF(); }
#line 948 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 75:
#line 251 "Sheep.yy" // lalr1.cc:859
    { builder.SubtractF(); }
#line 954 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 76:
#line 252 "Sheep.yy" // lalr1.cc:859
    { builder.MultiplyF(); }
#line 960 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 77:
#line 253 "Sheep.yy" // lalr1.cc:859
    { builder.DivideF(); }
#line 966 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 78:
#line 254 "Sheep.yy" // lalr1.cc:859
    { builder.NegateF(); }
#line 972 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 79:
#line 256 "Sheep.yy" // lalr1.cc:859
    { builder.IsLessF(); }
#line 978 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 80:
#line 257 "Sheep.yy" // lalr1.cc:859
    { builder.IsGreaterF(); }
#line 984 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 81:
#line 258 "Sheep.yy" // lalr1.cc:859
    { builder.IsLessEqualF(); }
#line 990 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 82:
#line 259 "Sheep.yy" // lalr1.cc:859
    { builder.IsGreaterEqualF(); }
#line 996 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 83:
#line 260 "Sheep.yy" // lalr1.cc:859
    { builder.IsNotEqualF(); }
#line 1002 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 84:
#line 261 "Sheep.yy" // lalr1.cc:859
    { builder.IsEqualF(); }
#line 1008 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 85:
#line 262 "Sheep.yy" // lalr1.cc:859
    { builder.Or(); }
#line 1014 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 86:
#line 263 "Sheep.yy" // lalr1.cc:859
    { builder.And(); }
#line 1020 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 87:
#line 264 "Sheep.yy" // lalr1.cc:859
    { builder.Not(); }
#line 1026 "Sheep.tab.cc" // lalr1.cc:859
    break;

  case 92:
#line 275 "Sheep.yy" // lalr1.cc:859
    { }
#line 1032 "Sheep.tab.cc" // lalr1.cc:859
    break;


#line 1036 "Sheep.tab.cc" // lalr1.cc:859
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


  const signed char Parser::yypact_ninf_ = -94;

  const signed char Parser::yytable_ninf_ = -42;

  const short int
  Parser::yypact_[] =
  {
      11,    16,    52,    69,    73,   -94,   -94,   -94,   -94,   -94,
      54,    80,   -94,    58,   -94,   -94,    49,    56,    82,   -94,
      98,    85,    -3,    89,    27,    91,    63,   -94,    61,    97,
     -94,   100,   110,   -94,   114,   115,   -94,   127,   -94,   117,
     -94,   125,   -94,   126,   -94,   120,   131,   149,    72,   -94,
     -94,   -94,   244,   230,   248,   -94,   -10,   -94,   -94,    26,
     249,   250,   -94,   -94,   -94,     0,   251,   -21,   -21,   -94,
     -94,   -94,   252,   132,   148,   -94,   257,   -10,   254,   -94,
     -94,   253,   107,   -94,   237,   255,   -94,   -94,   -94,   -10,
     -10,   -94,   -94,   -94,   -94,   -94,   -16,   -16,   -16,   -16,
     -16,   -16,   -16,   -16,   -16,   -16,   -16,   -16,   -16,    -5,
      -5,    -5,    -5,    -5,    -5,    -5,    -5,    -5,    -5,    -5,
      -5,     7,   -94,   258,   -94,   -94,   -94,   -94,   -94,   260,
      -2,   -94,   -16,   -16,   159,   170,   200,   200,   213,   213,
     213,   213,    88,    88,   -94,   -94,   -94,    -5,    -5,   182,
     192,   209,   209,    66,    66,    66,    66,    53,    53,   -94,
     -94,   -94,   -94,   259,   133,   -94,   -10,   -94,   -94,   -94,
     -94
  };

  const unsigned char
  Parser::yydefact_[] =
  {
       2,     0,     0,     0,     3,     4,    25,     7,     1,     5,
       0,     0,    24,     0,    26,     6,     0,     0,     0,     8,
       0,    12,     0,    16,     0,    20,     0,    27,     0,     0,
       9,     0,     0,    10,     0,     0,    11,     0,    13,    14,
      17,    18,    21,    22,    29,     0,     0,     0,     0,    15,
      19,    23,     0,     0,     0,    44,     0,    29,    28,    39,
       0,     0,    57,    73,    55,    52,     0,     0,     0,    30,
      43,    51,     0,    53,    54,    45,    88,     0,     0,    33,
      52,     0,     0,    38,     0,     0,    34,    35,    37,     0,
      48,    72,    87,    63,    78,    32,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    89,     0,    36,    56,    46,    40,    29,     0,
       0,    49,     0,     0,    70,    71,    69,    68,    64,    66,
      65,    67,    58,    59,    60,    61,    62,     0,     0,    85,
      86,    84,    83,    79,    81,    80,    82,    74,    75,    76,
      77,    91,    92,     0,     0,    31,     0,    47,    90,    42,
      50
  };

  const short int
  Parser::yypgoto_[] =
  {
     -94,   -94,   -94,   -94,   -94,   -94,   -94,   -94,   265,   -94,
     -94,   -94,   -54,   -94,   -94,   -94,   -93,   186,   -94,   -56,
     -41,   -66,   153,   -94,   -94
  };

  const short int
  Parser::yydefgoto_[] =
  {
      -1,     3,     4,    11,    19,    22,    24,    26,     5,    10,
      14,    37,    48,    69,    84,    85,    70,    71,   130,    72,
      73,    74,    75,    76,   122
  };

  const short int
  Parser::yytable_[] =
  {
      81,    92,    94,    82,    56,    62,    63,    29,   166,    30,
      62,    88,    52,   167,     1,     2,    62,    63,    64,    80,
      66,   123,    63,    57,    67,    68,    91,    93,   161,   132,
     133,    89,     6,   129,   131,    67,    68,    32,    83,    33,
     147,   148,   -41,   149,   150,   151,   152,   153,   154,   155,
     156,   157,   158,   159,   160,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,     7,     8,
     168,    12,    20,    35,   164,    36,     1,    52,    21,    53,
      54,    92,    94,    13,    55,    23,    56,    38,    57,    58,
      59,    91,    93,    60,    61,   119,   120,    15,    62,    63,
      64,    65,    66,    16,    17,    18,   117,   118,   119,   120,
     170,    25,    52,    27,    53,    54,    28,    67,    68,    55,
      31,    56,    34,    57,   126,    59,    39,    40,    60,    61,
     106,   107,   108,    62,    63,    64,    65,    66,    52,    41,
      53,    54,    42,    44,    43,    55,    49,    56,    45,    57,
     169,    59,    67,    68,    60,    61,    46,    47,    50,    62,
      63,    64,    65,    66,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,    51,    67,    68,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   113,   114,   115,   116,   117,
     118,   119,   120,   104,   105,   106,   107,   108,    77,    78,
      79,    86,    87,   121,    95,    90,   124,    66,   125,     9,
     127,   128,   165,   163,   162,    57
  };

  const unsigned char
  Parser::yycheck_[] =
  {
      56,    67,    68,    57,    14,    26,    27,    10,    10,    12,
      26,    11,     5,    15,     3,     4,    26,    27,    28,    29,
      30,    77,    27,    16,    45,    46,    67,    68,   121,    45,
      46,    31,    16,    89,    90,    45,    46,    10,    12,    12,
      45,    46,    16,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,    16,     0,
     163,    17,    14,    10,   128,    12,     3,     5,    29,     7,
       8,   147,   148,    29,    12,    29,    14,    26,    16,    17,
      18,   132,   133,    21,    22,    42,    43,    17,    26,    27,
      28,    29,    30,    23,    24,    25,    40,    41,    42,    43,
     166,    29,     5,    15,     7,     8,    31,    45,    46,    12,
      31,    14,    31,    16,    17,    18,    29,    27,    21,    22,
      42,    43,    44,    26,    27,    28,    29,    30,     5,    29,
       7,     8,    28,    16,    29,    12,    26,    14,    31,    16,
      17,    18,    45,    46,    21,    22,    31,    31,    27,    26,
      27,    28,    29,    30,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    28,    45,    46,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    36,    37,    38,    39,    40,
      41,    42,    43,    40,    41,    42,    43,    44,    14,    29,
      12,    12,    12,     6,    12,    14,    12,    30,    15,     4,
      84,    16,    12,    15,   121,    16
  };

  const unsigned char
  Parser::yystos_[] =
  {
       0,     3,     4,    48,    49,    55,    16,    16,     0,    55,
      56,    50,    17,    29,    57,    17,    23,    24,    25,    51,
      14,    29,    52,    29,    53,    29,    54,    15,    31,    10,
      12,    31,    10,    12,    31,    10,    12,    58,    26,    29,
      27,    29,    28,    29,    16,    31,    31,    31,    59,    26,
      27,    28,     5,     7,     8,    12,    14,    16,    17,    18,
      21,    22,    26,    27,    28,    29,    30,    45,    46,    60,
      63,    64,    66,    67,    68,    69,    70,    14,    29,    12,
      29,    66,    59,    12,    61,    62,    12,    12,    11,    31,
      14,    67,    68,    67,    68,    12,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,     6,    71,    66,    12,    15,    17,    64,    16,    66,
      65,    66,    45,    46,    67,    67,    67,    67,    67,    67,
      67,    67,    67,    67,    67,    67,    67,    45,    46,    68,
      68,    68,    68,    68,    68,    68,    68,    68,    68,    68,
      68,    63,    69,    15,    59,    12,    10,    15,    63,    17,
      66
  };

  const unsigned char
  Parser::yyr1_[] =
  {
       0,    47,    48,    48,    48,    48,    49,    50,    50,    51,
      51,    51,    52,    52,    52,    52,    53,    53,    53,    53,
      54,    54,    54,    54,    55,    56,    56,    58,    57,    59,
      59,    60,    60,    60,    60,    60,    60,    60,    60,    61,
      60,    62,    60,    60,    60,    60,    63,    64,    65,    65,
      65,    66,    66,    66,    66,    66,    66,    67,    67,    67,
      67,    67,    67,    67,    67,    67,    67,    67,    67,    67,
      67,    67,    67,    68,    68,    68,    68,    68,    68,    68,
      68,    68,    68,    68,    68,    68,    68,    68,    69,    69,
      70,    71,    71
  };

  const unsigned char
  Parser::yyr2_[] =
  {
       0,     2,     0,     1,     1,     2,     4,     0,     2,     3,
       3,     3,     1,     3,     3,     5,     1,     3,     3,     5,
       1,     3,     3,     5,     4,     0,     2,     0,     7,     0,
       2,     4,     2,     2,     2,     2,     3,     2,     2,     0,
       3,     0,     5,     1,     1,     1,     3,     4,     0,     1,
       3,     1,     1,     1,     1,     1,     3,     1,     3,     3,
       3,     3,     3,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     2,     1,     3,     3,     3,     3,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     2,     1,     2,
       5,     2,     2
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
  "code_section", "functions", "function", "$@1", "statements",
  "statement", "$@2", "$@3", "statements_block", "sysfunc_call",
  "sysfunc_call_args", "expr", "int_expr", "float_expr", "if_else_block",
  "if_statement", "else_statement", YY_NULLPTR
  };


  const unsigned short int
  Parser::yyrline_[] =
  {
       0,   128,   128,   129,   130,   131,   136,   139,   140,   146,
     147,   148,   151,   152,   153,   154,   157,   158,   159,   160,
     163,   164,   165,   166,   172,   176,   177,   182,   182,   186,
     187,   191,   192,   193,   194,   195,   196,   197,   198,   199,
     199,   200,   200,   201,   202,   203,   207,   212,   216,   217,
     218,   222,   223,   224,   225,   226,   227,   230,   231,   232,
     233,   234,   235,   236,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   249,   250,   251,   252,   253,   254,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   267,   268,
     271,   274,   275
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


#line 18 "Sheep.yy" // lalr1.cc:1167
} // Sheep
#line 1457 "Sheep.tab.cc" // lalr1.cc:1167
#line 278 "Sheep.yy" // lalr1.cc:1168


/* No epilogue is needed. */

