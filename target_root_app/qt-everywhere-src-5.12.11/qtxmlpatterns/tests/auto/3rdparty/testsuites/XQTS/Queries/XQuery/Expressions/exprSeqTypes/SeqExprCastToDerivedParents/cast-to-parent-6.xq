(: name : cast-to-parent-6 :)
(: description : Casting from a negativeInteger to a float.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:negativeInteger(-10.0)
return $value cast as xs:float