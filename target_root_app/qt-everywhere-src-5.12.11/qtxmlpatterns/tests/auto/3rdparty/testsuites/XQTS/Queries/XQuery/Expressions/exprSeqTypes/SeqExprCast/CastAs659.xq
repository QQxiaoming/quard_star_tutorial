(:*******************************************************:)
(:Test: CastAs659                                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 10, 2006                                    :)
(:Purpose: Evaluates casting a string into an xs:unsignedByte :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

xs:string(12) cast as xs:unsignedByte