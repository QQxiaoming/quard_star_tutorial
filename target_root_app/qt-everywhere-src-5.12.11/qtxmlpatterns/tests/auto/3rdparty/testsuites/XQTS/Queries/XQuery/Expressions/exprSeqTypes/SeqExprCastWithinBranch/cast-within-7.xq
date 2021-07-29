(: name : cast-within-7 :)
(: description : Casting from integer to positiveInteger.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:integer(10.0)
return $value cast as xs:positiveInteger