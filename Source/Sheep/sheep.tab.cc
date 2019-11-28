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
#line 3 "sheep.yy" // lalr1.cc:397

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
#line 51 "sheep.yy" // lalr1.cc:413

	#include "SheepCompiler.h"
	#include "SheepScriptBuilder.h"

	// When requesting yylex, redirect to scanner.yylex.
	#undef yylex
	#define yylex scanner.yylex

	// The C++ yytext is const, but we need to modify to remove
	// quotes in some cases. So, this is a bit of a hack.
	#undef yytext
	#define yytext scanner.GetYYText()

    // In case of error, just pass that back to our SheepCompiler class to handle.
    // This is probably the main reason we need to pass in the compiler reference!
	void Sheep::Parser::error(const location_type& loc, const std::string& msg)
	{
		compiler.Error(loc, msg);
	}

	#define BUILDER_ERROR_CHECK if(builder.CheckError(yyla.location, *this)) { YYERROR; }

#line 83 "sheep.tab.cc" // lalr1.cc:413


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

#line 20 "sheep.yy" // lalr1.cc:479
namespace Sheep {
#line 169 "sheep.tab.cc" // lalr1.cc:479

  /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
  std::string
  Parser::yytnamerr_ (const char *yystr)
  {
    if (*yystr == '"')
      {
        std::string yyr = "";
        char const *yyp = yystr;

        for (;;)
          switch (*++yyp)
            {
            case '\'':
            case ',':
              goto do_not_strip_quotes;

            case '\\':
              if (*++yyp != '\\')
                goto do_not_strip_quotes;
              // Fall through.
            default:
              yyr += *yyp;
              break;

            case '"':
              return yyr;
            }
      do_not_strip_quotes: ;
      }

    return yystr;
  }


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
      case 65: // expr
      case 66: // sysfunc_call
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
      case 65: // expr
      case 66: // sysfunc_call
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
      case 65: // expr
      case 66: // sysfunc_call
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
#line 160 "sheep.yy" // lalr1.cc:859
    { }
#line 603 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 3:
#line 161 "sheep.yy" // lalr1.cc:859
    { }
#line 609 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 4:
#line 162 "sheep.yy" // lalr1.cc:859
    { }
#line 615 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 5:
#line 163 "sheep.yy" // lalr1.cc:859
    { }
#line 621 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 6:
#line 164 "sheep.yy" // lalr1.cc:859
    { }
#line 627 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 7:
#line 169 "sheep.yy" // lalr1.cc:859
    { }
#line 633 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 8:
#line 172 "sheep.yy" // lalr1.cc:859
    {  }
#line 639 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 9:
#line 173 "sheep.yy" // lalr1.cc:859
    {  }
#line 645 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 10:
#line 179 "sheep.yy" // lalr1.cc:859
    { }
#line 651 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 11:
#line 180 "sheep.yy" // lalr1.cc:859
    { }
#line 657 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 12:
#line 181 "sheep.yy" // lalr1.cc:859
    { }
#line 663 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 13:
#line 184 "sheep.yy" // lalr1.cc:859
    { builder.AddIntVariable(yystack_[0].value.as< std::string > (), 0); }
#line 669 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 14:
#line 185 "sheep.yy" // lalr1.cc:859
    { builder.AddIntVariable(yystack_[2].value.as< std::string > (), yystack_[0].value.as< int > ()); }
#line 675 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 15:
#line 186 "sheep.yy" // lalr1.cc:859
    { builder.AddIntVariable(yystack_[0].value.as< std::string > (), 0); }
#line 681 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 16:
#line 187 "sheep.yy" // lalr1.cc:859
    { builder.AddIntVariable(yystack_[2].value.as< std::string > (), yystack_[0].value.as< int > ()); }
#line 687 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 17:
#line 190 "sheep.yy" // lalr1.cc:859
    { builder.AddFloatVariable(yystack_[0].value.as< std::string > (), 0.0f); }
#line 693 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 18:
#line 191 "sheep.yy" // lalr1.cc:859
    { builder.AddFloatVariable(yystack_[2].value.as< std::string > (), yystack_[0].value.as< float > ()); }
#line 699 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 19:
#line 192 "sheep.yy" // lalr1.cc:859
    { builder.AddFloatVariable(yystack_[0].value.as< std::string > (), 0.0f); }
#line 705 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 20:
#line 193 "sheep.yy" // lalr1.cc:859
    { builder.AddFloatVariable(yystack_[2].value.as< std::string > (), yystack_[0].value.as< float > ()); }
#line 711 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 21:
#line 196 "sheep.yy" // lalr1.cc:859
    { builder.AddStringVariable(yystack_[0].value.as< std::string > (), ""); }
#line 717 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 22:
#line 197 "sheep.yy" // lalr1.cc:859
    { builder.AddStringVariable(yystack_[2].value.as< std::string > (), yystack_[0].value.as< std::string > ()); }
#line 723 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 23:
#line 198 "sheep.yy" // lalr1.cc:859
    { builder.AddStringVariable(yystack_[0].value.as< std::string > (), ""); }
#line 729 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 24:
#line 199 "sheep.yy" // lalr1.cc:859
    { builder.AddStringVariable(yystack_[2].value.as< std::string > (), yystack_[0].value.as< std::string > ()); }
#line 735 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 25:
#line 205 "sheep.yy" // lalr1.cc:859
    { }
#line 741 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 26:
#line 209 "sheep.yy" // lalr1.cc:859
    { }
#line 747 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 27:
#line 210 "sheep.yy" // lalr1.cc:859
    { }
#line 753 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 28:
#line 215 "sheep.yy" // lalr1.cc:859
    { builder.StartFunction(yystack_[2].value.as< std::string > ()); }
#line 759 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 29:
#line 215 "sheep.yy" // lalr1.cc:859
    { builder.EndFunction(yystack_[6].value.as< std::string > ()); }
#line 765 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 32:
#line 224 "sheep.yy" // lalr1.cc:859
    { builder.Store(yystack_[3].value.as< std::string > ()); }
#line 771 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 33:
#line 225 "sheep.yy" // lalr1.cc:859
    { }
#line 777 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 34:
#line 226 "sheep.yy" // lalr1.cc:859
    { builder.ReturnV(); }
#line 783 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 35:
#line 227 "sheep.yy" // lalr1.cc:859
    { builder.Breakpoint(); }
#line 789 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 36:
#line 228 "sheep.yy" // lalr1.cc:859
    { builder.SitnSpin(); }
#line 795 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 37:
#line 229 "sheep.yy" // lalr1.cc:859
    { builder.BranchGoto(yystack_[1].value.as< std::string > ()); }
#line 801 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 38:
#line 230 "sheep.yy" // lalr1.cc:859
    { builder.AddGoto(yystack_[1].value.as< std::string > ()); }
#line 807 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 39:
#line 231 "sheep.yy" // lalr1.cc:859
    { builder.BeginWait(); builder.EndWait(); }
#line 813 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 40:
#line 232 "sheep.yy" // lalr1.cc:859
    { builder.BeginWait(); }
#line 819 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 41:
#line 232 "sheep.yy" // lalr1.cc:859
    { builder.EndWait(); }
#line 825 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 42:
#line 233 "sheep.yy" // lalr1.cc:859
    { builder.BeginWait(); }
#line 831 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 43:
#line 233 "sheep.yy" // lalr1.cc:859
    { builder.EndWait(); }
#line 837 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 45:
#line 235 "sheep.yy" // lalr1.cc:859
    { }
#line 843 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 46:
#line 236 "sheep.yy" // lalr1.cc:859
    { builder.BeginIfElseBlock(); }
#line 849 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 47:
#line 236 "sheep.yy" // lalr1.cc:859
    { builder.EndIfElseBlock(); }
#line 855 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 49:
#line 244 "sheep.yy" // lalr1.cc:859
    { yylhs.value.as< SheepValue > () = yystack_[0].value.as< SheepValue > (); }
#line 861 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 50:
#line 245 "sheep.yy" // lalr1.cc:859
    { auto type = builder.Load(yystack_[0].value.as< std::string > ()); yylhs.value.as< SheepValue > () = SheepValue(type); }
#line 867 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 51:
#line 246 "sheep.yy" // lalr1.cc:859
    { builder.PushI(yystack_[0].value.as< int > ()); yylhs.value.as< SheepValue > () = SheepValue(yystack_[0].value.as< int > ()); }
#line 873 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 52:
#line 247 "sheep.yy" // lalr1.cc:859
    { builder.PushF(yystack_[0].value.as< float > ()); yylhs.value.as< SheepValue > () = SheepValue(yystack_[0].value.as< float > ()); }
#line 879 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 53:
#line 248 "sheep.yy" // lalr1.cc:859
    { builder.AddStringConst(yystack_[0].value.as< std::string > ()); builder.PushS(yystack_[0].value.as< std::string > ()); yylhs.value.as< SheepValue > () = SheepValue(""); }
#line 885 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 54:
#line 250 "sheep.yy" // lalr1.cc:859
    { auto type = builder.Add(yystack_[2].value.as< SheepValue > (), yystack_[0].value.as< SheepValue > ()); yylhs.value.as< SheepValue > () = SheepValue(type); }
#line 891 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 55:
#line 251 "sheep.yy" // lalr1.cc:859
    { auto type = builder.Subtract(yystack_[2].value.as< SheepValue > (), yystack_[0].value.as< SheepValue > ()); yylhs.value.as< SheepValue > () = SheepValue(type); }
#line 897 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 56:
#line 252 "sheep.yy" // lalr1.cc:859
    { auto type = builder.Multiply(yystack_[2].value.as< SheepValue > (), yystack_[0].value.as< SheepValue > ()); yylhs.value.as< SheepValue > () = SheepValue(type); }
#line 903 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 57:
#line 253 "sheep.yy" // lalr1.cc:859
    { auto type = builder.Divide(yystack_[2].value.as< SheepValue > (), yystack_[0].value.as< SheepValue > ()); yylhs.value.as< SheepValue > () = SheepValue(type); }
#line 909 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 58:
#line 254 "sheep.yy" // lalr1.cc:859
    { builder.Modulo(yystack_[2].value.as< SheepValue > (), yystack_[0].value.as< SheepValue > ()); yylhs.value.as< SheepValue > () = SheepValue(SheepValueType::Int); }
#line 915 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 59:
#line 255 "sheep.yy" // lalr1.cc:859
    { builder.Negate(yystack_[0].value.as< SheepValue > ()); }
#line 921 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 60:
#line 257 "sheep.yy" // lalr1.cc:859
    { auto type = builder.IsLess(yystack_[2].value.as< SheepValue > (), yystack_[0].value.as< SheepValue > ()); yylhs.value.as< SheepValue > () = SheepValue(type); }
#line 927 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 61:
#line 258 "sheep.yy" // lalr1.cc:859
    { auto type = builder.IsGreater(yystack_[2].value.as< SheepValue > (), yystack_[0].value.as< SheepValue > ()); yylhs.value.as< SheepValue > () = SheepValue(type); }
#line 933 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 62:
#line 259 "sheep.yy" // lalr1.cc:859
    { auto type = builder.IsLessEqual(yystack_[2].value.as< SheepValue > (), yystack_[0].value.as< SheepValue > ()); yylhs.value.as< SheepValue > () = SheepValue(type); }
#line 939 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 63:
#line 260 "sheep.yy" // lalr1.cc:859
    { auto type = builder.IsGreaterEqual(yystack_[2].value.as< SheepValue > (), yystack_[0].value.as< SheepValue > ()); yylhs.value.as< SheepValue > () = SheepValue(type); }
#line 945 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 64:
#line 261 "sheep.yy" // lalr1.cc:859
    { auto type = builder.IsEqual(yystack_[2].value.as< SheepValue > (), yystack_[0].value.as< SheepValue > ()); yylhs.value.as< SheepValue > () = SheepValue(type); }
#line 951 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 65:
#line 262 "sheep.yy" // lalr1.cc:859
    { auto type = builder.IsNotEqual(yystack_[2].value.as< SheepValue > (), yystack_[0].value.as< SheepValue > ()); yylhs.value.as< SheepValue > () = SheepValue(type); }
#line 957 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 66:
#line 263 "sheep.yy" // lalr1.cc:859
    { builder.Or(yystack_[2].value.as< SheepValue > (), yystack_[0].value.as< SheepValue > ()); yylhs.value.as< SheepValue > () = SheepValue(SheepValueType::Int); }
#line 963 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 67:
#line 264 "sheep.yy" // lalr1.cc:859
    { builder.And(yystack_[2].value.as< SheepValue > (), yystack_[0].value.as< SheepValue > ()); yylhs.value.as< SheepValue > () = SheepValue(SheepValueType::Int); }
#line 969 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 68:
#line 265 "sheep.yy" // lalr1.cc:859
    { builder.Not(); yylhs.value.as< SheepValue > () = yystack_[0].value.as< SheepValue > (); }
#line 975 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 69:
#line 267 "sheep.yy" // lalr1.cc:859
    { yylhs.value.as< SheepValue > () = yystack_[1].value.as< SheepValue > (); }
#line 981 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 70:
#line 272 "sheep.yy" // lalr1.cc:859
    { auto type = builder.CallSysFunc(yystack_[3].value.as< std::string > ()); yylhs.value.as< SheepValue > () = SheepValue(type); BUILDER_ERROR_CHECK; }
#line 987 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 71:
#line 276 "sheep.yy" // lalr1.cc:859
    { }
#line 993 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 72:
#line 277 "sheep.yy" // lalr1.cc:859
    { builder.AddToSysFuncArgCount(); }
#line 999 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 73:
#line 278 "sheep.yy" // lalr1.cc:859
    { builder.AddToSysFuncArgCount(); }
#line 1005 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 76:
#line 285 "sheep.yy" // lalr1.cc:859
    { builder.BeginIfBlock(); }
#line 1011 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 77:
#line 285 "sheep.yy" // lalr1.cc:859
    { builder.EndIfBlock(); }
#line 1017 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 78:
#line 288 "sheep.yy" // lalr1.cc:859
    { builder.BeginElseBlock(); }
#line 1023 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 79:
#line 288 "sheep.yy" // lalr1.cc:859
    { builder.EndElseBlock(); }
#line 1029 "sheep.tab.cc" // lalr1.cc:859
    break;

