(: name : cast-derived-1 :)
(: description :Casting from float to decimal.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:float(10.0)
return $value cast as xs:decimal