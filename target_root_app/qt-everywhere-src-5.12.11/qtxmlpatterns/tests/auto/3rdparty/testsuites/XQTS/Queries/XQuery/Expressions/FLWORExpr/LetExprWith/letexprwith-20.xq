(: Name: letexprwith-20 :)
(: Description: Let clause using a casting from double to string :)

let $var as xs:string := xs:string((xs:double(100E2)))
return $var