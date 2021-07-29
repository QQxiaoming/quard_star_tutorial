(: Name: letexprwith-17 :)
(: Description: Let clause using the "string-length" function. :)

let $var as xs:integer := fn:string-length("A String")
return $var