(: name : cast-to-parent-1 :)
(: description :Casting from decimal to float.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:decimal(10.0)
return $value cast as xs:float