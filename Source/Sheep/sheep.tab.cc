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
#line 2 "sheep.yy" // lalr1.cc:397

	#include <cstdlib>
	#include <string>

#line 39 "sheep.tab.cc" // lalr1.cc:397


// First part of user declarations.

#line 44 "sheep.tab.cc" // lalr1.cc:404

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

#include "sheep.tab.hh"

// User implementation prologue.

#line 58 "sheep.tab.cc" // lalr1.cc:412
// Unqualified %code blocks.
#line 45 "sheep.yy" // lalr1.cc:413

	#include "SheepCompiler.h"

	// When requesting yylex, redirect to scanner.yylex.
	#undef yylex
	#define yylex scanner.yylex

	// The C++ yytext is const, but we need to modify to remove
	// quotes in some cases. So, this is a bit of a hack.
	#undef yytext
	#define yytext scanner.GetYYText()

	void Sheep::Parser::error(const location_type& loc, const std::string& msg)
	{
		compiler.error(loc, msg);
	}

#line 78 "sheep.tab.cc" // lalr1.cc:413


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

#line 18 "sheep.yy" // lalr1.cc:479
namespace Sheep {
#line 164 "sheep.tab.cc" // lalr1.cc:479

  /// Build a parser object.
  Parser::Parser (SheepScanner& scanner_yyarg, SheepCompiler& compiler_yyarg, SheepScriptBuilder& builder_yyarg)
    :
#if YYDEBUG
      yydebug_ (false),
      yycdebug_ (&std::cerr),
#endif
      scanner (scanner_yyarg),
      compiler (compiler_yyarg),
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
      case 64: // expr
      case 65: // sysfunc_call
        value.move< SheepValue > (that.value);
        break;

      case 27: // FLOAT
        value.move< float > (that.value);
        break;

      case 26: // INT
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
      case 64: // expr
      case 65: // sysfunc_call
        value.copy< SheepValue > (that.value);
        break;

      case 27: // FLOAT
        value.copy< float > (that.value);
        break;

      case 26: // INT
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
            symbol_type yylookahead (yylex (scanner, compiler, builder));
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
      case 64: // expr
      case 65: // sysfunc_call
        yylhs.value.build< SheepValue > ();
        break;

      case 27: // FLOAT
        yylhs.value.build< float > ();
        break;

