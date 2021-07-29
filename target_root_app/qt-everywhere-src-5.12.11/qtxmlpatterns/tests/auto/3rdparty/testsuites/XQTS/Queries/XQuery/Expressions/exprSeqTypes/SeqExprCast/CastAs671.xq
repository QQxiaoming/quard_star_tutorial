(:*******************************************************:)
(:Test: CastAs671                                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 13, 2006                                    :)
(:Purpose: Evaluates casting a string as an xs:ENTITY :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

let $var := "abc" cast as xs:ENTITY 
return $var instance of xs:ENTITY