(: Name: extvardeclwithtype-5 :)
(: Description: Evaluates an external variable declaration with type:)
(: Use type xs:boolean. :)

declare variable $x as xs:boolean external;

fn:string($x)