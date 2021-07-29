(: Name: extvardeclwithouttype-21 :)
(: Description: Evaluates an external variable whose case is changed.:)

declare variable $x external;

fn:lower-case($x)