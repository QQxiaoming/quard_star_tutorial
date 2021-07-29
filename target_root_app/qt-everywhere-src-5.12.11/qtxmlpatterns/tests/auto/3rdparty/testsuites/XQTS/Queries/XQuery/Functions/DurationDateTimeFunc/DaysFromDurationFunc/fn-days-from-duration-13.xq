(:*******************************************************:)
(:Test: fn-days-from-duration-13                         :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "days-from-duration" function   :)
(:as part of a "mod" expression.                         :) 
(:*******************************************************:)

fn:days-from-duration(xs:dayTimeDuration("P10DT10H")) mod fn:days-from-duration(xs:dayTimeDuration("P03DT03H"))
