(:*******************************************************:)
(:Test: CastAs667                                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 11, 2006                                    :)
(:Purpose: Evaluates casting a xs:unsignedShort into an xs:unsignedInt    :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

let $var := xs:unsignedShort(120) cast as xs:unsignedInt
return $var instance of xs:unsignedInt