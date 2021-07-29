(: Name: extvardeclwithouttype-6 :)
(: Description: Evaluates an external variable declaration without type:)
(: Use type xs:float. :)

declare variable $x external;

xs:float($x)