(: Name: whereClause-5 :)
(: Description: use of where clause with a typeswitch expression :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := 100
where typeswitch($var)
        case $i as xs:string
         return fn:false() 
        case $i as xs:integer
         return fn:true() 
        default
         return fn:false()
return
 $var