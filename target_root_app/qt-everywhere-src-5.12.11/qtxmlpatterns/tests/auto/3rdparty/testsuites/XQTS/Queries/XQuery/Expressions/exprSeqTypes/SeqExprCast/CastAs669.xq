(:*******************************************************:)
(:Test: CastAs669                                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 11, 2006                                    :)
(:Purpose: Evaluates casting a xs:byte into an xs:unsignedByte    :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

let $var := xs:byte(120) cast as xs:unsignedByte
return $var instance of xs:unsignedByte