(: name : cast-to-parent-16 :)
(: description : Casting from an integer to a double.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:integer(10E2)
return $value cast as xs:double