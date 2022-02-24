#  pl0c - PL/0 compiler.

```
program = block "." .
block   = [ "const" ident "=" number { "," ident "=" number } ";" ]
          [ "var" ident { "," ident } ";" ]
          { "procedure" ident ";" block ";" } statement .
statement = [ ident ":=" expression
          | "call" ident
          | "begin" statement { ";" statement } "end"
          | "if" condition "then" statement
          | "while" condition "do" statement ] .
condition = "odd" expression
          | expression ( "=" | "#" | "<" | ">" ) expression .
expression = [ "+" | "-" ] term { ( "+" | "-" ) term } .
term      = factor { ( "*" | "/" ) factor } .
factor    = ident
          | number
          | "(" expression ")" .
```
