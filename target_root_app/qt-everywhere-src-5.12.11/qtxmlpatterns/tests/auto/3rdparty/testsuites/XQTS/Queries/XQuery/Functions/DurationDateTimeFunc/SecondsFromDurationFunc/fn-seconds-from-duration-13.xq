(:*******************************************************:)
(:Test: fn-seconds-from-duration-13                      :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "seconds-from-duration" function:)
(:as part of a "mod" expression.                         :) 
(:*******************************************************:)

fn:seconds-from-duration(xs:dayTimeDuration("P10DT10H20M10S")) mod fn:seconds-from-duration(xs:dayTimeDuration("P03DT03H03M03S"))
