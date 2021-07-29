(: Name: extvardeclwithtoutype-3 :)
(: Description: Evaluates an external variable declaration without type:)
(: Use type xs:integer. :)

declare variable $x external;

xs:integer($x)