(:*******************************************************:)
(:Test: CastAs664                                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 11, 2006                                    :)
(:Purpose: Evaluates casting a xs:short into an xs:unsignedShort :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

let $var := xs:short(120) cast as xs:unsignedShort
return $var instance of xs:unsignedShort