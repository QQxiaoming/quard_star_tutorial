(: Name: letexprwith-15 :)
(: Description: Let clause using the "and" operator function. :)

let $var as xs:boolean := fn:true() and fn:true()
return $var