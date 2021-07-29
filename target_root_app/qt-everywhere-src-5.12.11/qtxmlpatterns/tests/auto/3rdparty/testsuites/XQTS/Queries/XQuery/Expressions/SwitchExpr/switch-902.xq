xquery version "1.1";
(:*******************************************************:)
(: Test: switch-902                                      :)
(: Written by: Michael Kay                               :)
(: Purpose: switch, type error, case operand >1 item   :)
(:*******************************************************:)

declare variable $in external := 2;

<out>{
switch ($in) 
   case 1 return "Moo"
   case 5 return "Meow"
   case 3 return "Quack"
   case ($in to 4) return "Oink"
   default return "Baa"
}</out>    