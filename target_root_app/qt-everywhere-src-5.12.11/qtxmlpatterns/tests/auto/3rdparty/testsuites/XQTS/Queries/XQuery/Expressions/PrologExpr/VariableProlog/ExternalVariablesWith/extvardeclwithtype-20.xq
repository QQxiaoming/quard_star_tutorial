(: Name: extvardeclwithtype-20 :)
(: Description: Evaluates an external variable whose case is changed.:)

declare variable $x as xs:string external;

fn:upper-case($x)