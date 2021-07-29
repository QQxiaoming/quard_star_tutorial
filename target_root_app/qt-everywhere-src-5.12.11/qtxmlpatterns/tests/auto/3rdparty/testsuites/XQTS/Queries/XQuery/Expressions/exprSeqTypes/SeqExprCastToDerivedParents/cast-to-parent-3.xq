(: name : cast-to-parent-3 :)
(: description : Casting from nonPositiveInteger to float.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:nonPositiveInteger(-10.0)
return $value cast as xs:float