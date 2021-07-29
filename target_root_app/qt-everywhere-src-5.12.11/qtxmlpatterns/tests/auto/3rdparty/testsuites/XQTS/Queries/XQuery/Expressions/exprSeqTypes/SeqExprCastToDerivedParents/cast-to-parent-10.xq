(: name : cast-to-parent-10 :)
(: description : Casting from a shortto a float.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:short(10.0)
return $value cast as xs:float