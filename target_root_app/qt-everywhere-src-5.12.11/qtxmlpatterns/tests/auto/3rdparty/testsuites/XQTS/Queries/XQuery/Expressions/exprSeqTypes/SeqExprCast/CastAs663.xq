(:*******************************************************:)
(:Test: CastAs663                                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 11, 2006                                    :)
(:Purpose: Evaluates casting a xs:nonNegativeInteger into an xs:positiveInteger :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

let $var := xs:nonNegativeInteger(120) cast as xs:positiveInteger
return $var instance of xs:positiveInteger