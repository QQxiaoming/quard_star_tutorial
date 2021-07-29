(: name : cast-to-parent-20 :)
(: description : Casting from an unsignedLong to a double.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:unsignedLong(10E2)
return $value cast as xs:double