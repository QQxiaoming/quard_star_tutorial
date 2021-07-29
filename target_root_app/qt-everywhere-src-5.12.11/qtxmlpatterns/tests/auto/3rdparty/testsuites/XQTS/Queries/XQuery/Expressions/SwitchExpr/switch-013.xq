xquery version "1.1";
(:*******************************************************:)
(: Test: switch-013                                      :)
(: Written by: Michael Kay                               :)
(: Purpose: switch test, no dynamic errors               :)
(:*******************************************************:)

declare variable $in external := 25;
declare variable $zero external := 0;

<out>{
switch ($in) 
   case 42 return "Quack"
   case 25 return "Baa"
   case $in div $zero return "Neigh"
   default return "Woof"
}</out>    