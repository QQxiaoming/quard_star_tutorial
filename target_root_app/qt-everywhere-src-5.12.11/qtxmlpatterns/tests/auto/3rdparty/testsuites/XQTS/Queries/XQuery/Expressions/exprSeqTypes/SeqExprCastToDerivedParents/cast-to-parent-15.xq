(: name : cast-to-parent-15 :)
(: description : Casting from a decimal to a double.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:decimal(10E2)
return $value cast as xs:double