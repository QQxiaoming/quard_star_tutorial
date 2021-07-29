(:*******************************************************:)
(:Test: fn-minutes-from-duration-9                       :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 11, 2005                                    :)
(:Purpose: Evaluates The "minutes-from-duration" function:)
(:as part of a "-" expression.                           :) 
(:*******************************************************:)

fn:minutes-from-duration(xs:dayTimeDuration("P30DT10H20M")) - fn:minutes-from-duration(xs:dayTimeDuration("P10DT09H10M"))
