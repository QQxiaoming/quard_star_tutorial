(:*******************************************************:)
(:Test: fn-days-from-duration-10                         :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "days-from-duration" function   :)
(:as part of a "*" expression.                           :) 
(:*******************************************************:)

fn:days-from-duration(xs:dayTimeDuration("P20DT09H")) * fn:days-from-duration(xs:dayTimeDuration("P03DT10H"))