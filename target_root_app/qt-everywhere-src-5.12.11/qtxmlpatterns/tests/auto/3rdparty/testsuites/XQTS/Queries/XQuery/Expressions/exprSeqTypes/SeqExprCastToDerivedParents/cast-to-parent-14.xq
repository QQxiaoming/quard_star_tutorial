(: name : cast-to-parent-14 :)
(: description : Casting from a byte to a float.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:byte(10.0)
return $value cast as xs:float