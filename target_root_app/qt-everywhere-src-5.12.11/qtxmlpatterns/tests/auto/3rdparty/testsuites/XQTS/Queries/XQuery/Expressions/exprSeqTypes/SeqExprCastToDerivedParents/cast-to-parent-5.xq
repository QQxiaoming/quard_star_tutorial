(: name : cast-to-parent-5 :)
(: description : Casting from nonNegativeInteger to float.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:nonNegativeInteger(10.0)
return $value cast as xs:float