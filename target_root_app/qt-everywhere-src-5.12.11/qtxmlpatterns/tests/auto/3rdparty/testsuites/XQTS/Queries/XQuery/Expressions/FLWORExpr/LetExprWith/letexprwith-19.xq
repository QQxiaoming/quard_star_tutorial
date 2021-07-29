(: Name: letexprwith-19 :)
(: Description: Let clause using a casting from decimal to string :)

let $var as xs:string := xs:string((xs:decimal(100)))
return $var