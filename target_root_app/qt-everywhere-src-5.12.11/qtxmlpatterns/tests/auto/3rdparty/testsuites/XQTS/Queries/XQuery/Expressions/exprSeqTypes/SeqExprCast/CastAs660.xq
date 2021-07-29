(:*******************************************************:)
(:Test: CastAs660                                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 10, 2006                                    :)
(:Purpose: Evaluates casting a long into an xs:short :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

let $var := xs:long(120) cast as xs:short
return $var instance of xs:short