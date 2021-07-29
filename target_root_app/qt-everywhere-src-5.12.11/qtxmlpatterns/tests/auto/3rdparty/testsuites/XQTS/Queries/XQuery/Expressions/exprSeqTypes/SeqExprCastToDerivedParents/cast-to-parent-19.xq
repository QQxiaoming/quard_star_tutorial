(: name : cast-to-parent-19 :)
(: description : Casting from an int to a double.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:int(10E2)
return $value cast as xs:double