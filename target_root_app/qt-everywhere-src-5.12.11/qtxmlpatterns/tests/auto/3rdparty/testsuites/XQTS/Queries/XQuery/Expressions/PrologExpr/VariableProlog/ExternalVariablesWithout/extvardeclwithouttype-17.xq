(: Name: extvardeclwithouttype-17 :)
(: Description: Evaluates an external variable that evaluates a boolean expression:)
(: Both queries perform the operation. :)

declare variable $x external;

$x or fn:false()