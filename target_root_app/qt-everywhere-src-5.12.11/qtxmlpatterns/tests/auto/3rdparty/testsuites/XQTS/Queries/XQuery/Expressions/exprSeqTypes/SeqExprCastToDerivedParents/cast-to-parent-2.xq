(: name : cast-parent-2 :)
(: description : Casting from intger to float.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:integer(10.0)
return $value cast as xs:float