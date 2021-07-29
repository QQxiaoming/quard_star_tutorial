(: name : cast-within-13 :)
(: description : Casting from a long to an integer.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:long(10.0)
return $value cast as xs:integer