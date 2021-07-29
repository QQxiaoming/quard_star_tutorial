(: Name: extvardeclwithtype-14 :)
(: Description: Evaluates an external variable that divides (div operator) two integers:)
(: Both queries perform the operation. :)

declare variable $x as xs:integer external;

$x div xs:integer(10)