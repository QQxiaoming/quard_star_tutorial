(:*******************************************************:)
(:Test: fn-timezone-from-time-11                         :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 27, 2005                                    :)
(:Purpose: Evaluates The "timezone-from-time" function    :)
(:as part of a "div" expression.                         :) 
(:*******************************************************:)

fn:timezone-from-time(xs:time("22:33:10+10:00")) div fn:timezone-from-time(xs:time("02:11:02+05:00"))