      case 26: // INT
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
#line 126 "sheep.yy" // lalr1.cc:859
    { }
#line 560 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 3:
#line 127 "sheep.yy" // lalr1.cc:859
    { }
#line 566 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 4:
#line 128 "sheep.yy" // lalr1.cc:859
    { }
#line 572 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 5:
#line 129 "sheep.yy" // lalr1.cc:859
    { }
#line 578 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 6:
#line 130 "sheep.yy" // lalr1.cc:859
    { }
#line 584 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 7:
#line 135 "sheep.yy" // lalr1.cc:859
    { }
#line 590 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 8:
#line 138 "sheep.yy" // lalr1.cc:859
    {  }
#line 596 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 9:
#line 139 "sheep.yy" // lalr1.cc:859
    {  }
#line 602 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 10:
#line 145 "sheep.yy" // lalr1.cc:859
    { }
#line 608 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 11:
#line 146 "sheep.yy" // lalr1.cc:859
    { }
#line 614 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 12:
#line 147 "sheep.yy" // lalr1.cc:859
    { }
#line 620 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 13:
#line 150 "sheep.yy" // lalr1.cc:859
    { builder.AddIntVariable(yystack_[0].value.as< std::string > (), 0); }
#line 626 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 14:
#line 151 "sheep.yy" // lalr1.cc:859
    { builder.AddIntVariable(yystack_[2].value.as< std::string > (), yystack_[0].value.as< int > ()); }
#line 632 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 15:
#line 152 "sheep.yy" // lalr1.cc:859
    { builder.AddIntVariable(yystack_[0].value.as< std::string > (), 0); }
#line 638 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 16:
#line 153 "sheep.yy" // lalr1.cc:859
    { builder.AddIntVariable(yystack_[2].value.as< std::string > (), yystack_[0].value.as< int > ()); }
#line 644 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 17:
#line 156 "sheep.yy" // lalr1.cc:859
    { builder.AddFloatVariable(yystack_[0].value.as< std::string > (), 0.0f); }
#line 650 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 18:
#line 157 "sheep.yy" // lalr1.cc:859
    { builder.AddFloatVariable(yystack_[2].value.as< std::string > (), yystack_[0].value.as< float > ()); }
#line 656 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 19:
#line 158 "sheep.yy" // lalr1.cc:859
    { builder.AddFloatVariable(yystack_[0].value.as< std::string > (), 0.0f); }
#line 662 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 20:
#line 159 "sheep.yy" // lalr1.cc:859
    { builder.AddFloatVariable(yystack_[2].value.as< std::string > (), yystack_[0].value.as< float > ()); }
#line 668 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 21:
#line 162 "sheep.yy" // lalr1.cc:859
    { builder.AddStringVariable(yystack_[0].value.as< std::string > (), ""); }
#line 674 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 22:
#line 163 "sheep.yy" // lalr1.cc:859
    { builder.AddStringVariable(yystack_[2].value.as< std::string > (), yystack_[0].value.as< std::string > ()); }
#line 680 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 23:
#line 164 "sheep.yy" // lalr1.cc:859
    { builder.AddStringVariable(yystack_[0].value.as< std::string > (), ""); }
#line 686 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 24:
#line 165 "sheep.yy" // lalr1.cc:859
    { builder.AddStringVariable(yystack_[2].value.as< std::string > (), yystack_[0].value.as< std::string > ()); }
#line 692 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 25:
#line 171 "sheep.yy" // lalr1.cc:859
    { }
#line 698 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 26:
#line 175 "sheep.yy" // lalr1.cc:859
    { }
#line 704 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 27:
#line 176 "sheep.yy" // lalr1.cc:859
    { }
#line 710 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 28:
#line 181 "sheep.yy" // lalr1.cc:859
    { builder.StartFunction(yystack_[2].value.as< std::string > ()); }
#line 716 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 29:
#line 181 "sheep.yy" // lalr1.cc:859
    { builder.EndFunction(yystack_[6].value.as< std::string > ()); }
#line 722 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 32:
#line 190 "sheep.yy" // lalr1.cc:859
    { builder.Store(yystack_[3].value.as< std::string > ()); }
#line 728 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 33:
#line 191 "sheep.yy" // lalr1.cc:859
    { }
#line 734 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 34:
#line 192 "sheep.yy" // lalr1.cc:859
    { builder.ReturnV(); }
#line 740 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 35:
#line 193 "sheep.yy" // lalr1.cc:859
    { builder.Breakpoint(); }
#line 746 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 36:
#line 194 "sheep.yy" // lalr1.cc:859
    { builder.SitnSpin(); }
#line 752 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 37:
#line 195 "sheep.yy" // lalr1.cc:859
    { builder.BranchGoto(yystack_[1].value.as< std::string > ()); }
#line 758 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 38:
#line 196 "sheep.yy" // lalr1.cc:859
    { builder.AddGoto(yystack_[1].value.as< std::string > ()); }
#line 764 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 39:
#line 197 "sheep.yy" // lalr1.cc:859
    { builder.BeginWait(); builder.EndWait(); }
#line 770 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 40:
#line 198 "sheep.yy" // lalr1.cc:859
    { builder.BeginWait(); }
#line 776 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 41:
#line 198 "sheep.yy" // lalr1.cc:859
    { builder.EndWait(); }
#line 782 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 42:
#line 199 "sheep.yy" // lalr1.cc:859
    { builder.BeginWait(); }
#line 788 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 43:
#line 199 "sheep.yy" // lalr1.cc:859
    { builder.EndWait(); }
#line 794 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 45:
#line 201 "sheep.yy" // lalr1.cc:859
    { }
#line 800 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 48:
#line 210 "sheep.yy" // lalr1.cc:859
    { yylhs.value.as< SheepValue > () = yystack_[0].value.as< SheepValue > (); }
#line 806 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 49:
#line 211 "sheep.yy" // lalr1.cc:859
    { auto type = builder.Load(yystack_[0].value.as< std::string > ()); yylhs.value.as< SheepValue > () = SheepValue(type); }
#line 812 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 50:
#line 212 "sheep.yy" // lalr1.cc:859
    { builder.PushI(yystack_[0].value.as< int > ()); yylhs.value.as< SheepValue > () = SheepValue(yystack_[0].value.as< int > ()); }
#line 818 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 51:
#line 213 "sheep.yy" // lalr1.cc:859
    { builder.PushF(yystack_[0].value.as< float > ()); yylhs.value.as< SheepValue > () = SheepValue(yystack_[0].value.as< float > ()); }
#line 824 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 52:
#line 214 "sheep.yy" // lalr1.cc:859
    { builder.AddStringConst(yystack_[0].value.as< std::string > ()); builder.PushS(yystack_[0].value.as< std::string > ()); yylhs.value.as< SheepValue > () = SheepValue(""); }
#line 830 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 53:
#line 216 "sheep.yy" // lalr1.cc:859
    { auto type = builder.Add(yystack_[2].value.as< SheepValue > (), yystack_[0].value.as< SheepValue > ()); yylhs.value.as< SheepValue > () = SheepValue(type); }
#line 836 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 54:
#line 217 "sheep.yy" // lalr1.cc:859
    { auto type = builder.Subtract(yystack_[2].value.as< SheepValue > (), yystack_[0].value.as< SheepValue > ()); yylhs.value.as< SheepValue > () = SheepValue(type); }
#line 842 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 55:
#line 218 "sheep.yy" // lalr1.cc:859
    { auto type = builder.Multiply(yystack_[2].value.as< SheepValue > (), yystack_[0].value.as< SheepValue > ()); yylhs.value.as< SheepValue > () = SheepValue(type); }
#line 848 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 56:
#line 219 "sheep.yy" // lalr1.cc:859
    { auto type = builder.Divide(yystack_[2].value.as< SheepValue > (), yystack_[0].value.as< SheepValue > ()); yylhs.value.as< SheepValue > () = SheepValue(type); }
#line 854 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 57:
#line 220 "sheep.yy" // lalr1.cc:859
    { builder.Modulo(yystack_[2].value.as< SheepValue > (), yystack_[0].value.as< SheepValue > ()); yylhs.value.as< SheepValue > () = SheepValue(SheepValueType::Int); }
#line 860 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 58:
#line 221 "sheep.yy" // lalr1.cc:859
    { builder.Negate(yystack_[0].value.as< SheepValue > ()); }
#line 866 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 59:
#line 223 "sheep.yy" // lalr1.cc:859
    { auto type = builder.IsLess(yystack_[2].value.as< SheepValue > (), yystack_[0].value.as< SheepValue > ()); yylhs.value.as< SheepValue > () = SheepValue(type); }
#line 872 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 60:
#line 224 "sheep.yy" // lalr1.cc:859
    { auto type = builder.IsGreater(yystack_[2].value.as< SheepValue > (), yystack_[0].value.as< SheepValue > ()); yylhs.value.as< SheepValue > () = SheepValue(type); }
#line 878 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 61:
#line 225 "sheep.yy" // lalr1.cc:859
    { auto type = builder.IsLessEqual(yystack_[2].value.as< SheepValue > (), yystack_[0].value.as< SheepValue > ()); yylhs.value.as< SheepValue > () = SheepValue(type); }
#line 884 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 62:
#line 226 "sheep.yy" // lalr1.cc:859
    { auto type = builder.IsGreaterEqual(yystack_[2].value.as< SheepValue > (), yystack_[0].value.as< SheepValue > ()); yylhs.value.as< SheepValue > () = SheepValue(type); }
#line 890 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 63:
#line 227 "sheep.yy" // lalr1.cc:859
    { auto type = builder.IsEqual(yystack_[2].value.as< SheepValue > (), yystack_[0].value.as< SheepValue > ()); yylhs.value.as< SheepValue > () = SheepValue(type); }
#line 896 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 64:
#line 228 "sheep.yy" // lalr1.cc:859
    { auto type = builder.IsNotEqual(yystack_[2].value.as< SheepValue > (), yystack_[0].value.as< SheepValue > ()); yylhs.value.as< SheepValue > () = SheepValue(type); }
#line 902 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 65:
#line 229 "sheep.yy" // lalr1.cc:859
    { builder.Or(yystack_[2].value.as< SheepValue > (), yystack_[0].value.as< SheepValue > ()); yylhs.value.as< SheepValue > () = SheepValue(SheepValueType::Int); }
#line 908 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 66:
#line 230 "sheep.yy" // lalr1.cc:859
    { builder.And(yystack_[2].value.as< SheepValue > (), yystack_[0].value.as< SheepValue > ()); yylhs.value.as< SheepValue > () = SheepValue(SheepValueType::Int); }
#line 914 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 67:
#line 231 "sheep.yy" // lalr1.cc:859
    { builder.Not(); yylhs.value.as< SheepValue > () = yystack_[0].value.as< SheepValue > (); }
#line 920 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 68:
#line 233 "sheep.yy" // lalr1.cc:859
    { yylhs.value.as< SheepValue > () = yystack_[1].value.as< SheepValue > (); }
#line 926 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 69:
#line 238 "sheep.yy" // lalr1.cc:859
    { auto type = builder.CallSysFunction(yystack_[3].value.as< std::string > ()); yylhs.value.as< SheepValue > () = SheepValue(type); }
#line 932 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 70:
#line 242 "sheep.yy" // lalr1.cc:859
    { }
#line 938 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 71:
#line 243 "sheep.yy" // lalr1.cc:859
    { }
#line 944 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 72:
#line 244 "sheep.yy" // lalr1.cc:859
    { }
#line 950 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 77:
#line 255 "sheep.yy" // lalr1.cc:859
    { }
#line 956 "sheep.tab.cc" // lalr1.cc:859
    break;


#line 960 "sheep.tab.cc" // lalr1.cc:859
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
    if ((false))
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


