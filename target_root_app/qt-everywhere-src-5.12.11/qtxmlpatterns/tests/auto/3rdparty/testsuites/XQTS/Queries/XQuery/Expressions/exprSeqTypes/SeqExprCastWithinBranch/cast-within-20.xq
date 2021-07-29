(: name : cast-within-20 :)
(: description : Casting from long to unsignedInt.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:long(10)
return $value cast as xs:unsignedInt