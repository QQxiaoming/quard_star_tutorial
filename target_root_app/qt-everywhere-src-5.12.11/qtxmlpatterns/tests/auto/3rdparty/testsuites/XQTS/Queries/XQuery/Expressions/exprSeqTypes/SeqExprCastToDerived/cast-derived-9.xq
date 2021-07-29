(: name : cast-derived-9 :)
(: description : Casting from float to a positiveInteger.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:float(10.0)
return $value cast as xs:positiveInteger