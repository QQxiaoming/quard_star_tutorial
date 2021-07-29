(: Name: extvardeclwithtype-22 :)
(: Description: Evaluates an external variable whose value is succesfully casted.:)

declare variable $x as xs:boolean external;

fn:not($x)