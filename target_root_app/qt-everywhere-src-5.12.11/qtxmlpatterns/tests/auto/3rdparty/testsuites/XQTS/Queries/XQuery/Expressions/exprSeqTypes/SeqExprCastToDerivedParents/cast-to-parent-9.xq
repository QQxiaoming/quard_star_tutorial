(: name : cast-to-parent-9 :)
(: description : Casting from a positiveInteger to a float.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:positiveInteger(10.0)
return $value cast as xs:float