(:*******************************************************:)
(:Test: CastAs661                                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 10, 2006                                    :)
(:Purpose: Evaluates casting a short into an xs:long :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

let $var := xs:short(120) cast as xs:long
return $var instance of xs:long