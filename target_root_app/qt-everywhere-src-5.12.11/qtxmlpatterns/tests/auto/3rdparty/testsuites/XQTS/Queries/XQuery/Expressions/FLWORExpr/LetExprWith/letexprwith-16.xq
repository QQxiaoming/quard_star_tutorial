(: Name: letexprwith-16 :)
(: Description: Let clause using the "or" operator function. :)

let $var as xs:boolean := fn:true() and fn:true()
return $var