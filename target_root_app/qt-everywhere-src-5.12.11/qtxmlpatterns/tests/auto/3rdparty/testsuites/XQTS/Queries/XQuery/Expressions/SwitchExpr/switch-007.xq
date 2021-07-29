xquery version "1.1";
(:*******************************************************:)
(: Test: switch-007                                      :)
(: Written by: Michael Kay                               :)
(: Purpose: switch test, untypedAtomic is converted to string :)
(:*******************************************************:)

declare variable $in external := <a>42</a>;

<out>{
switch ($in) 
   case 42 return "Moo"
   case "42" return "Meow"
   case 42e0 return "Quack"
   case "42e0" return "Oink"
   default return "Expletive deleted"
}</out>    