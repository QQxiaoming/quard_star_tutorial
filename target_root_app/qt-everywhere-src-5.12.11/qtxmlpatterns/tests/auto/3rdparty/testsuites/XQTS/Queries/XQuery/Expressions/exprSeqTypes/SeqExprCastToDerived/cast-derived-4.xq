(: name : cast-derived-4 :)
(: description : Casting from float to a long.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:float(10.0)
return $value cast as xs:long