  case 80:
#line 289 "sheep.yy" // lalr1.cc:859
    {  }
#line 1035 "sheep.tab.cc" // lalr1.cc:859
    break;


#line 1039 "sheep.tab.cc" // lalr1.cc:859
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
  Parser::yysyntax_error_ (state_type yystate, const symbol_type& yyla) const
  {
    // Number of reported tokens (one for the "unexpected", one per
    // "expected").
    size_t yycount = 0;
    // Its maximum.
    enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
    // Arguments of yyformat.
    char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];

    /* There are many possibilities here to consider:
       - If this state is a consistent state with a default action, then
         the only way this function was invoked is if the default action
         is an error action.  In that case, don't check for expected
         tokens because there are none.
       - The only way there can be no lookahead present (in yyla) is
         if this state is a consistent state with a default action.
         Thus, detecting the absence of a lookahead is sufficient to
         determine that there is no unexpected or expected token to
         report.  In that case, just report a simple "syntax error".
       - Don't assume there isn't a lookahead just because this state is
         a consistent state with a default action.  There might have
         been a previous inconsistent state, consistent state with a
         non-default action, or user semantic action that manipulated
         yyla.  (However, yyla is currently not documented for users.)
       - Of course, the expected token list depends on states to have
         correct lookahead information, and it depends on the parser not
         to perform extra reductions after fetching a lookahead from the
         scanner and before detecting a syntax error.  Thus, state
         merging (from LALR or IELR) and default reductions corrupt the
         expected token list.  However, the list is correct for
         canonical LR with one exception: it will still contain any
         token that will not be accepted due to an error action in a
         later state.
    */
    if (!yyla.empty ())
      {
        int yytoken = yyla.type_get ();
        yyarg[yycount++] = yytname_[yytoken];
        int yyn = yypact_[yystate];
        if (!yy_pact_value_is_default_ (yyn))
          {
            /* Start YYX at -YYN if negative to avoid negative indexes in
               YYCHECK.  In other words, skip the first -YYN actions for
               this state because they are default actions.  */
            int yyxbegin = yyn < 0 ? -yyn : 0;
            // Stay within bounds of both yycheck and yytname.
            int yychecklim = yylast_ - yyn + 1;
            int yyxend = yychecklim < yyntokens_ ? yychecklim : yyntokens_;
            for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
              if (yycheck_[yyx + yyn] == yyx && yyx != yyterror_
                  && !yy_table_value_is_error_ (yytable_[yyx + yyn]))
                {
                  if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                    {
                      yycount = 1;
                      break;
                    }
                  else
                    yyarg[yycount++] = yytname_[yyx];
                }
          }
      }

