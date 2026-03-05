%{
%}

%option noyywrap

%%

[^|<>\n \t\v\f]+  { return(STRING); }
"|"               { return(PIPE); }
"<"               { return(REDIRECT_IN); }
">"               { return(REDIRECT_OUT); }
[\n]              { return(END_OF_LINE); }
[ \t\v\f]         { /* ignore whitespace */ }

%%
