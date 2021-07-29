(:*******************************************************:)
(:Test: fn-minutes-from-duration-3                       :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "minutes-from-duration" function:)
(:involving a "numeric-less-than" operation (lt operator):)
(:*******************************************************:)

fn:minutes-from-duration(xs:dayTimeDuration("P20DT20H20M")) lt fn:minutes-from-duration(xs:dayTimeDuration("P03DT02H10M"))