(: Name: letexprwith-14 :)
(: Description: Let clause using the fn:not function. :)

let $var as xs:boolean := fn:not(fn:true())
return $var