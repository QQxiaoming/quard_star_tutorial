xquery version "1.1";
(:*******************************************************:)
(: Test: switch-012                                      :)
(: Written by: Michael Kay                               :)
(: Purpose: switch test, no dynamic errors               :)
(:*******************************************************:)

declare variable $in external := 25;
declare variable $zero external := 0;

<out>{
switch ($in) 
   case 42 return $in div $zero
   case 25 return "Baa"
   case 39 return $in div $zero
   default return "Woof"
}</out>    