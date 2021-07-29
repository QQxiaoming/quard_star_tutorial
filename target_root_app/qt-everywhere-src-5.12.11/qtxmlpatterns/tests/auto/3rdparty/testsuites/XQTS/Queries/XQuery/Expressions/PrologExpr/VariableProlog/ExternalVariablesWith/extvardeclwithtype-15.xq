(: Name: extvardeclwithtype-15 :)
(: Description: Evaluates an external variable that perform modulus operation on two integers:)
(: Both queries perform the operation. :)

declare variable $x as xs:integer external;

$x mod xs:integer(2)