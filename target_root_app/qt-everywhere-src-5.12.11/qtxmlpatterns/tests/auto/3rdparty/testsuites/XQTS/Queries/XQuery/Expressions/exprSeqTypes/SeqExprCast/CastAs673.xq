(:*******************************************************:)
(:Test: CastAs673                                        :)
(:Written By: Nicolae Brinza                             :)
(:Purpose: Try to cast a variable to xs:QName            :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

let $var := "ABC"
return $var cast as xs:QName
