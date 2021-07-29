(: name : cast-within-14 :)
(: description : Casting from long to a nonNegativeInteger.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:long(10.0)
return $value cast as xs:nonNegativeInteger