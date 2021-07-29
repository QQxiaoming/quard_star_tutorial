(:*******************************************************:)
(:Test: CastAs666                                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 11, 2006                                    :)
(:Purpose: Evaluates casting a xs:long into an xs:int    :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

let $var := xs:long(120) cast as xs:int
return $var instance of xs:int