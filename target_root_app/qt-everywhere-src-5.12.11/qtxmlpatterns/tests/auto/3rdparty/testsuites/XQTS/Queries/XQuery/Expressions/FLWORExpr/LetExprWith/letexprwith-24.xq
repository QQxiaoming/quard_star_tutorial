(: Name: letexprwith-24 :)
(: Description: Let clause using a "typeswitch" expression :)

let $var as xs:string := typeswitch(fn:true())
                           case $i as xs:boolean
                             return "Test Passed"
                           default
                             return "Test failed"
return $var