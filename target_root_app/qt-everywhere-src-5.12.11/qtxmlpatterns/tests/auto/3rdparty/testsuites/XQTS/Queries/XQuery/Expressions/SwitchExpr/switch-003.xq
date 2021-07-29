xquery version "1.1";
(:*******************************************************:)
(: Test: switch-003                                      :)
(: Written by: Michael Kay                               :)
(: Purpose: switch test, non-constant case clause        :)
(:*******************************************************:)

declare variable $animal as xs:string external := "!?!?";

<out>{
switch (upper-case($animal)) 
   case "COW" return "Moo"
   case "CAT" return "Meow"
   case "DUCK" return "Quack"
   case lower-case($animal) return "Oink"
   default return "What's that odd noise?"
}</out>    