(: name : cast-derived-19 :)
(: description : Casting from double to an int.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:double(10E2)
return $value cast as xs:int