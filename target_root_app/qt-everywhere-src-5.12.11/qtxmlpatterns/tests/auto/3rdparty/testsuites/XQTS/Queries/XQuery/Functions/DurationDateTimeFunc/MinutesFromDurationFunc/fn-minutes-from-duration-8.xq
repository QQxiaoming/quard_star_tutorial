(:*******************************************************:)
(:Test: fn-minutes-from-duration-8                       :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "minutes-from-duration" function:)
(:as part of a "+" expression.                           :) 
(:*******************************************************:)

fn:minutes-from-duration(xs:dayTimeDuration("P21DT10H10M")) + fn:minutes-from-duration(xs:dayTimeDuration("P22DT11H30M"))