(: name : cast-to-parent-12 :)
(: description : Casting from an unsignedShort to a float.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:unsignedShort(10.0)
return $value cast as xs:float