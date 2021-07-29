(:*******************************************************:)
(:Test: fn-hours-from-duration-9                         :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 11, 2005                                    :)
(:Purpose: Evaluates The "hours-from-duration" function  :)
(:as part of a "-" expression.                           :) 
(:*******************************************************:)

fn:hours-from-duration(xs:dayTimeDuration("P30DT10H")) - fn:hours-from-duration(xs:dayTimeDuration("P10DT02H"))
