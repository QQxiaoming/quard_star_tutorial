(:*******************************************************:)
(:Test: fn-days-from-duration-16                         :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "days-from-duration" function   :)
(:as part of a "numeric-equal" expression (eq operator)  :) 
(:*******************************************************:)

fn:days-from-duration(xs:dayTimeDuration("P22DT10H")) eq fn:days-from-duration(xs:dayTimeDuration("P22DT09H"))
