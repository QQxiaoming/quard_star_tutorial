(: name : cast-within-21 :)
(: description : Casting from long to a byte.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:long(10)
return $value cast as xs:byte