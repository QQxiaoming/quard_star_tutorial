(: Name: extvardeclwithtype-21 :)
(: Description: Evaluates an external variable whose case is changed.:)

declare variable $x as xs:string external;

fn:lower-case($x)