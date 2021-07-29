(: name : cast-within-17 :)
(: description : Casting from long to an unsignedLong.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:long(10)
return $value cast as xs:unsignedLong