(: Name: extvardeclwithtype-13 :)
(: Description: Evaluates an external variable that divides two integers:)
(: Both queries perform the operation. :)

declare variable $x as xs:integer external;

$x idiv xs:integer(2)