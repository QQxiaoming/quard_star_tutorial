(: name : cast-derived-10 :)
(: description : Casting from float to a short.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:float(10.0)
return $value cast as xs:short