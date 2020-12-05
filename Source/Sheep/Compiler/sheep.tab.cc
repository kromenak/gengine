// A Bison parser, made by GNU Bison 3.5.3.

// Skeleton implementation for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2015, 2018-2020 Free Software Foundation, Inc.

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

// Undocumented macros, especially those whose name start with YY_,
// are private implementation details.  Do not rely on them.

// "%code top" blocks.
#line 3 "sheep.yy"

	#include <cstdlib>
	#include <string>

#line 43 "sheep.tab.cc"




#include "sheep.tab.hh"


// Unqualified %code blocks.
#line 51 "sheep.yy"

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
		compiler.Error(&builder, loc, msg);
	}

	//#define BUILDER_ERROR_CHECK if(builder.CheckError(yyla.location, *this)) { YYERROR; }

#line 75 "sheep.tab.cc"


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

// Whether we are compiled with exception support.
#ifndef YY_EXCEPTIONS
# if defined __GNUC__ && !defined __EXCEPTIONS
#  define YY_EXCEPTIONS 0
# else
#  define YY_EXCEPTIONS 1
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
    while (false)
# endif


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
      *yycdebug_ << '\n';                       \
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
# define YY_SYMBOL_PRINT(Title, Symbol)  YYUSE (Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void> (0)
# define YY_STACK_PRINT()                static_cast<void> (0)

#endif // !YYDEBUG

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyla.clear ())

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)

#line 20 "sheep.yy"
namespace Sheep {
#line 167 "sheep.tab.cc"


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
        std::string yyr;
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
              else
                goto append;

            append:
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
#if YYDEBUG
    : yydebug_ (false),
      yycdebug_ (&std::cerr),
#else
    :
#endif
      scanner (scanner_yyarg),
      compiler (compiler_yyarg),
      builder (builder_yyarg)
  {}

  Parser::~Parser ()
  {}

  Parser::syntax_error::~syntax_error () YY_NOEXCEPT YY_NOTHROW
  {}

  /*---------------.
  | Symbol types.  |
  `---------------*/



  // by_state.
  Parser::by_state::by_state () YY_NOEXCEPT
    : state (empty_state)
  {}

  Parser::by_state::by_state (const by_state& that) YY_NOEXCEPT
    : state (that.state)
  {}

  void
  Parser::by_state::clear () YY_NOEXCEPT
  {
    state = empty_state;
  }

  void
  Parser::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

  Parser::by_state::by_state (state_type s) YY_NOEXCEPT
    : state (s)
  {}

  Parser::symbol_number_type
  Parser::by_state::type_get () const YY_NOEXCEPT
  {
    if (state == empty_state)
      return empty_symbol;
    else
      return yystos_[+state];
  }

  Parser::stack_symbol_type::stack_symbol_type ()
  {}