    char const* yyformat = YY_NULLPTR;
    switch (yycount)
      {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
        YYCASE_(0, YY_("syntax error"));
        YYCASE_(1, YY_("syntax error, unexpected %s"));
        YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    std::string yyres;
    // Argument number.
    size_t yyi = 0;
    for (char const* yyp = yyformat; *yyp; ++yyp)
      if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
          yyres += yytnamerr_ (yyarg[yyi++]);
          ++yyp;
        }
      else
        yyres += *yyp;
    return yyres;
  }


  const signed char Parser::yypact_ninf_ = -117;

  const signed char Parser::yytable_ninf_ = -43;

  const short int
  Parser::yypact_[] =
  {
      29,    -8,     1,  -117,    20,    33,  -117,  -117,  -117,    -4,
    -117,  -117,   -13,     4,    11,    52,  -117,    59,  -117,  -117,
      23,    53,    63,  -117,  -117,  -117,    -1,    32,    59,    59,
    -117,    65,  -117,   178,  -117,  -117,    66,  -117,  -117,    79,
      82,    83,  -117,   103,  -117,  -117,   152,    54,  -117,    88,
     105,  -117,  -117,  -117,    59,    59,  -117,  -117,   108,  -117,
     124,    59,    59,    59,    59,    59,    59,    59,    59,    59,
      59,    59,    59,    59,   113,    96,    68,   106,    86,   107,
      87,  -117,  -117,  -117,  -117,  -117,   120,   178,    22,    59,
      65,  -117,   190,   201,   132,   132,    50,    50,    50,    50,
      16,    16,  -117,  -117,  -117,  -117,   118,   116,  -117,   119,
     148,  -117,   151,   153,  -117,    95,  -117,    59,  -117,   165,
    -117,   166,   167,  -117,   215,  -117,   216,  -117,   217,  -117,
     178,  -117,  -117,  -117,   157,   222,   223,   166,   121,  -117,
    -117,  -117,  -117,  -117
  };

  const unsigned char
  Parser::yydefact_[] =
  {
       2,     0,     0,    30,     0,     3,     4,    26,     8,    46,
       1,     5,     0,     0,     0,     0,    45,     0,    30,     6,
      40,     0,     0,    51,    52,    53,    50,     0,     0,     0,
      31,     0,    44,    33,    49,    25,     0,    27,     7,     0,
       0,     0,     9,     0,    34,    50,     0,    46,    39,     0,
       0,    35,    36,    38,     0,    71,    68,    59,     0,    47,
      74,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    13,     0,    17,     0,    21,
       0,    37,    69,    48,    41,    30,     0,    72,     0,     0,
      78,    75,    66,    67,    64,    65,    60,    62,    61,    63,
      54,    55,    56,    57,    58,    28,     0,     0,    10,     0,
       0,    11,     0,     0,    12,    46,    32,     0,    70,     0,
      80,     0,     0,    14,    15,    18,    19,    22,    23,    43,
      73,    76,    79,    30,     0,     0,     0,     0,    46,    16,
      20,    24,    77,    29
  };

  const short int
  Parser::yypgoto_[] =
  {
    -117,  -117,  -117,  -117,  -117,  -117,  -117,  -117,   129,  -117,
    -117,  -117,   -18,  -117,  -117,  -117,  -117,  -116,   -16,   203,
    -117,   160,  -117,  -117,  -117,  -117
  };

  const short int
  Parser::yydefgoto_[] =
  {
      -1,     4,     5,    13,    42,    76,    78,    80,     6,    12,
      37,   122,     9,    30,    49,    50,    31,    32,    33,    34,
      88,    59,    60,   137,    91,   121
  };

  const short int
  Parser::yytable_[] =
  {
      47,    46,    35,    14,    15,   132,     7,    16,    17,    53,
      18,    19,    56,    57,    20,     8,    36,    21,    22,    38,
      10,   142,    23,    24,    25,    26,    27,    39,    40,    41,
      54,   117,     1,     2,    48,   118,     1,   -42,    86,    87,
      43,    28,    29,     3,    55,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,    71,    72,
      73,    14,    15,    44,    51,    16,    17,   115,    18,    83,
      58,    17,    20,   119,    52,    21,    22,   107,    74,   108,
      23,    24,    25,    26,    27,    23,    24,    25,    45,    27,
      69,    70,    71,    72,    73,   110,   113,   111,   114,    28,
      29,   130,    14,    15,    28,    29,    16,    17,    75,    18,
     129,    77,    79,    20,    81,   138,    21,    22,    27,    85,
      89,    23,    24,    25,    26,    27,   105,   106,    14,    15,
      90,   116,    16,    17,    11,    18,   143,   109,   112,    20,
      28,    29,    21,    22,   123,   124,   125,    23,    24,    25,
      26,    27,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    82,    28,    29,    65,    66,
      67,    68,    69,    70,    71,    72,    73,   126,   131,   127,
      18,   133,   128,   139,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,   134,   135,   136,   140,
     120,   141,    84
  };

  const unsigned char
  Parser::yycheck_[] =
  {
      18,    17,    15,     7,     8,   121,    14,    11,    12,    10,
      14,    15,    28,    29,    18,    14,    29,    21,    22,    15,
       0,   137,    26,    27,    28,    29,    30,    23,    24,    25,
      31,     9,     3,     4,    11,    13,     3,    14,    54,    55,
      29,    45,    46,    14,    12,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    42,    43,
      44,     7,     8,    11,    11,    11,    12,    85,    14,    15,
       5,    12,    18,    89,    11,    21,    22,     9,    12,    11,
      26,    27,    28,    29,    30,    26,    27,    28,    29,    30,
      40,    41,    42,    43,    44,     9,     9,    11,    11,    45,
      46,   117,     7,     8,    45,    46,    11,    12,    29,    14,
      15,    29,    29,    18,    11,   133,    21,    22,    30,    14,
      12,    26,    27,    28,    29,    30,    13,    31,     7,     8,
       6,    11,    11,    12,     5,    14,    15,    31,    31,    18,
      45,    46,    21,    22,    26,    29,    27,    26,    27,    28,
      29,    30,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    13,    45,    46,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    29,    13,    28,
      14,    14,    29,    26,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    31,    31,    31,    27,
      90,    28,    49
  };

  const unsigned char
  Parser::yystos_[] =
  {
       0,     3,     4,    14,    48,    49,    55,    14,    14,    59,
       0,    55,    56,    50,     7,     8,    11,    12,    14,    15,
      18,    21,    22,    26,    27,    28,    29,    30,    45,    46,
      60,    63,    64,    65,    66,    15,    29,    57,    15,    23,
      24,    25,    51,    29,    11,    29,    65,    59,    11,    61,
      62,    11,    11,    10,    31,    12,    65,    65,     5,    68,
      69,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    12,    29,    52,    29,    53,    29,
      54,    11,    13,    15,    66,    14,    65,    65,    67,    12,
       6,    71,    65,    65,    65,    65,    65,    65,    65,    65,
      65,    65,    65,    65,    65,    13,    31,     9,    11,    31,
       9,    11,    31,     9,    11,    59,    11,     9,    13,    65,
      68,    72,    58,    26,    29,    27,    29,    28,    29,    15,
      65,    13,    64,    14,    31,    31,    31,    70,    59,    26,
      27,    28,    64,    15
  };

  const unsigned char
  Parser::yyr1_[] =
  {
       0,    47,    48,    48,    48,    48,    48,    49,    50,    50,
      51,    51,    51,    52,    52,    52,    52,    53,    53,    53,
      53,    54,    54,    54,    54,    55,    56,    56,    58,    57,
      59,    59,    60,    60,    60,    60,    60,    60,    60,    60,
      61,    60,    62,    60,    60,    60,    63,    60,    64,    65,
      65,    65,    65,    65,    65,    65,    65,    65,    65,    65,
      65,    65,    65,    65,    65,    65,    65,    65,    65,    65,
      66,    67,    67,    67,    68,    68,    70,    69,    72,    71,
      71
  };

  const unsigned char
  Parser::yyr2_[] =
  {
       0,     2,     0,     1,     1,     2,     3,     4,     0,     2,
       3,     3,     3,     1,     3,     3,     5,     1,     3,     3,
       5,     1,     3,     3,     5,     4,     0,     2,     0,     7,
       0,     2,     4,     1,     2,     2,     2,     3,     2,     2,
       0,     3,     0,     5,     1,     1,     0,     2,     3,     1,
       1,     1,     1,     1,     3,     3,     3,     3,     3,     2,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     3,
       4,     0,     1,     3,     1,     2,     0,     6,     0,     3,
       2
  };



  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a yyntokens_, nonterminals.
  const char*
  const Parser::yytname_[] =
  {
  "\"end of file\"", "error", "$undefined", "CODE", "SYMBOLS", "IF",
  "ELSE", "GOTO", "RETURN", "\",\"", "\":\"", "\";\"", "\"(\"", "\")\"",
  "\"{\"", "\"}\"", "DOLLAR", "QUOTE", "\"wait\"", "YIELD", "EXPORT",
  "BREAKPOINT", "SITNSPIN", "INTVAR", "FLOATVAR", "STRINGVAR", "INT",
  "FLOAT", "STRING", "USERID", "SYSID", "ASSIGN", "OR", "AND", "EQUAL",
  "NOTEQUAL", "LT", "LTE", "GT", "GTE", "PLUS", "MINUS", "MULTIPLY",
  "DIVIDE", "MOD", "NOT", "NEGATE", "$accept", "script", "symbols_section",
  "symbol_decls", "symbol_decl", "symbol_decl_int", "symbol_decl_float",
  "symbol_decl_string", "code_section", "functions", "function", "$@1",
  "statements", "statement", "$@2", "$@3", "$@4", "statements_block",
  "expr", "sysfunc_call", "sysfunc_call_args", "if_else_block",
  "if_statement", "$@5", "else_statement", "$@6", YY_NULLPTR
  };

#if YYDEBUG
  const unsigned short int
  Parser::yyrline_[] =
  {
       0,   160,   160,   161,   162,   163,   164,   169,   172,   173,
     179,   180,   181,   184,   185,   186,   187,   190,   191,   192,
     193,   196,   197,   198,   199,   205,   209,   210,   215,   215,
     219,   220,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   232,   233,   233,   234,   235,   236,   236,   240,   244,
     245,   246,   247,   248,   250,   251,   252,   253,   254,   255,
     257,   258,   259,   260,   261,   262,   263,   264,   265,   267,
     272,   276,   277,   278,   281,   282,   285,   285,   288,   288,
     289
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


#line 20 "sheep.yy" // lalr1.cc:1167
} // Sheep
#line 1532 "sheep.tab.cc" // lalr1.cc:1167
#line 292 "sheep.yy" // lalr1.cc:1168


/* No epilogue is needed. */

