(: name : cast-to-parent-8 :)
(: description : Casting from an unsignedLong to a float.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:unsignedLong(10.0)
return $value cast as xs:float