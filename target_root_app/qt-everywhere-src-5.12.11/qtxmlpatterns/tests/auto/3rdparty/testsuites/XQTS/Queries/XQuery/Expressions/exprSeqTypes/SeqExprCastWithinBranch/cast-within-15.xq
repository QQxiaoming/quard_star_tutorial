(: name : cast-within-15 :)
(: description : Casting from long to a negativeInteger.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $value := xs:long(-10)
return $value cast as xs:negativeInteger