(: Name: extvardeclwithtype-2 :)
(: Description: Evaluates an external variable declaration with type:)
(: Use type xs:date. :)

declare variable $x as xs:date external;

fn:string($x)