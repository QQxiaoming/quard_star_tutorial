(: Name: letexprwith-18 :)
(: Description: Let clause using a casting from integer to string :)

let $var as xs:string := xs:string((xs:integer(100)))
return $var