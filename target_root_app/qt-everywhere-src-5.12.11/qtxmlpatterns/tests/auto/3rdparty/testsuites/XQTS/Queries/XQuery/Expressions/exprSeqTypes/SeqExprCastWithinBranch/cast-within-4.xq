(: name : cast-within-4 :)
(: description : Casting from integer to negativeInteger.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:integer(-10.0)
return $value cast as xs:negativeInteger