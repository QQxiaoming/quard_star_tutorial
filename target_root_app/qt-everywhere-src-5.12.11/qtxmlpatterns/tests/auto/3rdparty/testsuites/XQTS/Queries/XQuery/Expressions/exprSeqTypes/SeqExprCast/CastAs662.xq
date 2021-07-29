(:*******************************************************:)
(:Test: CastAs662                                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 11, 2006                                    :)
(:Purpose: Evaluates casting a xs:nonPositiveInteger into an xs:negativeInteger :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

let $var := xs:nonPositiveInteger(-120) cast as xs:negativeInteger
return $var instance of xs:negativeInteger