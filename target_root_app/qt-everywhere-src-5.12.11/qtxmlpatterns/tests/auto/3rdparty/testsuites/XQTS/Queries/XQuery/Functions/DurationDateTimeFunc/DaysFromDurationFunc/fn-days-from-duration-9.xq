(:*******************************************************:)
(:Test: fn-days-from-duration-9                          :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 11, 2005                                    :)
(:Purpose: Evaluates The "days-from-duration" function   :)
(:as part of a "-" expression.                           :) 
(:*******************************************************:)

fn:days-from-duration(xs:dayTimeDuration("P30DT10H")) - fn:days-from-duration(xs:dayTimeDuration("P10DT09H"))
