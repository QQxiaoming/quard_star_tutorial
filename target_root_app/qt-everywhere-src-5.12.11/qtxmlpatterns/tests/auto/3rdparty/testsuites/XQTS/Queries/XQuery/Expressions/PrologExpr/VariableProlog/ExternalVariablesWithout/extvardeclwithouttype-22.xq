(: Name: extvardeclwithouttype-22 :)
(: Description: Evaluates an external variable with no type whose value is succesfully casted.:)

declare variable $x external;

fn:not($x)