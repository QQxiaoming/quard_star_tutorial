(:*******************************************************:)
(:Test: fn-days-from-duration-8                          :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "days-from-duration" function   :)
(:as part of a "+" expression.                           :) 
(:*******************************************************:)

fn:days-from-duration(xs:dayTimeDuration("P21DT10H")) + fn:days-from-duration(xs:dayTimeDuration("P22DT11H"))