(:*******************************************************:)
(:Test: fn-seconds-from-duration-9                       :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 11, 2005                                    :)
(:Purpose: Evaluates The "seconds-from-duration" function:)
(:as part of a "-" expression.                           :) 
(:*******************************************************:)

fn:seconds-from-duration(xs:dayTimeDuration("P30DT10H20M10S")) - fn:seconds-from-duration(xs:dayTimeDuration("P10DT09H10M02S"))
