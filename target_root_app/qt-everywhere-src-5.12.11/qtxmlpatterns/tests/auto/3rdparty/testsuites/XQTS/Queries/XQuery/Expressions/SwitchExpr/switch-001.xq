xquery version "1.1";
(:*******************************************************:)
(: Test: switch-001                                      :)
(: Written by: Michael Kay                               :)
(: Purpose: basic switch example                         :)
(:*******************************************************:)

declare variable $animal as xs:string external := "Cat";

<out>{
switch ($animal) 
   case "Cow" return "Moo"
   case "Cat" return "Meow"
   case "Duck" return "Quack"
   default return "What's that odd noise?"
}</out>    