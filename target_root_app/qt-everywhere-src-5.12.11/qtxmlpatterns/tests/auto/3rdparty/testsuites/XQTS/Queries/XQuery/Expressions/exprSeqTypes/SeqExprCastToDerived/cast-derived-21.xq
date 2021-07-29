(: name : cast-derived-21 :)
(: description : Casting from double to a short.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:double(10E2)
return $value cast as xs:short