(: name : cast-derived-5 :)
(: description : Casting from float to a nonNegativeInteger.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:float(10.0)
return $value cast as xs:nonNegativeInteger