  const signed char Parser::yypact_ninf_ = -61;

  const signed char Parser::yytable_ninf_ = -43;

  const short int
  Parser::yypact_[] =
  {
       4,    11,    16,   -61,    38,     8,   -61,   -61,   -61,    54,
     -61,   -61,    -8,     6,    25,    13,    31,   -61,   -11,   -61,
     -61,    21,    51,    52,   -61,   -61,   -61,    -9,    53,   -11,
     -11,   -61,   -61,   196,   -61,   -61,    59,   -61,    60,   -61,
     -61,    44,    57,    61,   -61,   -11,    77,   -61,   -61,   170,
      80,   -61,    63,    75,   -61,   -61,   -61,   -11,   -11,   -61,
     -61,   -11,   -11,   -11,   -11,   -11,   -11,   -11,   -11,   -11,
     -11,   -11,   -11,   -11,    20,   -61,    89,    64,    -6,    81,
      14,    88,    48,   183,   -61,   -61,   -61,   -61,   -61,   140,
     196,    -5,   208,   219,   152,   152,    74,    74,    74,    74,
      35,    35,   -61,   -61,   -61,   -61,   -61,   -61,    79,    82,
     -61,    94,    95,   -61,   100,   101,   -61,   118,   115,   -61,
     -11,   -61,   119,   -61,   107,   -61,   108,   -61,   109,   -61,
     -61,   196,   -61,   121,   123,   126,   141,   -61,   -61,   -61,
     -61
  };

