(: Name: letexprwith-21 :)
(: Description: Let clause using a casting from boolean to string :)

let $var as xs:string := xs:string(fn:true())
return $var