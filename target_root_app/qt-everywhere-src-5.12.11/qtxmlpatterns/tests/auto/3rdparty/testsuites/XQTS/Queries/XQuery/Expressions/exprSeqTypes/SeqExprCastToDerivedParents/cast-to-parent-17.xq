(: name : cast-to-parent-17 :)
(: description : Casting from a positiveInteger to a double.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:positiveInteger(10E2)
return $value cast as xs:double