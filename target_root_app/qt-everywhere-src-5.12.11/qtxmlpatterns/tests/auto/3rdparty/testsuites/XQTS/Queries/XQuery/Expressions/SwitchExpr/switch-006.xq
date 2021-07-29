xquery version "1.1";
(:*******************************************************:)
(: Test: switch-006                                      :)
(: Written by: Michael Kay                               :)
(: Purpose: switch test, numeric case clauses, no type error :)
(:*******************************************************:)

declare variable $number as xs:decimal external := 42;

<out>{
switch ($number) 
   case 21 return "Moo"
   case current-time() return "Meow"
   case 42 return "Quack"
   default return 3.14159
}</out>    