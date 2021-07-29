(:*******************************************************:)
(:Test: fn-days-from-duration-3                          :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "days-from-duration" function   :)
(:involving a "numeric-less-than" operation (lt operator):)
(:*******************************************************:)

fn:days-from-duration(xs:dayTimeDuration("P20DT20H")) lt fn:days-from-duration(xs:dayTimeDuration("P03DT02H"))