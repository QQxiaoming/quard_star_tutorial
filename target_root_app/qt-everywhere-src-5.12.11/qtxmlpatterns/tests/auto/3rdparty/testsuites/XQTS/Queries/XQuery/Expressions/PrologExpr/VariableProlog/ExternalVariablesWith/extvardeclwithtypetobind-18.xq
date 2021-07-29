(: Name: extvardeclwithtypetobind-18 :)
(: Description: Binding result of fn:avg function for extvardeclwithtype-18.:)

let $var := avg((1,2,4))
return xs:float($var)