  const unsigned char
  Parser::yydefact_[] =
  {
       2,     0,     0,    30,     0,     3,     4,    26,     8,     0,
       1,     5,     0,     0,     0,     0,     0,    45,     0,    30,
       6,    40,     0,     0,    50,    51,    52,    49,     0,     0,
       0,    31,    44,    33,    48,    46,    73,    25,     0,    27,
       7,     0,     0,     0,     9,     0,     0,    34,    49,     0,
       0,    39,     0,     0,    35,    36,    38,     0,    70,    67,
      58,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    74,     0,    13,     0,    17,
       0,    21,     0,     0,    37,    68,    47,    41,    30,     0,
      71,     0,    65,    66,    63,    64,    59,    61,    60,    62,
      53,    54,    55,    56,    57,    76,    77,    28,     0,     0,
      10,     0,     0,    11,     0,     0,    12,     0,     0,    32,
       0,    69,     0,    14,    15,    18,    19,    22,    23,    75,
      43,    72,    30,     0,     0,     0,     0,    16,    20,    24,
      29
  };

  const short int
  Parser::yypgoto_[] =
  {
     -61,   -61,   -61,   -61,   -61,   -61,   -61,   -61,   146,   -61,
     -61,   -61,   -19,   -61,   -61,   -61,   -60,   -17,   104,   -61,
      90,   -61,   -61
  };

