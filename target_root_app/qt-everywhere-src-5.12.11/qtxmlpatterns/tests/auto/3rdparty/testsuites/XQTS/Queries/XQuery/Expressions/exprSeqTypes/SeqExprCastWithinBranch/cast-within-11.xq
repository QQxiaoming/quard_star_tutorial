(: name : cast-within-11 :)
(: description : Casting from integer to unsignedShort.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:integer(10.0)
return $value cast as xs:unsignedShort