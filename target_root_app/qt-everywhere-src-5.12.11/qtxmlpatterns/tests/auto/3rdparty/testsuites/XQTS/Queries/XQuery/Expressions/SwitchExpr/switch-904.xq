xquery version "1.1";
(:*******************************************************:)
(: Test: switch-904                                      :)
(: Written by: Michael Kay                               :)
(: Purpose: switch, static error, no default             :)
(:*******************************************************:)

declare variable $in external := 2;

<out>{
switch ($in) 
   case 1 return "Moo"
   case 5 return "Meow"
   case 3 return "Quack"
   case ($in to 4) return "Oink"
}</out>    