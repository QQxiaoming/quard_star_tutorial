(: name : cast-to-parent-11 :)
(: description : Casting from an unsignedInt to a float.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:unsignedInt(10.0)
return $value cast as xs:float