xquery version "1.1";
(:*******************************************************:)
(: Test: switch-008                                      :)
(: Written by: Michael Kay                               :)
(: Purpose: switch test, untypedAtomic is converted to string :)
(:*******************************************************:)

declare variable $in external := "42";

<out>{
switch ($in) 
   case 42 return "Moo"
   case <a>42</a> return "Meow"
   case 42e0 return "Quack"
   case "42e0" return "Oink"
   default return "Expletive deleted"
}</out>    