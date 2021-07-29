xquery version "1.1";
(:*******************************************************:)
(: Test: switch-011                                      :)
(: Written by: Michael Kay                               :)
(: Purpose: switch test, NaN matches NaN                 :)
(:*******************************************************:)

declare variable $in external := xs:double('NaN');

<out>{
switch ($in) 
   case 42 return "Moo"
   case <a>42</a> return "Meow"
   case 42e0 return "Quack"
   case "42e0" return "Oink"
   case xs:float('NaN') return "Woof"
   default return "Expletive deleted"
}</out>    