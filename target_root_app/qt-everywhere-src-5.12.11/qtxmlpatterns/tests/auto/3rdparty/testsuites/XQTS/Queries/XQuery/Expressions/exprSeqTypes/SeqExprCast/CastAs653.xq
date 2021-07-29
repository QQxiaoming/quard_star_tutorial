(:*******************************************************:)
(:Test: CastAs653                                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 10, 2006                                    :)
(:Purpose: Evaluates casting a string into an xs:unsignedLong :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

xs:string(20) cast as xs:unsignedLong