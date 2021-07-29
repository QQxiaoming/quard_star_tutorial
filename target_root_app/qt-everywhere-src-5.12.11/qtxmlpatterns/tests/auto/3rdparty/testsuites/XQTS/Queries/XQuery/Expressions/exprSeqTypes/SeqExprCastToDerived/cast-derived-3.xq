(: name : cast-derived-3 :)
(: description : Casting from float to nonPositiveInteger.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:float(-10.0)
return $value cast as xs:nonPositiveInteger