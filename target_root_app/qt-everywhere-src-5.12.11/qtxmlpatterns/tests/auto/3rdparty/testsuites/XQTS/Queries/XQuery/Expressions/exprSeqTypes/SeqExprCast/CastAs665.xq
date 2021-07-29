(:*******************************************************:)
(:Test: CastAs665                                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 11, 2006                                    :)
(:Purpose: Evaluates casting a xs:int into an xs:long :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

let $var := xs:int(120) cast as xs:long
return $var instance of xs:long