  Parser::stack_symbol_type::stack_symbol_type (YY_RVREF (stack_symbol_type) that)
    : super_type (YY_MOVE (that.state), YY_MOVE (that.location))
  {
    switch (that.type_get ())
    {
      case 67: // expr
      case 68: // sysfunc_call
        value.YY_MOVE_OR_COPY< SheepValue > (YY_MOVE (that.value));
        break;

      case 27: // FLOAT
        value.YY_MOVE_OR_COPY< float > (YY_MOVE (that.value));
        break;

      case 26: // INT
        value.YY_MOVE_OR_COPY< int > (YY_MOVE (that.value));
        break;

      case 28: // STRING
      case 29: // USERID
      case 30: // SYSID
        value.YY_MOVE_OR_COPY< std::string > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

#if 201103L <= YY_CPLUSPLUS
    // that is emptied.
    that.state = empty_state;
#endif
  }

  Parser::stack_symbol_type::stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) that)
    : super_type (s, YY_MOVE (that.location))
  {
    switch (that.type_get ())
    {
      case 67: // expr
      case 68: // sysfunc_call
        value.move< SheepValue > (YY_MOVE (that.value));
        break;

      case 27: // FLOAT
        value.move< float > (YY_MOVE (that.value));
        break;

      case 26: // INT
        value.move< int > (YY_MOVE (that.value));
        break;

      case 28: // STRING
      case 29: // USERID
      case 30: // SYSID
        value.move< std::string > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

    // that is emptied.
    that.type = empty_symbol;
  }

#if YY_CPLUSPLUS < 201103L
  Parser::stack_symbol_type&
  Parser::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
    switch (that.type_get ())
    {
      case 67: // expr
      case 68: // sysfunc_call
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

  Parser::stack_symbol_type&
  Parser::stack_symbol_type::operator= (stack_symbol_type& that)
  {
    state = that.state;
    switch (that.type_get ())
    {
      case 67: // expr
      case 68: // sysfunc_call
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

    location = that.location;
    // that is emptied.
    that.state = empty_state;
    return *this;
  }
#endif

  template <typename Base>
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
#if defined __GNUC__ && ! defined __clang__ && ! defined __ICC && __GNUC__ * 100 + __GNUC_MINOR__ <= 408
    // Avoid a (spurious) G++ 4.8 warning about "array subscript is
    // below array bounds".
    if (yysym.empty ())
      std::abort ();
#endif
    yyo << (yytype < yyntokens_ ? "token" : "nterm")
        << ' ' << yytname_[yytype] << " ("
        << yysym.location << ": ";
    YYUSE (yytype);
    yyo << ')';
  }
#endif

  void
  Parser::yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym)
  {
    if (m)
      YY_SYMBOL_PRINT (m, sym);
    yystack_.push (YY_MOVE (sym));
  }

  void
  Parser::yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym)
  {
#if 201103L <= YY_CPLUSPLUS
    yypush_ (m, stack_symbol_type (s, std::move (sym)));
#else
    stack_symbol_type ss (s, sym);
    yypush_ (m, ss);
#endif
  }

  void
  Parser::yypop_ (int n)
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

  Parser::state_type
  Parser::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - yyntokens_] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - yyntokens_];
  }

  bool
  Parser::yy_pact_value_is_default_ (int yyvalue)
  {
    return yyvalue == yypact_ninf_;
  }

  bool
  Parser::yy_table_value_is_error_ (int yyvalue)
  {
    return yyvalue == yytable_ninf_;
  }

  int
  Parser::operator() ()
  {
    return parse ();
  }

  int
  Parser::parse ()
  {
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

#if YY_EXCEPTIONS
    try
#endif // YY_EXCEPTIONS
      {
    YYCDEBUG << "Starting parse\n";


    /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystack_.clear ();
    yypush_ (YY_NULLPTR, 0, YY_MOVE (yyla));

  /*-----------------------------------------------.
  | yynewstate -- push a new symbol on the stack.  |
  `-----------------------------------------------*/
  yynewstate:
    YYCDEBUG << "Entering state " << int (yystack_[0].state) << '\n';

    // Accept?
    if (yystack_[0].state == yyfinal_)
      YYACCEPT;

    goto yybackup;


  /*-----------.
  | yybackup.  |
  `-----------*/
  yybackup:
    // Try to take a decision without lookahead.
    yyn = yypact_[+yystack_[0].state];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    // Read a lookahead token.
    if (yyla.empty ())
      {
        YYCDEBUG << "Reading a token: ";
#if YY_EXCEPTIONS
        try
#endif // YY_EXCEPTIONS
          {
            symbol_type yylookahead (yylex (scanner, compiler, builder));
            yyla.move (yylookahead);
          }
#if YY_EXCEPTIONS
        catch (const syntax_error& yyexc)
          {
            YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
            error (yyexc);
            goto yyerrlab1;
          }
#endif // YY_EXCEPTIONS
      }
    YY_SYMBOL_PRINT ("Next token is", yyla);

    /* If the proper action on seeing token YYLA.TYPE is to reduce or
       to detect an error, take that action.  */
    yyn += yyla.type_get ();
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.type_get ())
      {
        goto yydefault;
      }

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
    yypush_ ("Shifting", state_type (yyn), YY_MOVE (yyla));
    goto yynewstate;


  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[+yystack_[0].state];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;


  /*-----------------------------.
  | yyreduce -- do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    {
      stack_symbol_type yylhs;
      yylhs.state = yy_lr_goto_state_ (yystack_[yylen].state, yyr1_[yyn]);
      /* Variants are always initialized to an empty instance of the
         correct type. The default '$$ = $1' action is NOT applied
         when using variants.  */
      switch (yyr1_[yyn])
    {
      case 67: // expr
      case 68: // sysfunc_call
        yylhs.value.emplace< SheepValue > ();
        break;

      case 27: // FLOAT
        yylhs.value.emplace< float > ();
        break;

      case 26: // INT
        yylhs.value.emplace< int > ();
        break;

      case 28: // STRING
      case 29: // USERID
      case 30: // SYSID
        yylhs.value.emplace< std::string > ();
        break;

      default:
        break;
    }


      // Default location.
      {
        stack_type::slice range (yystack_, yylen);
        YYLLOC_DEFAULT (yylhs.location, range, yylen);
        yyerror_range[1].location = yylhs.location;
      }

      // Perform the reduction.
      YY_REDUCE_PRINT (yyn);
#if YY_EXCEPTIONS
      try
#endif // YY_EXCEPTIONS
        {
          switch (yyn)
            {
  case 2:
#line 160 "sheep.yy"
                                                                        { }
#line 691 "sheep.tab.cc"
    break;

  case 3:
#line 161 "sheep.yy"
                                                                        { }
#line 697 "sheep.tab.cc"
    break;

  case 4:
#line 162 "sheep.yy"
                                                                        { }
#line 703 "sheep.tab.cc"
    break;

  case 5:
#line 163 "sheep.yy"
                                                        { }
#line 709 "sheep.tab.cc"
    break;

  case 6:
#line 164 "sheep.yy"
                                                { }
#line 715 "sheep.tab.cc"
    break;

  case 7:
#line 169 "sheep.yy"
                                     { builder.BeginSymbols(); }
#line 721 "sheep.tab.cc"
    break;

  case 8:
#line 169 "sheep.yy"
                                                                                           { }
#line 727 "sheep.tab.cc"
    break;

  case 9:
#line 172 "sheep.yy"
                                                {  }
#line 733 "sheep.tab.cc"
    break;

  case 10:
#line 173 "sheep.yy"
                                        {  }
#line 739 "sheep.tab.cc"
    break;

  case 11:
#line 179 "sheep.yy"
                                                        { }
#line 745 "sheep.tab.cc"
    break;

  case 12:
#line 180 "sheep.yy"
                                                                { }
#line 751 "sheep.tab.cc"
    break;

  case 13:
#line 181 "sheep.yy"
                                                                { }
#line 757 "sheep.tab.cc"
    break;

  case 14:
#line 184 "sheep.yy"
                                                                                { builder.AddIntVariable(yystack_[0].value.as < std::string > (), 0); }
#line 763 "sheep.tab.cc"
    break;

  case 15:
#line 185 "sheep.yy"
                                                                                { builder.AddIntVariable(yystack_[2].value.as < std::string > (), yystack_[0].value.as < int > ()); }
#line 769 "sheep.tab.cc"
    break;

  case 16:
#line 186 "sheep.yy"
                                                                        { builder.AddIntVariable(yystack_[0].value.as < std::string > (), 0); }
#line 775 "sheep.tab.cc"
    break;

  case 17:
#line 187 "sheep.yy"
                                                                { builder.AddIntVariable(yystack_[2].value.as < std::string > (), yystack_[0].value.as < int > ()); }
#line 781 "sheep.tab.cc"
    break;

  case 18:
#line 190 "sheep.yy"
                                                                                { builder.AddFloatVariable(yystack_[0].value.as < std::string > (), 0.0f); }
#line 787 "sheep.tab.cc"
    break;

  case 19:
#line 191 "sheep.yy"
                                                                                { builder.AddFloatVariable(yystack_[2].value.as < std::string > (), yystack_[0].value.as < float > ()); }
#line 793 "sheep.tab.cc"
    break;

  case 20:
#line 192 "sheep.yy"
                                                                { builder.AddFloatVariable(yystack_[0].value.as < std::string > (), 0.0f); }
#line 799 "sheep.tab.cc"
    break;

  case 21:
#line 193 "sheep.yy"
                                                        { builder.AddFloatVariable(yystack_[2].value.as < std::string > (), yystack_[0].value.as < float > ()); }
#line 805 "sheep.tab.cc"
    break;

  case 22:
#line 196 "sheep.yy"
                                                                                { builder.AddStringVariable(yystack_[0].value.as < std::string > (), ""); }
#line 811 "sheep.tab.cc"
    break;

  case 23:
#line 197 "sheep.yy"
                                                                                { builder.AddStringVariable(yystack_[2].value.as < std::string > (), yystack_[0].value.as < std::string > ()); }
#line 817 "sheep.tab.cc"
    break;

  case 24:
#line 198 "sheep.yy"
                                                                        { builder.AddStringVariable(yystack_[0].value.as < std::string > (), ""); }
#line 823 "sheep.tab.cc"
    break;

  case 25:
#line 199 "sheep.yy"
                                                        { builder.AddStringVariable(yystack_[2].value.as < std::string > (), yystack_[0].value.as < std::string > ()); }
#line 829 "sheep.tab.cc"
    break;

  case 26:
#line 205 "sheep.yy"
                               { builder.BeginCode(); }
#line 835 "sheep.tab.cc"
    break;

  case 27:
#line 205 "sheep.yy"
                                                                               { }
#line 841 "sheep.tab.cc"
    break;

  case 28:
#line 209 "sheep.yy"
                                        { }
#line 847 "sheep.tab.cc"
    break;

  case 29:
#line 210 "sheep.yy"
                                { }
#line 853 "sheep.tab.cc"
    break;

  case 30:
#line 215 "sheep.yy"
                                      { builder.StartFunction(yystack_[2].value.as < std::string > ()); }
#line 859 "sheep.tab.cc"
    break;

  case 31:
#line 215 "sheep.yy"
                                                                                                         { builder.EndFunction(yystack_[6].value.as < std::string > ()); }
#line 865 "sheep.tab.cc"
    break;

  case 34:
#line 224 "sheep.yy"
                                                                        { builder.Store(yystack_[3].value.as < std::string > (), yylhs.location); }
#line 871 "sheep.tab.cc"
    break;

  case 35:
#line 225 "sheep.yy"
                                                                                                        { }
#line 877 "sheep.tab.cc"
    break;

  case 36:
#line 226 "sheep.yy"
                                                                                                { builder.ReturnV(); }
#line 883 "sheep.tab.cc"
    break;

  case 37:
#line 227 "sheep.yy"
                                                                                        { builder.Breakpoint(); }
#line 889 "sheep.tab.cc"
    break;

  case 38:
#line 228 "sheep.yy"
                                                                                        { builder.SitnSpin(); }
#line 895 "sheep.tab.cc"
    break;

  case 39:
#line 229 "sheep.yy"
                                                                                        { builder.BranchGoto(yystack_[1].value.as < std::string > ()); }
#line 901 "sheep.tab.cc"
    break;

  case 40:
#line 230 "sheep.yy"
                                                                                                { builder.AddGoto(yystack_[1].value.as < std::string > (), yylhs.location); }
#line 907 "sheep.tab.cc"
    break;

  case 41:
#line 231 "sheep.yy"
                                                                                                { builder.BeginWait(); builder.EndWait(); }
#line 913 "sheep.tab.cc"
    break;

  case 42:
#line 232 "sheep.yy"
               { builder.BeginWait(); }
#line 919 "sheep.tab.cc"
    break;

  case 43:
#line 232 "sheep.yy"
                                                                { builder.EndWait(); }
#line 925 "sheep.tab.cc"
    break;

  case 44:
#line 233 "sheep.yy"
               { builder.BeginWait(); }
#line 931 "sheep.tab.cc"
    break;

  case 45:
#line 233 "sheep.yy"
                                                                            { builder.EndWait(); }
#line 937 "sheep.tab.cc"
    break;

  case 47:
#line 235 "sheep.yy"
                                                                                                { }
#line 943 "sheep.tab.cc"
    break;

  case 48:
#line 236 "sheep.yy"
          { builder.BeginIfElseBlock(); }
#line 949 "sheep.tab.cc"
    break;

  case 49:
#line 236 "sheep.yy"
                                                                { builder.EndIfElseBlock(); }
#line 955 "sheep.tab.cc"
    break;

  case 51:
#line 244 "sheep.yy"
                                                                { yylhs.value.as < SheepValue > () = yystack_[0].value.as < SheepValue > (); }
#line 961 "sheep.tab.cc"
    break;

  case 52:
#line 245 "sheep.yy"
                                                                        { auto type = builder.Load(yystack_[0].value.as < std::string > (), yylhs.location); yylhs.value.as < SheepValue > () = SheepValue(type); }
#line 967 "sheep.tab.cc"
    break;

  case 53:
#line 246 "sheep.yy"
                                                                        { builder.PushI(yystack_[0].value.as < int > ()); yylhs.value.as < SheepValue > () = SheepValue(yystack_[0].value.as < int > ()); }
#line 973 "sheep.tab.cc"
    break;

  case 54:
#line 247 "sheep.yy"
                                                                        { builder.PushF(yystack_[0].value.as < float > ()); yylhs.value.as < SheepValue > () = SheepValue(yystack_[0].value.as < float > ()); }
#line 979 "sheep.tab.cc"
    break;

  case 55:
#line 248 "sheep.yy"
                                                                        { builder.AddStringConst(yystack_[0].value.as < std::string > ()); builder.PushS(yystack_[0].value.as < std::string > ()); yylhs.value.as < SheepValue > () = SheepValue(""); }
#line 985 "sheep.tab.cc"
    break;

  case 56:
#line 250 "sheep.yy"
                                                                { auto type = builder.Add(yystack_[2].value.as < SheepValue > (), yystack_[0].value.as < SheepValue > (), yylhs.location); yylhs.value.as < SheepValue > () = SheepValue(type); }
#line 991 "sheep.tab.cc"
    break;

  case 57:
#line 251 "sheep.yy"
                                                                { auto type = builder.Subtract(yystack_[2].value.as < SheepValue > (), yystack_[0].value.as < SheepValue > (), yylhs.location); yylhs.value.as < SheepValue > () = SheepValue(type); }
#line 997 "sheep.tab.cc"
    break;

  case 58:
#line 252 "sheep.yy"
                                                        { auto type = builder.Multiply(yystack_[2].value.as < SheepValue > (), yystack_[0].value.as < SheepValue > (), yylhs.location); yylhs.value.as < SheepValue > () = SheepValue(type); }
#line 1003 "sheep.tab.cc"
    break;

  case 59:
#line 253 "sheep.yy"
                                                                { auto type = builder.Divide(yystack_[2].value.as < SheepValue > (), yystack_[0].value.as < SheepValue > (), yylhs.location); yylhs.value.as < SheepValue > () = SheepValue(type); }
#line 1009 "sheep.tab.cc"
    break;

  case 60:
#line 254 "sheep.yy"
                                                                { builder.Modulo(yystack_[2].value.as < SheepValue > (), yystack_[0].value.as < SheepValue > (), yylhs.location); yylhs.value.as < SheepValue > () = SheepValue(SheepValueType::Int); }
#line 1015 "sheep.tab.cc"
    break;

  case 61:
#line 255 "sheep.yy"
                                                                { builder.Negate(yystack_[0].value.as < SheepValue > (), yylhs.location); }
#line 1021 "sheep.tab.cc"
    break;

  case 62:
#line 257 "sheep.yy"
                                                                { auto type = builder.IsLess(yystack_[2].value.as < SheepValue > (), yystack_[0].value.as < SheepValue > (), yylhs.location); yylhs.value.as < SheepValue > () = SheepValue(type); }
#line 1027 "sheep.tab.cc"
    break;

  case 63:
#line 258 "sheep.yy"
                                                                { auto type = builder.IsGreater(yystack_[2].value.as < SheepValue > (), yystack_[0].value.as < SheepValue > (), yylhs.location); yylhs.value.as < SheepValue > () = SheepValue(type); }
#line 1033 "sheep.tab.cc"
    break;

  case 64:
#line 259 "sheep.yy"
                                                                { auto type = builder.IsLessEqual(yystack_[2].value.as < SheepValue > (), yystack_[0].value.as < SheepValue > (), yylhs.location); yylhs.value.as < SheepValue > () = SheepValue(type); }
#line 1039 "sheep.tab.cc"
    break;

  case 65:
#line 260 "sheep.yy"
                                                                { auto type = builder.IsGreaterEqual(yystack_[2].value.as < SheepValue > (), yystack_[0].value.as < SheepValue > (), yylhs.location); yylhs.value.as < SheepValue > () = SheepValue(type); }
#line 1045 "sheep.tab.cc"
    break;

  case 66:
#line 261 "sheep.yy"
                                                                { auto type = builder.IsEqual(yystack_[2].value.as < SheepValue > (), yystack_[0].value.as < SheepValue > (), yylhs.location); yylhs.value.as < SheepValue > () = SheepValue(type); }
#line 1051 "sheep.tab.cc"
    break;

  case 67:
#line 262 "sheep.yy"
                                                        { auto type = builder.IsNotEqual(yystack_[2].value.as < SheepValue > (), yystack_[0].value.as < SheepValue > (), yylhs.location); yylhs.value.as < SheepValue > () = SheepValue(type); }
#line 1057 "sheep.tab.cc"
    break;

  case 68:
#line 263 "sheep.yy"
                                                                { builder.Or(yystack_[2].value.as < SheepValue > (), yystack_[0].value.as < SheepValue > (), yylhs.location); yylhs.value.as < SheepValue > () = SheepValue(SheepValueType::Int); }
#line 1063 "sheep.tab.cc"
    break;

  case 69:
#line 264 "sheep.yy"
                                                                { builder.And(yystack_[2].value.as < SheepValue > (), yystack_[0].value.as < SheepValue > (), yylhs.location); yylhs.value.as < SheepValue > () = SheepValue(SheepValueType::Int); }
#line 1069 "sheep.tab.cc"
    break;

  case 70:
#line 265 "sheep.yy"
                                                                        { builder.Not(); yylhs.value.as < SheepValue > () = yystack_[0].value.as < SheepValue > (); }
#line 1075 "sheep.tab.cc"
    break;

  case 71:
#line 267 "sheep.yy"
                                                { yylhs.value.as < SheepValue > () = yystack_[1].value.as < SheepValue > (); }
#line 1081 "sheep.tab.cc"
    break;

  case 72:
#line 272 "sheep.yy"
                                                           { auto type = builder.CallSysFunc(yystack_[3].value.as < std::string > (), yylhs.location); yylhs.value.as < SheepValue > () = SheepValue(type); }
#line 1087 "sheep.tab.cc"
    break;

  case 73:
#line 276 "sheep.yy"
                                                        { }
#line 1093 "sheep.tab.cc"
    break;

  case 74:
#line 277 "sheep.yy"
                                                                        { builder.AddSysFuncArg(yystack_[0].value.as < SheepValue > (), yylhs.location); }
#line 1099 "sheep.tab.cc"
    break;

  case 75:
#line 278 "sheep.yy"
                                                { builder.AddSysFuncArg(yystack_[0].value.as < SheepValue > (), yylhs.location); }
#line 1105 "sheep.tab.cc"
    break;

  case 78:
#line 285 "sheep.yy"
                                           { builder.BeginIfBlock(); }
#line 1111 "sheep.tab.cc"
    break;

  case 79:
#line 285 "sheep.yy"
                                                                                        { builder.EndIfBlock(); }
#line 1117 "sheep.tab.cc"
    break;

  case 80:
#line 288 "sheep.yy"
                     { builder.BeginElseBlock(); }
#line 1123 "sheep.tab.cc"
    break;

  case 81:
#line 288 "sheep.yy"
                                                                    { builder.EndElseBlock(); }
#line 1129 "sheep.tab.cc"
    break;

  case 82:
#line 289 "sheep.yy"
                                                        {  }
#line 1135 "sheep.tab.cc"
    break;


#line 1139 "sheep.tab.cc"

            default:
              break;
            }
        }
#if YY_EXCEPTIONS
      catch (const syntax_error& yyexc)
        {
          YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
          error (yyexc);
          YYERROR;
        }
#endif // YY_EXCEPTIONS
      YY_SYMBOL_PRINT ("-> $$ =", yylhs);
      yypop_ (yylen);
      yylen = 0;
      YY_STACK_PRINT ();

      // Shift the result of the reduction.
      yypush_ (YY_NULLPTR, YY_MOVE (yylhs));
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
    /* Pacify compilers when the user code never invokes YYERROR and
       the label yyerrorlab therefore never appears in user code.  */
    if ((false))
      YYERROR;

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
          yyn = yypact_[+yystack_[0].state];
          if (!yy_pact_value_is_default_ (yyn))
            {
              yyn += yy_error_token_;
              if (0 <= yyn && yyn <= yylast_ && yycheck_[yyn] == yy_error_token_)
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
      error_token.state = state_type (yyn);
      yypush_ ("Shifting", YY_MOVE (error_token));
    }
    goto yynewstate;


  /*-------------------------------------.
  | yyacceptlab -- YYACCEPT comes here.  |
  `-------------------------------------*/
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;


  /*-----------------------------------.
  | yyabortlab -- YYABORT comes here.  |
  `-----------------------------------*/
  yyabortlab:
    yyresult = 1;
    goto yyreturn;


  /*-----------------------------------------------------.
  | yyreturn -- parsing is finished, return the result.  |
  `-----------------------------------------------------*/
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
#if YY_EXCEPTIONS
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack\n";
        // Do not try to display the values of the reclaimed symbols,
        // as their printers might throw an exception.
        if (!yyla.empty ())
          yy_destroy_ (YY_NULLPTR, yyla);

        while (1 < yystack_.size ())
          {
            yy_destroy_ (YY_NULLPTR, yystack_[0]);
            yypop_ ();
          }
        throw;
      }
