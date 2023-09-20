%option noyywrap
%{
/*****************声明和选项设置  begin*****************/
#include <stdio.h>
#include <stdlib.h>

#include "syntax_tree.h"
#include "syntax_analyzer.h"

int lines = 1;
int pos_start;
int pos_end = 1;

void pass_node(char *text){
     yylval.node = new_syntax_tree_node(text);
}

/*****************声明和选项设置  end*****************/

%}


%%
 /* to do for students */
 /* two cases for you, pass_node will send flex's token to bison */
[/][*][^*]*[*]+([^*/][^*]*[*]+)*[/] {pos_start = pos_end; pos_end += strlen(yytext);}

"+" 	{pos_start = pos_end; pos_end += 1; pass_node(yytext); return ADD;}
"-"   {pos_start = pos_end; pos_end += 1; pass_node(yytext); return MINUS;}
"*"  {pos_start = pos_end; pos_end += 1; pass_node(yytext); return MUL;}
"/"   {pos_start = pos_end; pos_end += 1; pass_node(yytext); return DIV;}
"="   {pos_start = pos_end; pos_end += 1; pass_node(yytext); return ASS;}

"<="   {pos_start = pos_end; pos_end += 1; pass_node(yytext); return EQLESS;}
"<"   {pos_start = pos_end; pos_end += 1; pass_node(yytext); return LESS;}
">"   {pos_start = pos_end; pos_end += 1; pass_node(yytext); return MORE;}
">="   {pos_start = pos_end; pos_end += 1; pass_node(yytext); return EQMORE;}
"="   {pos_start = pos_end; pos_end += 1; pass_node(yytext); return EQ;}
"!="   {pos_start = pos_end; pos_end += 1; pass_node(yytext); return NEEQ;}

"("   {pos_start = pos_end; pos_end += 1; pass_node(yytext); return LP;}
")"   {pos_start = pos_end; pos_end += 1; pass_node(yytext); return RP;}
"["   {pos_start = pos_end; pos_end += 1; pass_node(yytext); return LB;}
"]"   {pos_start = pos_end; pos_end += 1; pass_node(yytext); return RB;}
"{"   {pos_start = pos_end; pos_end += 1; pass_node(yytext); return LC;}
"}"   {pos_start = pos_end; pos_end += 1; pass_node(yytext); return RC;}
","   {pos_start = pos_end; pos_end += 1; pass_node(yytext); return COMMA;}
";"   {pos_start = pos_end; pos_end += 1; pass_node(yytext); return SEM;}
" " {pos_start = pos_end; pos_end += 1;}
\n {pos_start = pos_end; pos_end += 1;lines++;}
\t {pos_start = pos_end; pos_end += 1;}
\r {pos_start = pos_end; pos_end += 1;}

"if"   {pos_start = pos_end; pos_end += strlen(yytext); pass_node(yytext); return IF;}
"while"   {pos_start = pos_end; pos_end += strlen(yytext); pass_node(yytext); return WHILE;}
"return"   {pos_start = pos_end; pos_end += strlen(yytext); pass_node(yytext); return RETURN;}
"else"   {pos_start = pos_end; pos_end += strlen(yytext); pass_node(yytext); return ELSE;}
"int"   {pos_start = pos_end; pos_end += strlen(yytext); pass_node(yytext); return INT;}
"float"   {pos_start = pos_end; pos_end += strlen(yytext); pass_node(yytext); return FLOAT;}
"void"   {pos_start = pos_end; pos_end += strlen(yytext); pass_node(yytext); return VOID;}

([0-9])+ {pos_start = pos_end; pos_end += strlen(yytext); pass_node(yytext); return INTEGER;}
([0-9]*\.[0-9]+)|([0-9]+\.) {pos_start = pos_end; pos_end += strlen(yytext); pass_node(yytext); return FLOATPOINT;}
[a-zA-Z]+ {pos_start = pos_end; pos_end += strlen(yytext); pass_node(yytext); return ID;}

. { pos_start = pos_end; pos_end++; return ERROR; }

 /****请在此补全所有flex的模式与动作  end******/
%%
