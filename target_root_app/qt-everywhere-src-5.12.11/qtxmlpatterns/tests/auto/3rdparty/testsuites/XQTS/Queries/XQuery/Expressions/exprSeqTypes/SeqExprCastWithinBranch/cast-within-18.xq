(: name : cast-within-18 :)
(: description : Casting from long to a positiveInteger.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:long(10)
return $value cast as xs:positiveInteger