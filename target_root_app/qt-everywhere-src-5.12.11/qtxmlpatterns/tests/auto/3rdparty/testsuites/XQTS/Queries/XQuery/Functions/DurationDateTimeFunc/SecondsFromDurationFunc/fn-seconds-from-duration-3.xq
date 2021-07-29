(:*******************************************************:)
(:Test: fn-seconds-from-duration-3                       :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "seconds-from-duration" function:)
(:involving a "numeric-less-than" operation (lt operator):)
(:*******************************************************:)

fn:seconds-from-duration(xs:dayTimeDuration("P20DT20H20M10S")) lt fn:seconds-from-duration(xs:dayTimeDuration("P03DT02H10M20S"))