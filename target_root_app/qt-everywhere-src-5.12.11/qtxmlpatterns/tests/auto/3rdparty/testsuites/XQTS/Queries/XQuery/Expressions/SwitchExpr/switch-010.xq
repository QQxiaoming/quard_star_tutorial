xquery version "1.1";
(:*******************************************************:)
(: Test: switch-010                                      :)
(: Written by: Michael Kay                               :)
(: Purpose: switch test, non-matching empty              :)
(:*******************************************************:)

declare variable $in external := 21;

<out>{
switch ($in) 
   case 42 return "Moo"
   case <a>42</a> return "Meow"
   case 42e0 return "Quack"
   case "42e0" return "Oink"
   case () return "Woof"
   default return "Expletive deleted"
}</out>    