(: name : cast-to-parent-18 :)
(: description : Casting from a long to a double.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:long(10E2)
return $value cast as xs:double