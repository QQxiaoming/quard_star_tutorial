(: Name: extvardeclwithtype-10 :)
(: Description: Evaluates an external variable that adds two integers:)
(: Only THIS query performs the operation. :)

declare variable $x as xs:integer external;

$x + $x