(: name : cast-within-19 :)
(: description : Casting from long to a short.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:long(10)
return $value cast as xs:short