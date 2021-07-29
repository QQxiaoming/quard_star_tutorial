(: Name: letexprwith-23 :)
(: Description: Let clause using an "if" expression :)

let $var as xs:boolean := if (fn:true()) then fn:true() else fn:false()
return $var