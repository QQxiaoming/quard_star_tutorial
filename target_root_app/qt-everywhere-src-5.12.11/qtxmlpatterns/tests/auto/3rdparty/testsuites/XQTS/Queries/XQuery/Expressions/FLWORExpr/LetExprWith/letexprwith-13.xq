(: Name: letexprwith-13 :)
(: Description: Let clause using the fn:count function. :)

let $var as xs:integer := fn:count((100,200))
return $var