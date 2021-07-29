xquery version "1.1";
(:*******************************************************:)
(: Test: switch-905                                      :)
(: Written by: Michael Kay                               :)
(: Purpose: switch, static error, empty case list        :)
(:*******************************************************:)

declare variable $in external := 2;

<out>{
switch ($in) 
   case 1 return "Moo"
   case 5 return "Meow"
   return "Quack"
   case ($in to 4) return "Oink"
   default return "Baa"
}</out>    