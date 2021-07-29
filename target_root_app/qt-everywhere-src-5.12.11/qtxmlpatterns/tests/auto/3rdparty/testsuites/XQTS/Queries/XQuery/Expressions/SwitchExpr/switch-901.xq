xquery version "1.1";
(:*******************************************************:)
(: Test: switch-901                                      :)
(: Written by: Michael Kay                               :)
(: Purpose: switch, type error, switch operand >1 item   :)
(:*******************************************************:)

declare variable $in external := 2;

<out>{
switch (1 to $in) 
   case 1 return "Moo"
   case 2 return "Meow"
   case 3 return "Quack"
   case 4 return "Oink"
   default return "Baa"
}</out>    