(: name : cast-to-parent-13 :)
(: description : Casting from an unsignedByte to a float.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:unsignedByte(10.0)
return $value cast as xs:float