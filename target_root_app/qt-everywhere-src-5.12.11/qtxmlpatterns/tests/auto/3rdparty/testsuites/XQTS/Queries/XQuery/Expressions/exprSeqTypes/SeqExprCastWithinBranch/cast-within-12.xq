(: name : cast-within-12 :)
(: description : Casting from integer to an unsignedByte.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:integer(10.0)
return $value cast as xs:unsignedByte