  const signed char
  Parser::yydefgoto_[] =
  {
      -1,     4,     5,    13,    44,    78,    80,    82,     6,    12,
      39,   122,     9,    31,    52,    53,    32,    33,    34,    91,
      35,    36,    75
  };

  const short int
  Parser::yytable_[] =
  {
      50,    49,    56,    18,   109,   120,   110,     1,     2,    37,
     121,     1,    59,    60,   105,    24,    25,    26,    48,    28,
       3,    38,    57,    40,   112,    14,   113,     7,    83,    41,
      42,    43,     8,    51,    29,    30,    19,   -42,    10,    45,
      89,    90,    46,    47,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   129,   115,    14,
     116,    15,    16,    54,    55,    74,    17,    58,    18,   118,
      19,    20,    21,    77,    76,    22,    23,    71,    72,    73,
      24,    25,    26,    27,    28,    14,    79,    15,    16,    84,
      81,    88,    17,    28,    18,   108,    19,    86,    21,    29,
      30,    22,    23,   131,   107,   123,    24,    25,    26,    27,
      28,   124,   111,   136,    69,    70,    71,    72,    73,   114,
      14,   125,    15,    16,   126,    29,    30,    17,   127,    18,
     128,    19,   130,    21,    19,   132,    22,    23,   133,   134,
     135,    24,    25,    26,    27,    28,    14,   137,    15,    16,
     138,    11,   119,    17,   139,    18,    87,    19,   140,    21,
      29,    30,    22,    23,   106,     0,     0,    24,    25,    26,
      27,    28,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    85,    29,    30,    65,    66,
      67,    68,    69,    70,    71,    72,    73,     0,   117,     0,
       0,     0,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73
  };

  const short int
  Parser::yycheck_[] =
  {
      19,    18,    11,    14,    10,    10,    12,     3,     4,    17,
      15,     3,    29,    30,    74,    26,    27,    28,    29,    30,
      16,    29,    31,    17,    10,     5,    12,    16,    45,    23,
      24,    25,    16,    12,    45,    46,    16,    16,     0,    14,
      57,    58,    29,    12,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,   117,    10,     5,
      12,     7,     8,    12,    12,     6,    12,    14,    14,    88,
      16,    17,    18,    29,    14,    21,    22,    42,    43,    44,
      26,    27,    28,    29,    30,     5,    29,     7,     8,    12,
      29,    16,    12,    30,    14,    31,    16,    17,    18,    45,
      46,    21,    22,   120,    15,    26,    26,    27,    28,    29,
      30,    29,    31,   132,    40,    41,    42,    43,    44,    31,
       5,    27,     7,     8,    29,    45,    46,    12,    28,    14,
      29,    16,    17,    18,    16,    16,    21,    22,    31,    31,
      31,    26,    27,    28,    29,    30,     5,    26,     7,     8,
      27,     5,    12,    12,    28,    14,    52,    16,    17,    18,
      45,    46,    21,    22,    74,    -1,    -1,    26,    27,    28,
      29,    30,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    15,    45,    46,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    -1,    15,    -1,
      -1,    -1,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44
  };