#endif // YY_EXCEPTIONS
  }

  void
  Parser::error (const syntax_error& yyexc)
  {
    error (yyexc.location, yyexc.what ());
  }

  // Generate an error message.
  std::string
  Parser::yysyntax_error_ (state_type yystate, const symbol_type& yyla) const
  {
    // Number of reported tokens (one for the "unexpected", one per
    // "expected").
    std::ptrdiff_t yycount = 0;
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
         scanner and before detecting a syntax error.  Thus, state merging
         (from LALR or IELR) and default reductions corrupt the expected
         token list.  However, the list is correct for canonical LR with
         one exception: it will still contain any token that will not be
         accepted due to an error action in a later state.
    */
    if (!yyla.empty ())
      {
        symbol_number_type yytoken = yyla.type_get ();
        yyarg[yycount++] = yytname_[yytoken];

        int yyn = yypact_[+yystate];
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
              if (yycheck_[yyx + yyn] == yyx && yyx != yy_error_token_
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
      default: // Avoid compiler warnings.
        YYCASE_ (0, YY_("syntax error"));
        YYCASE_ (1, YY_("syntax error, unexpected %s"));
        YYCASE_ (2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_ (3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_ (4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_ (5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    std::string yyres;
    // Argument number.
    std::ptrdiff_t yyi = 0;
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


  const signed char Parser::yypact_ninf_ = -112;

  const signed char Parser::yytable_ninf_ = -45;

  const short
  Parser::yypact_[] =
  {
       4,    -8,     7,  -112,    10,    24,  -112,  -112,  -112,     8,
    -112,  -112,  -112,  -112,    -1,    20,  -112,    57,  -112,  -112,
       0,    41,    44,  -112,  -112,  -112,    -6,    46,    57,    57,
    -112,    54,  -112,   176,  -112,   -12,    65,    55,  -112,  -112,
     150,    49,  -112,    35,    67,  -112,  -112,  -112,    57,    57,
    -112,  -112,    70,  -112,    85,    57,    57,    57,    57,    57,
      57,    57,    57,    57,    57,    57,    57,    57,  -112,    80,
    -112,  -112,    77,    84,    95,  -112,  -112,  -112,  -112,  -112,
    -112,   118,   176,    -4,    57,    54,  -112,   188,   199,   130,
     130,   137,   137,   137,   137,    30,    30,  -112,  -112,  -112,
      96,    86,    87,    94,    88,    97,   101,    93,  -112,    57,
    -112,   163,  -112,   102,  -112,   106,   107,  -112,   108,   113,
    -112,   115,   146,  -112,  -112,   176,  -112,  -112,   230,  -112,
     214,  -112,   215,  -112,   216,   102,  -112,   222,   117,   221,
    -112,   119,  -112,  -112,  -112,  -112
  };

  const signed char
  Parser::yydefact_[] =
  {
       2,     0,     0,    32,     0,     3,     4,    26,     7,    48,
       1,     5,    28,     9,     0,     0,    47,     0,    32,     6,
      42,     0,     0,    53,    54,    55,    52,     0,     0,     0,
      33,     0,    46,    35,    51,     0,     0,     0,    36,    52,
       0,    48,    41,     0,     0,    37,    38,    40,     0,    73,
      70,    61,     0,    49,    76,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    27,     0,
      29,     8,     0,     0,     0,    10,    39,    71,    50,    43,
      32,     0,    74,     0,     0,    80,    77,    68,    69,    66,
      67,    62,    64,    63,    65,    56,    57,    58,    59,    60,
       0,    14,     0,    18,     0,    22,     0,    48,    34,     0,
      72,     0,    82,     0,    30,     0,     0,    11,     0,     0,
      12,     0,     0,    13,    45,    75,    78,    81,     0,    15,
      16,    19,    20,    23,    24,     0,    32,     0,     0,     0,
      79,    48,    17,    21,    25,    31
  };

  const short
  Parser::yypgoto_[] =
  {
    -112,  -112,  -112,  -112,  -112,  -112,  -112,  -112,  -112,   245,
    -112,  -112,  -112,  -112,   -18,  -112,  -112,  -112,  -112,  -111,
     -16,   208,  -112,   167,  -112,  -112,  -112,  -112
  };

  const short
  Parser::yydefgoto_[] =
  {
      -1,     4,     5,    13,    36,    75,   102,   104,   106,     6,
      12,    35,    70,   128,     9,    30,    43,    44,    31,    32,
      33,    34,    83,    53,    54,   135,    86,   113
  };

  const short
  Parser::yytable_[] =
  {
      41,    40,   127,    68,    47,   109,     7,     1,     2,   110,
      10,    42,    50,    51,   -44,    14,    15,    69,     3,    16,
      17,     8,    18,    19,   140,    48,    20,     1,    37,    21,
      22,    38,    81,    82,    23,    24,    25,    26,    27,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,    45,    28,    29,    46,    14,    15,    49,    52,
      16,    17,   107,    18,    78,    27,    76,    20,   111,    17,
      21,    22,    65,    66,    67,    23,    24,    25,    26,    27,
      71,    80,    84,    23,    24,    25,    39,    27,    72,    73,
      74,    85,   100,   125,    28,    29,   116,   119,   117,   120,
      14,    15,    28,    29,    16,    17,   101,    18,   124,   114,
     122,    20,   123,   103,    21,    22,    18,   115,   141,    23,
      24,    25,    26,    27,   105,   118,    14,    15,   121,   108,
      16,    17,   129,    18,   145,   131,   130,    20,    28,    29,
      21,    22,   132,   133,   143,    23,    24,    25,    26,    27,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    77,    28,    29,    59,    60,    61,    62,
      63,    64,    65,    66,    67,   134,   126,    63,    64,    65,
      66,    67,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,   136,   137,   138,   139,   142,   144,
      11,    79,   112
  };

  const unsigned char
  Parser::yycheck_[] =
  {
      18,    17,   113,    15,    10,     9,    14,     3,     4,    13,
       0,    11,    28,    29,    14,     7,     8,    29,    14,    11,
      12,    14,    14,    15,   135,    31,    18,     3,    29,    21,
      22,    11,    48,    49,    26,    27,    28,    29,    30,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    11,    45,    46,    11,     7,     8,    12,     5,
      11,    12,    80,    14,    15,    30,    11,    18,    84,    12,
      21,    22,    42,    43,    44,    26,    27,    28,    29,    30,
      15,    14,    12,    26,    27,    28,    29,    30,    23,    24,
      25,     6,    12,   109,    45,    46,     9,     9,    11,    11,
       7,     8,    45,    46,    11,    12,    29,    14,    15,    13,
       9,    18,    11,    29,    21,    22,    14,    31,   136,    26,
      27,    28,    29,    30,    29,    31,     7,     8,    31,    11,
      11,    12,    26,    14,    15,    27,    29,    18,    45,    46,
      21,    22,    29,    28,    27,    26,    27,    28,    29,    30,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    13,    45,    46,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    29,    13,    40,    41,    42,
      43,    44,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    14,    31,    31,    31,    26,    28,
       5,    43,    85
  };

  const signed char
  Parser::yystos_[] =
  {
       0,     3,     4,    14,    48,    49,    56,    14,    14,    61,
       0,    56,    57,    50,     7,     8,    11,    12,    14,    15,
      18,    21,    22,    26,    27,    28,    29,    30,    45,    46,
      62,    65,    66,    67,    68,    58,    51,    29,    11,    29,
      67,    61,    11,    63,    64,    11,    11,    10,    31,    12,
      67,    67,     5,    70,    71,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    15,    29,
      59,    15,    23,    24,    25,    52,    11,    13,    15,    68,
      14,    67,    67,    69,    12,     6,    73,    67,    67,    67,
      67,    67,    67,    67,    67,    67,    67,    67,    67,    67,
      12,    29,    53,    29,    54,    29,    55,    61,    11,     9,
      13,    67,    70,    74,    13,    31,     9,    11,    31,     9,
      11,    31,     9,    11,    15,    67,    13,    66,    60,    26,
      29,    27,    29,    28,    29,    72,    14,    31,    31,    31,
      66,    61,    26,    27,    28,    15
  };

  const signed char
  Parser::yyr1_[] =
  {
       0,    47,    48,    48,    48,    48,    48,    50,    49,    51,
      51,    52,    52,    52,    53,    53,    53,    53,    54,    54,
      54,    54,    55,    55,    55,    55,    57,    56,    58,    58,
      60,    59,    61,    61,    62,    62,    62,    62,    62,    62,
      62,    62,    63,    62,    64,    62,    62,    62,    65,    62,
      66,    67,    67,    67,    67,    67,    67,    67,    67,    67,
      67,    67,    67,    67,    67,    67,    67,    67,    67,    67,
      67,    67,    68,    69,    69,    69,    70,    70,    72,    71,
      74,    73,    73
  };

  const signed char
  Parser::yyr2_[] =
  {
       0,     2,     0,     1,     1,     2,     3,     0,     5,     0,
       2,     3,     3,     3,     1,     3,     3,     5,     1,     3,
       3,     5,     1,     3,     3,     5,     0,     5,     0,     2,
       0,     7,     0,     2,     4,     1,     2,     2,     2,     3,
       2,     2,     0,     3,     0,     5,     1,     1,     0,     2,
       3,     1,     1,     1,     1,     1,     3,     3,     3,     3,
       3,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       2,     3,     4,     0,     1,     3,     1,     2,     0,     6,
       0,     3,     2
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
  "$@1", "symbol_decls", "symbol_decl", "symbol_decl_int",
  "symbol_decl_float", "symbol_decl_string", "code_section", "$@2",
  "functions", "function", "$@3", "statements", "statement", "$@4", "$@5",
  "$@6", "statements_block", "expr", "sysfunc_call", "sysfunc_call_args",
  "if_else_block", "if_statement", "$@7", "else_statement", "$@8", YY_NULLPTR
  };

#if YYDEBUG
  const short
  Parser::yyrline_[] =
  {
       0,   160,   160,   161,   162,   163,   164,   169,   169,   172,
     173,   179,   180,   181,   184,   185,   186,   187,   190,   191,
     192,   193,   196,   197,   198,   199,   205,   205,   209,   210,
     215,   215,   219,   220,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   232,   233,   233,   234,   235,   236,   236,
     240,   244,   245,   246,   247,   248,   250,   251,   252,   253,
     254,   255,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   267,   272,   276,   277,   278,   281,   282,   285,   285,
     288,   288,   289
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
      *yycdebug_ << ' ' << int (i->state);
    *yycdebug_ << '\n';
  }

  // Report on the debug stream that the rule \a yyrule is going to be reduced.
  void
  Parser::yy_reduce_print_ (int yyrule)
  {
    int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    // Print the symbols being reduced, and their result.
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
               << " (line " << yylno << "):\n";
    // The symbols being reduced.
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
                       yystack_[(yynrhs) - (yyi + 1)]);
  }
#endif // YYDEBUG


#line 20 "sheep.yy"
} // Sheep
#line 1648 "sheep.tab.cc"

#line 292 "sheep.yy"


/* No epilogue is needed. */

