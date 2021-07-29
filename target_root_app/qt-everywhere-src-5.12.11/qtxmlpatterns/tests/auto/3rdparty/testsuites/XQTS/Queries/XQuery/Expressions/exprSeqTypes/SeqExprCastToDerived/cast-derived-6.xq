(: name : cast-derived-6 :)
(: description : Casting from float to a negativeInteger.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:float(-10.0)
return $value cast as xs:negativeInteger