  const unsigned char
  Parser::yystos_[] =
  {
       0,     3,     4,    16,    48,    49,    55,    16,    16,    59,
       0,    55,    56,    50,     5,     7,     8,    12,    14,    16,
      17,    18,    21,    22,    26,    27,    28,    29,    30,    45,
      46,    60,    63,    64,    65,    67,    68,    17,    29,    57,
      17,    23,    24,    25,    51,    14,    29,    12,    29,    64,
      59,    12,    61,    62,    12,    12,    11,    31,    14,    64,
      64,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,     6,    69,    14,    29,    52,    29,
      53,    29,    54,    64,    12,    15,    17,    65,    16,    64,
      64,    66,    64,    64,    64,    64,    64,    64,    64,    64,
      64,    64,    64,    64,    64,    63,    67,    15,    31,    10,
      12,    31,    10,    12,    31,    10,    12,    15,    59,    12,
      10,    15,    58,    26,    29,    27,    29,    28,    29,    63,
      17,    64,    16,    31,    31,    31,    59,    26,    27,    28,
      17
  };

  const unsigned char
  Parser::yyr1_[] =
  {
       0,    47,    48,    48,    48,    48,    48,    49,    50,    50,
      51,    51,    51,    52,    52,    52,    52,    53,    53,    53,
      53,    54,    54,    54,    54,    55,    56,    56,    58,    57,
      59,    59,    60,    60,    60,    60,    60,    60,    60,    60,
      61,    60,    62,    60,    60,    60,    60,    63,    64,    64,
      64,    64,    64,    64,    64,    64,    64,    64,    64,    64,
      64,    64,    64,    64,    64,    64,    64,    64,    64,    65,
      66,    66,    66,    67,    67,    68,    69,    69
  };

  const unsigned char
  Parser::yyr2_[] =
  {
       0,     2,     0,     1,     1,     2,     3,     4,     0,     2,
       3,     3,     3,     1,     3,     3,     5,     1,     3,     3,
       5,     1,     3,     3,     5,     4,     0,     2,     0,     7,
       0,     2,     4,     1,     2,     2,     2,     3,     2,     2,
       0,     3,     0,     5,     1,     1,     1,     3,     1,     1,
       1,     1,     1,     3,     3,     3,     3,     3,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     2,     3,     4,
       0,     1,     3,     1,     2,     5,     2,     2
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
  "statement", "$@2", "$@3", "statements_block", "expr", "sysfunc_call",
  "sysfunc_call_args", "if_else_block", "if_statement", "else_statement", YY_NULLPTR
  };


  const unsigned char
  Parser::yyrline_[] =
  {
       0,   126,   126,   127,   128,   129,   130,   135,   138,   139,
     145,   146,   147,   150,   151,   152,   153,   156,   157,   158,
     159,   162,   163,   164,   165,   171,   175,   176,   181,   181,
     185,   186,   190,   191,   192,   193,   194,   195,   196,   197,
     198,   198,   199,   199,   200,   201,   202,   206,   210,   211,
     212,   213,   214,   216,   217,   218,   219,   220,   221,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   233,   238,
     242,   243,   244,   247,   248,   251,   254,   255
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


#line 18 "sheep.yy" // lalr1.cc:1167
} // Sheep
#line 1363 "sheep.tab.cc" // lalr1.cc:1167
#line 258 "sheep.yy" // lalr1.cc:1168


/* No epilogue is needed. */

