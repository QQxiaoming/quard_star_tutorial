(: name : cast-within-6 :)
(: description : Casting from integer to unsignedLong.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:integer(10.0)
return $value cast as xs:unsignedLong