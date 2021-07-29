(: name : cast-derived-13 :)
(: description : Casting from float to an unsignedByte.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:float(10.0)
return $value cast as xs:unsignedByte