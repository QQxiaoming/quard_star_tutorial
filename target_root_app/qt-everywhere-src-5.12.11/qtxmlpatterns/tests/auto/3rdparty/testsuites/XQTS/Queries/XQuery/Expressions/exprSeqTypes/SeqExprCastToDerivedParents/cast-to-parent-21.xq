(: name : cast-to-parent-21 :)
(: description : Casting from a short to a double.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:short(10E2)
return $value cast as xs:double