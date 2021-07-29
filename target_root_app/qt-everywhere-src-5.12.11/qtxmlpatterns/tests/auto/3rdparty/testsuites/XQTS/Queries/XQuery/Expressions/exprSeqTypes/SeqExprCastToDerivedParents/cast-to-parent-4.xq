(: name : cast-to-parent-4 :)
(: description : Casting from long to float.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:long(10.0)
return $value cast as xs:float