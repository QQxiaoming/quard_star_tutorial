(: name : cast-within-16 :)
(: description : Casting from long to an int.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:long(10)
return $value cast as xs:int