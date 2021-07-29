(: name : cast-derived-8 :)
(: description : Casting from float to an unsignedLong.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:float(10.0)
return $value cast as xs:unsignedLong