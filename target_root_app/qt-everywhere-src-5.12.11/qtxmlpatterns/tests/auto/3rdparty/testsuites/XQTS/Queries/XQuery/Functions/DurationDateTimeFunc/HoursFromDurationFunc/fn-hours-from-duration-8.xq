(:*******************************************************:)
(:Test: fn-hours-from-duration-8                         :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "hours-from-duration" function  :)
(:as part of a "+" expression.                           :) 
(:*******************************************************:)

fn:hours-from-duration(xs:dayTimeDuration("P21DT10H")) + fn:hours-from-duration(xs:dayTimeDuration("P22DT20H"))