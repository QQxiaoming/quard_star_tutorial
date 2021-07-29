(: name : cast-to-parent-7 :)
(: description : Casting from an int to a float.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:int(10.0)
return $value cast as xs:float