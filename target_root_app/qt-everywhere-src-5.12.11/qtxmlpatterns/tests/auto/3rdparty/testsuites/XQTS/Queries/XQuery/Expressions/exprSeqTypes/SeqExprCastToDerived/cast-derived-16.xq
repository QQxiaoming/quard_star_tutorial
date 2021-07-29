(: name : cast-derived-16 :)
(: description : Casting from double to an integer.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:double(10E2)
return $value cast as xs:integer