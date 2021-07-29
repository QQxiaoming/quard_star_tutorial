xquery version "1.1";
(:*******************************************************:)
(: Test: switch-004                                      :)
(: Written by: Michael Kay                               :)
(: Purpose: switch test, multiple case clauses           :)
(:*******************************************************:)

declare variable $animal as xs:string external := "goose";

<out>{
switch (upper-case($animal)) 
   case "COW" return "Moo"
   case "CAT" return "Meow"
   case "DUCK" 
   case "GOOSE" return "Quack"
   case "PIG"
   case "SWINE" return "Oink"
   default return "What's that odd noise?"
}</out>    